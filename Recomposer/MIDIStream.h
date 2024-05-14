
/** $VER: MIDIStream.h (2024.05.11) P. Stuer - Based on Valley Bell's rpc2mid (https://github.com/ValleyBell/MidiConverters). **/

#pragma once

#include <CppCoreCheck/Warnings.h>

#include "framework.h"

#include <MIDI.h>

#define SYXOPT_DELAY 0x01

#pragma warning(disable: 4820) // x bytes padding added after data member 'y'

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
        Ensure(0x08 + 0x06);

        WriteBE32(0x4D546864);  // write 'MThd'
        WriteBE32(0x00000006);  // Header Length

        WriteBE16(format);      // MIDI Format (0/1/2)
        WriteBE16(trackCount);  // number of tracks
        WriteBE16(ticksPerQuarter);  // Ticks per Quarter

        _TicksPerQuarter = ticksPerQuarter;
    }

    void BeginWriteMIDITrack()
    {
        Ensure(8);

        WriteBE32(0x4D54726B); // write 'MTrk'
        WriteBE32(0x00000000); // The correct timestamp will be written later.

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

    void WriteEvent(StatusCodes statusCode, const uint8_t * data, uint32_t size)
    {
        WriteTimestamp();

        Ensure(1 + 4 + size); // Worst case: 4 bytes of data length

        _State.Status = 0x00;

        Add((uint8_t) statusCode);

        EncodeVariableLengthQuantity(size);

        Add(data, size);
    }

    void WriteMetaEvent(MetaDataTypes type, uint32_t value, uint32_t size)
    {
        uint8_t Data[4]
        {
            (value >> 24) & 0xFF,
            (value >> 16) & 0xFF,
            (value >>  8) & 0xFF,
            (value >>  0) & 0xFF
        };

        WriteMetaEvent(type, Data + (4 - size), size);
    }

    void WriteMetaEvent(MetaDataTypes type, const void * data, uint32_t size)
    {
        WriteTimestamp();

        Ensure(2 + 5 + size); // Worst case: 5 bytes of data length.

        _State.Status = 0x00;

        Add(StatusCodes::MetaData);
        Add((uint8_t) type);

        EncodeVariableLengthQuantity(size);

        Add((const uint8_t *) data, size);
    }

    void WriteMetaEvent(MetaDataTypes type, const char * text)
    {
        WriteMetaEvent(type, text, (uint32_t) ::strlen(text));
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

        assert(Size != 0);

        Ensure(Size);

        _Offs += Size;

        {
            uint8_t * q = _Data + _Offs - 1;
            uint8_t * p = q;

            {
                uint32_t t = quantity;

                do
                {
                    *p-- = 0x80 | (t & 0x7F);
                    t >>= 7;
                }
                while (t != 0);
            }

            *q &= 0x7F;
        }
    }

    void Ensure(uint32_t bytesNeeded)
    {
        uint32_t NewOffs = _Offs + bytesNeeded;

        if (NewOffs <= _Size)
            return;

        const uint32_t ChunkSize = 0x8000;

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

    void Add(uint8_t value) noexcept
    {
        _Data[_Offs++] = value;
    }

    const uint8_t * GetData() const noexcept { return _Data; }
    uint32_t GetOffs() const noexcept { return _Offs; }

private:
    void WriteTimestamp()
    {
        if ((_HandleTimestamp != nullptr) && _HandleTimestamp(this, &_State.Timestamp))
            return;

        EncodeVariableLengthQuantity(_State.Timestamp);

        _State.Timestamp = 0;
    }

    void WriteEventInternal(StatusCodes statusCode, uint8_t value1, uint8_t value2)
    {
        WriteTimestamp();

        Ensure(3);

        const uint8_t Status = (uint8_t) (statusCode | _State.Channel);

        switch (statusCode & 0xF0)
        {
            case StatusCodes::NoteOff:
            case StatusCodes::NoteOn:
            case StatusCodes::KeyPressure:
            case StatusCodes::ControlChange:
            case StatusCodes::PitchBendChange:
            {
                // Only add the status if it is different from the running status.
                if (_State.Status != Status)
                {
                    _State.Status = Status;

                    Add(Status);
                }

                Add(value1);
                Add(value2);
                break;
            }

            case StatusCodes::ProgramChange:
            case StatusCodes::ChannelPressure:
            {
                // Only add the status if it is different from the running status.
                if (_State.Status != Status)
                {
                    _State.Status = Status;

                    Add(Status);
                }

                Add(value1);
                break;
            }

            case StatusCodes::SysEx: // Meta Event: Track End
            {
                _State.Status = 0;

                Add((uint8_t) statusCode);
                Add(value1);
                Add(value2);
                break;
            }

            default:
                break;
        }
    }

    void WriteBE32(uint32_t value)
    {
        Add((value >> 24) & 0xFF);
        Add((value >> 16) & 0xFF);
        Add((value >>  8) & 0xFF);
        Add( value        & 0xFF);
    }

    void WriteBE16(uint16_t value)
    {
        Add((value >> 8) & 0xFFu);
        Add( value       & 0xFFu);
    }

    void Add(const uint8_t * data, uint32_t size) noexcept
    {
        ::memcpy(_Data + _Offs, data, size);
        _Offs += size;
    }

private:
    uint8_t * _Data;
    uint32_t _Size;
    uint32_t _Offs;

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
