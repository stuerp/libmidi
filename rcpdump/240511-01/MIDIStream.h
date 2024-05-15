
/** $VER: MIDIStream.h (2024.05.10) P. Stuer **/

#pragma once

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include <string.h>

#include <MIDI.h>

#define SYXOPT_DELAY 0x01

class midi_stream_t
{
public:
    typedef uint8_t (* timestamp_handler_t)(midi_stream_t * fileInfo, uint32_t * delay);

    midi_stream_t() : _Data(), _Size(), _Offs(), _TicksPerQuarter(), _Tempo(500000)
    {
        SetTimestampHandler(nullptr);
    }

    midi_stream_t(uint32_t size) : _Size(size), _Offs(), _TicksPerQuarter(), _Tempo(500000)
    {
        _Data = (uint8_t *) ::malloc(_Size);
    }

    virtual ~midi_stream_t()
    {
        if (_Data != nullptr)
        {
            ::free(_Data);

            _Data = nullptr;
            _Size = 0;
            _Offs = 0;
        }
    }

    void Reset() { _Offs = 0; }

    void SetTimestampHandler(timestamp_handler_t timeStampHandler) { _HandleTimestamp = timeStampHandler; }

    uint32_t GetTicksPerQuarter() const noexcept { return _TicksPerQuarter; }

    uint32_t GetTempo() const noexcept { return _Tempo; }
    void SetTempo(uint32_t tempo) noexcept { _Tempo = tempo; }

    uint32_t GetTimestamp() const noexcept { return _State.Timestamp; }
    void SetTimestamp(uint32_t timestamp) noexcept { _State.Timestamp = timestamp; }

    uint8_t GetChannel() const noexcept { return _State.Channel; }
    void SetChannel(uint8_t channel) noexcept { _State.Channel = channel; }

    void WriteMIDIHeader(uint16_t format, uint16_t trackCount, uint16_t ticksPerQuarter)
    {
        Grow(0x08 + 0x06);

        WriteBE32(0x4D546864);  // write 'MThd'
        WriteBE32(0x00000006);  // Header Length

        WriteBE16(format);      // MIDI Format (0/1/2)
        WriteBE16(trackCount);  // number of tracks
        WriteBE16(ticksPerQuarter);  // Ticks per Quarter

        _TicksPerQuarter = ticksPerQuarter;
    }

    void BeginWriteMIDITrack()
    {
        Grow(8);

        WriteBE32(0x4D54726B); // write 'MTrk'
        WriteBE32(0x00000000); // write dummy length

        _State.Offs      = _Offs;
        _State.Timestamp = 0;
        _State.Status    = 0x00;
    }

    void EndWriteMIDITrack()
    {
        uint32_t Size = _Offs - _State.Offs;

        uint8_t * p = &_Data[_State.Offs - 0x04];

        *p++ = (Size >> 24) & 0xFF;
        *p++ = (Size >> 16) & 0xFF;
        *p++ = (Size >>  8) & 0xFF;
        *p++ = (Size >>  0) & 0xFF;
    }

    void WriteEvent(StatusCodes statusCode, uint8_t value1, uint8_t value2)
    {
        _State.Status = 0;

        WriteEventInternal(statusCode, value1, value2);
    }

    void WriteEvent(StatusCodes statusCode, const void * data, uint32_t size)
    {
        WriteTimestamp();

        Grow(1 + 4 + size); // Worst case: 4 bytes of data length

        _State.Status = 0x00;

        _Data[_Offs++] = statusCode;

        EncodeVariableLengthQuantity(size);

        ::memcpy(&_Data[_Offs], data, size);
        _Offs += size;
    }

    void WriteMetaEvent(MetaDataTypes type, uint32_t value, uint32_t size)
    {
        uint8_t Data[4];

        uint8_t * p = Data;

        *p++ = (value >> 24) & 0xFF;
        *p++ = (value >> 16) & 0xFF;
        *p++ = (value >>  8) & 0xFF;
        *p++ = (value >>  0) & 0xFF;

        WriteMetaEvent(type, Data + (4 - size), size);
    }

    void WriteMetaEvent(MetaDataTypes type, const void * data, uint32_t size)
    {
        WriteTimestamp();

        Grow(2 + 5 + size); // Worst case: 5 bytes of data length.

        _State.Status = 0x00;

        _Data[_Offs++] = StatusCodes::MetaData;
        _Data[_Offs++] = type;

        EncodeVariableLengthQuantity(size);

        ::memcpy(&_Data[_Offs], data, size);
        _Offs += size;
    }

    void WriteMetaEvent(MetaDataTypes type, const char * text)
    {
        WriteMetaEvent(type, text, (uint32_t) ::strlen(text));
    }

    void WriteBE32(uint32_t value)
    {
        _Data[_Offs++] = (value >> 24) & 0xFF;
        _Data[_Offs++] = (value >> 16) & 0xFF;
        _Data[_Offs++] = (value >>  8) & 0xFF;
        _Data[_Offs++] = (value >>  0) & 0xFF;
    }

    void WriteBE16(uint16_t value)
    {
        _Data[_Offs++] = (value >> 8) & 0xFF;
        _Data[_Offs++] = (value >> 0) & 0xFF;
    }

    void EncodeVariableLengthQuantity(uint32_t quantity)
    {
        uint8_t Size = 0;

        // Determine the number of required bytes.
        {
            uint32_t t = quantity;

            do
            {
                t >>= 7;
                Size++;
            }
            while (t != 0);
        }

        Grow(Size);

        {
            uint8_t * p = &_Data[_Offs];

            {
                uint32_t n = Size;

                uint32_t t = quantity;

                do
                {
                    n--;
                    p[n] = 0x80 | (t & 0x7F);
                    t >>= 7;
                }
                while (t != 0);
            }

            p[Size - 1] &= 0x7F;
        }

        _Offs += Size;
    }

    void Grow(uint32_t bytesNeeded)
    {
        const uint32_t ChunkSize = 0x8000;

        uint32_t NewOffs = _Offs + bytesNeeded;

        if (NewOffs <= _Size)
            return;

        size_t NewSize = _Size;

        while (NewOffs > NewSize)
            NewSize += ChunkSize;

        void * NewData = ::realloc(_Data, NewSize);

        if (NewData != nullptr)
        {
            _Data = (uint8_t *) NewData;
            _Size = (uint32_t) NewSize;
        }
    }

    void WriteRolandSysEx(const uint8_t * syxHdr, uint32_t address, const uint8_t * data, uint32_t size, uint8_t opts);
    void WriteRolandSysEx(const uint8_t * syxHdr, uint32_t address, const uint8_t * data, uint32_t size, uint8_t opts, uint32_t blockSize);

private:
    void WriteTimestamp()
    {
        if ((_HandleTimestamp != nullptr) && _HandleTimestamp(this, &_State.Timestamp))
            return;

        EncodeVariableLengthQuantity(_State.Timestamp);

        _State.Timestamp = 0;
    }

    void WriteEventInternal(uint8_t statusCode, uint8_t value1, uint8_t value2)
    {
        uint8_t Status = statusCode | _State.Channel;

        WriteTimestamp();

        Grow(3);

        switch (statusCode & 0xF0)
        {
            case StatusCodes::NoteOff:
            case StatusCodes::NoteOn:
            case StatusCodes::KeyPressure:
            case StatusCodes::ControlChange:
            case StatusCodes::PitchBendChange:
            {
                if (_State.Status != Status)
                {
                    _State.Status = Status;

                    _Data[_Offs++] = Status;
                }

                _Data[_Offs++] = value1;
                _Data[_Offs++] = value2;
                break;
            }

            case StatusCodes::ProgramChange:
            case StatusCodes::ChannelPressure:
                if (_State.Status != Status)
                {
                    _State.Status = Status;

                    _Data[_Offs++] = Status;
                }

                _Data[_Offs++] = value1;
                break;

            case StatusCodes::SysEx: // Meta Event: Track End
                _State.Status = 0;

                _Data[_Offs++] = statusCode;
                _Data[_Offs++] = value1;
                _Data[_Offs++] = value2;
                break;

            default:
                break;
        }
    }

public:
    uint8_t * _Data;
    uint32_t _Size;
    uint32_t _Offs;

private:
    uint32_t _TicksPerQuarter;
    uint32_t _Tempo;

    struct midi_state_t
    {
        uint32_t Offs;      // Offset in the track data in the MIDI data
        uint32_t Timestamp; // delay until next event
        uint8_t  Channel;
        uint8_t  Status;

        midi_state_t() : Offs(), Timestamp(), Channel(), Status() { }
    };

    midi_state_t _State;

    static timestamp_handler_t _HandleTimestamp;
};
