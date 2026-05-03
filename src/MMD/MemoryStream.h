
/** $VER: MemoryStream.h (2026.05.03) P. Stuer - Based on Valley Bell's mmd2mid (https://github.com/ValleyBell/MidiConverters). **/

#pragma once

#include <CppCoreCheck\Warnings.h>

#pragma warning(disable: 4100 4514 4625 4626 4710 4711 4738 5045 ALL_CPPCORECHECK_WARNINGS)

#include <cstdint>

#include <malloc.h>
#include <string.h>

namespace mmd
{

inline void WriteBE32(uint8_t * buffer, uint32_t value);
inline void WriteBE16(uint8_t * buffer, uint16_t value);

struct midi_state_t
{
    uint32_t TrackOffset;
    uint32_t Delay;         // Delay until next event
    uint8_t Channel;
    uint8_t RunningStatus;
};

class memory_stream_t
{
public:
    typedef uint8_t (* GetDelayCallback)(memory_stream_t * file, uint32_t * delay);

    memory_stream_t() : Data(), Size(), Offset(), _GetDelay() {}

    memory_stream_t(size_t initialSize, GetDelayCallback callback) : Data((uint8_t *) ::malloc(initialSize)), Size(initialSize), Offset(), _GetDelay(callback) {}

    void WriteHeader(uint16_t format, uint16_t tracks, uint16_t resolution) noexcept;
    void WriteTrackBegin(midi_state_t * state) noexcept;
    void WriteTrackEnd(midi_state_t * state) noexcept;

    void WriteEvent(midi_state_t * state, uint8_t event, uint8_t param1, uint8_t param2) noexcept;
    void WriteEvent(midi_state_t * state, uint8_t event, const void * data, uint32_t size) noexcept;
    void WriteMetaEvent(midi_state_t * state, uint8_t metaType, const void * data, uint32_t dataLen) noexcept;

    void WriteEventOpt(midi_state_t * state, uint8_t event, uint8_t param1, uint8_t param2) noexcept;

    void WriteDelay(uint32_t * delay) noexcept;
    void WriteVariableLengthValue(uint32_t value) noexcept;

    void Grow(uint32_t bytesNeeded) noexcept;

public:
    uint8_t * Data;
    size_t Size;
    size_t Offset;

    // Note: _GetDelay can be used to inject additional events. IMPORTANT: You must not call any of the Write*Event() functions or WriteDelay() within this function.
    // Optional callback for injecting raw data before writing delays. Returning non-zero makes it skip writing the delay.
    GetDelayCallback _GetDelay = nullptr;
};

void memory_stream_t::WriteHeader(uint16_t format, uint16_t trackCount, uint16_t resolution) noexcept
{
    Grow(0x08 + 0x06);

    WriteBE32(&Data[Offset + 0x00], 0x4D546864);    // 'MThd'
    WriteBE32(&Data[Offset + 0x04], 0x00000006);    // Header Length
    Offset += 0x08;

    WriteBE16(&Data[Offset + 0x00], format);        // MIDI Format (0/1/2)
    WriteBE16(&Data[Offset + 0x02], trackCount);    // number of tracks
    WriteBE16(&Data[Offset + 0x04], resolution);    // Ticks per Quarter
    Offset += 0x06;
}

void memory_stream_t::WriteTrackBegin(midi_state_t * state) noexcept
{
    Grow(0x08);

    WriteBE32(&Data[Offset + 0x00], 0x4D54726B);    // 'MTrk'
    WriteBE32(&Data[Offset + 0x04], 0x00000000);    // Write dummy length
    Offset += 0x08;

    state->TrackOffset   = (uint32_t) Offset;
    state->Delay         = 0;
    state->RunningStatus = 0x00;
}

void memory_stream_t::WriteTrackEnd(midi_state_t * state) noexcept
{
    const uint32_t n = (uint32_t) (Offset - state->TrackOffset);

    WriteBE32(&Data[state->TrackOffset - 0x04], n);
}

void memory_stream_t::WriteEvent(midi_state_t * state, uint8_t event, uint8_t param1, uint8_t param2) noexcept
{
    state->RunningStatus = 0x00;

    WriteEventOpt(state, event, param1, param2);
}

void memory_stream_t::WriteEvent(midi_state_t * state, uint8_t event, const void * data, uint32_t size) noexcept
{
    WriteDelay(&state->Delay);

    Grow(0x01 + 0x04 + size); // Worst case

    state->RunningStatus = 0x00;

    Data[Offset++] = event;

    WriteVariableLengthValue(size);

    ::memcpy(&Data[Offset], data, size);
    Offset += size;
}

void memory_stream_t::WriteMetaEvent(midi_state_t * state, uint8_t metaType, const void * data, uint32_t size) noexcept
{
    WriteDelay(&state->Delay);

    Grow(0x02 + 0x05 + size); // worst case

    state->RunningStatus = 0x00;

    Data[Offset++] = 0xFF;
    Data[Offset++] = metaType;

    WriteVariableLengthValue(size);

    ::memcpy(&Data[Offset], data, size);
    Offset += size;
}

void memory_stream_t::WriteEventOpt(midi_state_t * state, uint8_t event, uint8_t param1, uint8_t param2) noexcept
{
    WriteDelay(&state->Delay);

    const uint8_t Status = (uint8_t) (event | state->Channel);

    Grow(3);

    switch (event & 0xF0)
    {
        case 0x80:
        case 0x90:
        case 0xA0:
        case 0xB0:
        case 0xE0:
        {
//          if ((param1 & 0x80) || (param2 & 0x80))
//              ::printf("Warning: Pos 0x%04X, event %02X %02X %02X: Values out-of-range!\n", (uint32_t) Offset, Status, param1, param2);

            if (state->RunningStatus != Status)
            {
                state->RunningStatus = Status;

                Data[Offset++] = Status;
            }

            Data[Offset++] = param1;
            Data[Offset++] = param2;
            break;
        }

        case 0xC0:
        case 0xD0:
        {
//          if (param1 & 0x80)
//              ::printf("Warning: Pos 0x%04X, event %02X %02X: Values out-of-range!\n", (uint32_t) Offset, Status, param1);

            if (state->RunningStatus != Status)
            {
                state->RunningStatus = Status;

                Data[Offset++] = Status;
            }

            Data[Offset++] = param1;
            break;
        }

        case 0xF0: // Meta Event
        {
            state->RunningStatus = 0x00;

            Data[Offset++] = event;
            Data[Offset++] = param1;
            Data[Offset++] = param2;
            break;
        }

        default:
            break;
    }
}

void memory_stream_t::WriteDelay(uint32_t * delay) noexcept
{
    if ((_GetDelay != nullptr) && _GetDelay(this, delay))
        return;

    WriteVariableLengthValue(*delay);
    *delay = 0;

    return;
}

void memory_stream_t::WriteVariableLengthValue(uint32_t value) noexcept
{
    uint8_t ByteCount = 0;

    {
        uint32_t v = value;

        do
        {
            v >>= 7;
            ByteCount++;
        }
        while (v);
    }

    Grow(ByteCount);

    uint8_t * p = &Data[Offset];

    {
        uint32_t v = value;
        uint32_t n = ByteCount;

        do
        {
            p[--n] = 0x80u | (v & 0x7Fu);
            v >>= 7;
        }
        while (v);
    }

    p[ByteCount - 1] &= 0x7Fu;

    Offset += ByteCount;
}

void memory_stream_t::Grow(uint32_t bytesNeeded) noexcept
{
    const size_t NewOffset = Offset + bytesNeeded;

    if (NewOffset <= Size)
        return;

    const size_t REALLOC_STEP = 0x8000u;

    while (NewOffset > Size)
        Size += REALLOC_STEP;

    auto p = (uint8_t *) realloc(Data, Size);

    if (p != nullptr)
        Data = p;
}

/// <summary>
/// Writes a 32-bit unsigned integer in big-endian format to the specified buffer.
/// </summary>
inline void WriteBE32(uint8_t * data, uint32_t value)
{
    data[0x00] = (value >> 24) & 0xFFu;
    data[0x01] = (value >> 16) & 0xFFu;
    data[0x02] = (value >>  8) & 0xFFu;
    data[0x03] = (value >>  0) & 0xFFu;
}

/// <summary>
/// Writes a 16-bit unsigned integer in big-endian format to the specified buffer.
/// </summary>
/// <param name="data"></param>
/// <param name="value"></param>
inline void WriteBE16(uint8_t * data, uint16_t value)
{
    data[0x00] = (value >> 8) & 0xFFu;
    data[0x01] = (value >> 0) & 0xFFu;
}

}
