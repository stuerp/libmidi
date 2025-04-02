
/** $VER: MIDIProcessorMMF.cpp (2025.03.31) Mobile Music File / Synthetic-music Mobile Application Format (https://docs.fileformat.com/audio/mmf/) (SMAF) **/

/**######## WORK IN PROGRESS #########*/

#include "pch.h"

#include "MIDIProcessor.h"
#include "Encoding.h"

#include <span>

#include "MMF.h"

namespace midi
{

struct state_t
{
    uint8_t Format;
    uint8_t SequenceType;
    uint8_t DurationBase;
    uint8_t GateTimeBase;

    bool    IsMTSU;         // Is it a Setup track?
    uint8_t ChannelOffset;
};

template <class T> inline static uint32_t toInt32LE(T data)
{
    return static_cast<uint32_t>(data[0] << 24) | static_cast<uint32_t>(data[1] << 16) | static_cast<uint32_t>(data[2] << 8) | static_cast<uint32_t>(data[3]);
}

/// <summary>
/// Gets an HPS-encoded value.
/// </summary>
static uint32_t GetHPSValue(std::span<const uint8_t>::iterator data)
{
    uint32_t Value = 0;

    if (data[0] & 0x80)
        Value = (uint32_t) (((*data++ & 0x7F) + 1) << 7);

    Value |= *data++;

    return Value;
}

/// <summary>
/// Gets an HPS-encoded value.
/// </summary>
static uint32_t GetHPSValueEx(std::span<const uint8_t>::iterator & data)
{
    uint32_t Value = 0;

    if (data[0] & 0x80)
        Value = (uint32_t) (((*data++ & 0x7F) + 1) << 7);

    Value |= *data++;

    return Value;
}

/// <summary>
/// Returns true if the byte vector contains MMF data.
/// </summary>
bool processor_t::IsMMF(std::vector<uint8_t> const & data) noexcept
{
    if (data.size() < 8)
        return false;

    if (::memcmp(data.data(), "MMMD", 4) != 0)
        return false;

    const uint32_t Size = toInt32LE(data.data() + 4);

    if (data.size() < (size_t) Size + 8)
        return false;

    return true;
};

enum Format : uint8_t
{
    HandyPhoneStandard,         // Size:  2, Compressed: No

    MobileStandard_Compress,    // Size: 16, Compressed: Yes (Huffman encoding)
    MobileStandard_NoCompress,  // Size: 16, Compressed: No

    SEQU,                       // Size: 32, Compressed: No
};

enum SequenceType : uint8_t
{
    StreamSequence, // Sequence Data is one continuous sequence data. Seek Point and Phrase List are used to refer to meaningful positions in a sequence from the outside.
    SubSequence     // Sequence Data is a continuous representation of multiple phrase data. Phrase List is used to recognize individual phrases from the outside.
};

class ChannelStatus
{
public:
    enum ChannelType : uint8_t
    {
        NoCare,
        Melody,
        NoMelody,
        Rhythm
    };

    int Channel; // SMAF channel

    bool KeyControlStatus;
    bool VibrationStatus;
    bool LED;
    ChannelType Type;

    ChannelStatus(int channel, bool keyControlstatus, bool vibrationStatus, int type) noexcept
    {
        Channel = channel;

        KeyControlStatus = keyControlstatus;
        VibrationStatus  = vibrationStatus;
        LED              = false;
        Type             = (ChannelType) type;

        _Format = Format::HandyPhoneStandard;
    }

    ChannelStatus(int channel, int value) noexcept
    {
        Channel = channel;

        KeyControlStatus = false;
        VibrationStatus  = (((value & 0x20) >> 5) != 0);
        LED              = (((value & 0x10) >> 4) != 0);
        Type             = (ChannelType) (value & 0x03);

        _Format = Format::MobileStandard_NoCompress;
    }

private:
    Format _Format; // Internal use
};

/// <summary>
/// Converts HPS data.
/// </summary>
static bool ConvertHPS(const std::span<const uint8_t> & data, state_t & state, container_t & container)
{
    track_t Track;

    uint32_t RunningTime = 0;
    int8_t OctaveShift[4] = { }; // 4 HPS channels.

    auto it = data.begin();
    const auto Tail = data.end();

    while (it < Tail)
    {
        uint32_t Delta = !state.IsMTSU ? GetHPSValue(it) * state.DurationBase : 0;

        if (it[0] == 0x00 && it[1] == 0x00 && it[2] == 0x00 && it[3] == 0x00)
        {
            const uint8_t Data[] = { StatusCodes::MetaData, MetaDataTypes::EndOfTrack };

            Track.AddEvent(event_t(RunningTime, event_t::Extended, (uint32_t) 0, Data, 2));
            break;
        }

        if (!state.IsMTSU)
            GetHPSValueEx(it);

        // Exclusive event?
        if (it[0] == 0xFF && it[1] == 0xF0)
        {
            if ((it[2] == 0x12 || it[2] == 0x1C) && (it[3] == 0x43) && (it[4] == 0x03) && (it[9] == 0x01))
            {
                CHPARAM chp = { };
                OPPARAM opp[4] = { };

                if (GetHPSExclusiveFM(&it[2], &chp, opp))
                {
                    std::vector<uint8_t> Temp(48);

                    size_t Size = setMA3Exclusive(Temp.data(), &chp, opp);

                    Track.AddEvent(event_t(RunningTime, event_t::Extended, 0, Temp.data(), Size));
                }
                else
                {
                    // Conversion failed. Just copy the SysEx.
                    const size_t Size = it[2] + 2u;

                    std::vector<uint8_t> Temp(Size);

                    std::copy(it + 1, it + 1 + (ptrdiff_t) (Size), Temp.begin());

                    Track.AddEvent(event_t(RunningTime, event_t::Extended, 0, Temp.data(), Temp.size()));
                }
            }
            else
            {
                const size_t Size = it[2] + 2u;

                std::vector<uint8_t> Temp(Size);

                std::copy(it + 1, it + 1 + (ptrdiff_t) (Size), Temp.begin());

                Track.AddEvent(event_t(RunningTime, event_t::Extended, 0, Temp.data(), Temp.size()));
            }

            it += 2 + it[2] + 1;
        }
        else
        // NOP
        if (it[0] == 0xFF && it[1] == 0x00)
        {
            it += 2;
        }
        else
        {
            // Note
            if (it[0] != 0x00)
            {
                uint8_t Channel = (it[0] >> 6 & 0x03u);
                uint8_t Octave  =  it[0] >> 4 & 0x03u;
                uint8_t Note    =  it[0]      & 0x0Fu;

                Note += 36u + ((Octave + OctaveShift[Channel]) * 12u); // MIDI's 69 = 440Hz(A) = Oct2,9

                const uint8_t Data[2] = { Note, 0x7Fu };

                Track.AddEvent(event_t(RunningTime, event_t::NoteOn,  (uint32_t) Channel + state.ChannelOffset, Data, 2));
                it += 1;

                Track.AddEvent(event_t(RunningTime + Delta + (GetHPSValueEx(it) * state.GateTimeBase), event_t::NoteOff, (uint32_t) Channel + state.ChannelOffset, Data, 2));
            }
            else
            {
                uint8_t Channel = (it[1] >> 6 & 0x03u) + state.ChannelOffset;

                if ((it[1] & 0x30) == 0x30)
                {
                    switch (it[1] & 0x0F)
                    {
                        // Program Change
                        case 0x00:
                        {
                            Track.AddEvent(event_t(RunningTime, event_t::ProgramChange, Channel, &it[2], 1));
                            it += 3;
                            break;
                        }

                        // Bank Select
                        case 0x01:
                        {
                            uint8_t Data[2] = { 0x00, (it[2] & 0x80u) ? 0x7Du : 0x7Cu };

                            Track.AddEvent(event_t(RunningTime, event_t::ControlChange, Channel, Data, 2)); // MSB

                            Data[0] = 0x20u;
                            Data[1] = it[2] & 0x7Fu;

                            Track.AddEvent(event_t(RunningTime, event_t::ControlChange, Channel, Data, 2)); // LSB

                            it += 3;
                            break;
                        }

                        // Octave Shift
                        case 0x02:
                        {
                            const size_t i = ((size_t) it[0] >> 6) & 0x03u;

                            if (0x01 <= it[2] && it[2] <= 0x04)
                                OctaveShift[i] = (int8_t) it[2];
                            else
                            if (0x81 <= it[2] && it[2] <= 0x84)
                                OctaveShift[i] = (int8_t) -(it[2] - 0x80);

                            it += 3;
                            break;
                        }

                        // Modulation
                        case 0x03:
                        {
                            const uint8_t Data[2] = { 0x01, it[2] };

                            Track.AddEvent(event_t(RunningTime, event_t::ControlChange, Channel, Data, 2));
                            it += 3;
                            break;
                        }

                        // Pitch Bend
                        case 0x04:
                        {
                            const uint8_t Data[2] = { 0x00, it[2] };

                            Track.AddEvent(event_t(RunningTime, event_t::PitchBendChange, Channel, Data, 2));
                            it += 3;
                            break;
                        }

                        // Volume
                        case 0x07:
                        {
                            const uint8_t Data[2] = { 0x07, it[2] };

                            Track.AddEvent(event_t(RunningTime, event_t::ControlChange, Channel, Data, 2));
                            it += 3;
                            break;
                        }

                        // Pan
                        case 0x0A:
                        {
                            const uint8_t Data[2] = { 0x0A, it[2] };

                            Track.AddEvent(event_t(RunningTime, event_t::ControlChange, Channel, Data, 2));
                            it += 3;
                            break;
                        }

                        // Expression
                        case 0x0B:
                        {
                            const uint8_t Data[2] = { 0x0B, it[2] };

                            Track.AddEvent(event_t(RunningTime, event_t::ControlChange, Channel, Data, 2));
                            it += 3;
                            break;
                        }

                        default:
                            ::printf("Unknown opcode 0x%02X\n", it[1] & 0x0F);
                    }
                }
                else
                {
                    switch (it[1] & 0x30)
                    {
                        // Expression
                        case 0x00:
                        {
                            const uint8_t Lookup[] =
                            {
                                0x00 /* Reserved */,
                                0x00, 0x1F,
                                0x27, 0x2F,
                                0x37, 0x3F,
                                0x47, 0x4F,
                                0x57, 0x5F,
                                0x67, 0x6F,
                                0x77, 0x7F,
                                0x00 /* Reserved */
                            };

                            const uint8_t Data[2] = { 0x0B, Lookup[it[1] & 0x0F] };

                            Track.AddEvent(event_t(RunningTime, event_t::ControlChange, Channel, Data, 2));
                            it += 2;
                            break;
                        }

                        // Pitch Bend
                        case 0x10:
                        {
                            const uint8_t Lookup[] =
                            {
                                0x00 /* Reserved */,
                                0x08, 0x10,
                                0x18, 0x20,
                                0x28, 0x30,
                                0x38, 0x40,
                                0x48, 0x50,
                                0x58, 0x60,
                                0x68, 0x70,
                                0x00 /* Reserved */
                            };

                            const uint8_t Data[2] = { 0x00, Lookup[it[1] & 0xF0] };

                            Track.AddEvent(event_t(RunningTime, event_t::PitchBendChange, Channel, Data, 2));
                            it += 2;
                            break;
                        }

                        // Modulation
                        case 0x20:
                        {
                            const uint8_t Lookup[] =
                            {
                                0x00 /* Reserved */,
                                0x08, 0x10,
                                0x18, 0x20,
                                0x28, 0x30,
                                0x38, 0x40,
                                0x48, 0x50,
                                0x60, 0x68,
                                0x70, 0x7F,
                                0x00 /* Reserved */
                            };

                            const uint8_t Data[2] = { 0x01, Lookup[it[1] & 0xF0] };

                            Track.AddEvent(event_t(RunningTime, event_t::ControlChange, Channel, Data, 2));
                            it += 2;
                            break;
                        }

                        default:
                            ::printf("Unknown opcode 0x%02X\n", it[1] & 0x30);
                    }
                }
            }
        }

        RunningTime += Delta;
    }

    container.AddTrack(Track);

    return true;
}

/// <summary>
/// Processes an MTR chunk.
/// </summary>
static bool ProcessMTR(const std::span<const uint8_t> & data, state_t & state, container_t & container)
{
    auto it = data.begin();
    const auto Tail = data.end();

    // Decode the track header.
    {
        state.Format       = it[0];
        state.SequenceType = it[1];
        state.DurationBase = it[2];
        state.GateTimeBase = it[3];

        ::printf("- Format        : %02X\n", state.Format);
        ::printf("- Sequence      : %02X\n", state.SequenceType);
        ::printf("- Duration base : %02X\n", state.DurationBase);
        ::printf("- Gate Time base: %02X\n", state.GateTimeBase);

        switch (state.DurationBase)
        {
            case 0x00: state.DurationBase =  1; break;
            case 0x01: state.DurationBase =  2; break;
            case 0x02: state.DurationBase =  4; break;
            case 0x03: state.DurationBase =  5; break;
            case 0x10: state.DurationBase = 10; break;
            case 0x11: state.DurationBase = 20; break;
            case 0x12: state.DurationBase = 40; break;
            case 0x13: state.DurationBase = 50; break;

            default:
                return false;
        }

        switch (state.GateTimeBase)
        {
            case 0x00: state.GateTimeBase =  1; break;
            case 0x01: state.GateTimeBase =  2; break;
            case 0x02: state.GateTimeBase =  4; break;
            case 0x03: state.GateTimeBase =  5; break;
            case 0x10: state.GateTimeBase = 10; break;
            case 0x11: state.GateTimeBase = 20; break;
            case 0x12: state.GateTimeBase = 40; break;
            case 0x13: state.GateTimeBase = 50; break;

            default:
                return false;
        }

        switch (state.Format)
        {
            case HandyPhoneStandard:
            {
                std::vector<ChannelStatus> ChannelStatuses;

                ChannelStatuses.push_back(ChannelStatus(0, (it[0] & 0x80) == 0x80, (it[0] & 0x40) == 0x40, (it[0] >> 4) & 0x03));
                ChannelStatuses.push_back(ChannelStatus(1, (it[0] & 0x08) == 0x08, (it[0] & 0x04) == 0x04, (it[0]     ) & 0x03));
                ChannelStatuses.push_back(ChannelStatus(2, (it[1] & 0x80) == 0x80, (it[1] & 0x40) == 0x40, (it[1] >> 4) & 0x03));
                ChannelStatuses.push_back(ChannelStatus(3, (it[1] & 0x08) == 0x08, (it[1] & 0x04) == 0x04, (it[1]     ) & 0x03));

                it += (ptrdiff_t) 2;
                break;
            }

            case MobileStandard_Compress:
            {
                uint8_t Buffer[16];

                std::vector<ChannelStatus> ChannelStatuses;

                for (int i = 0; i < 16; ++i)
                    ChannelStatuses.push_back(ChannelStatus(i, (int) Buffer[i]));

                it += (ptrdiff_t) 16;
                break;
            }

            case MobileStandard_NoCompress:
            {
                uint8_t Buffer[16];

                std::vector<ChannelStatus> ChannelStatuses;

                for (int i = 0; i < 16; ++i)
                    ChannelStatuses.push_back(ChannelStatus(i, (int) Buffer[i]));

                it += (ptrdiff_t) 16;
                break;
            }

            case SEQU:
            {
                uint8_t Buffer[32];

                std::vector<ChannelStatus> ChannelStatuses;

                for (int i = 0; i < 32; ++i)
                    ChannelStatuses.push_back(ChannelStatus(i, (int) Buffer[i]));

                it += (ptrdiff_t) 32;
                break;
            }
        }

        it += (ptrdiff_t) 4;
    }

    while (it < Tail)
    {
        if (Tail - it < 8)
            break;

        const uint32_t ChunkSize = toInt32LE(it + 4);

        if ((Tail - it) < (ptrdiff_t) ChunkSize)
            throw midi::exception("Insufficient SMAF data");

        const std::string ChunkId(it, it + 4);

        if (::memcmp(&it[0], "Mtsu", 4) == 0)
        {
            ::_putws(::FormatText(L"Chunk \"%S\", %zu bytes", ChunkId.c_str(), ChunkSize).c_str());

            state.IsMTSU = true;

            if (state.Format == 0)
                ConvertHPS(std::span<const uint8_t>(&it[8], ChunkSize), state, container);
        }
        else
        if (::memcmp(&it[0], "Mtsq", 4) == 0)
        {
            ::_putws(::FormatText(L"Chunk \"%S\", %zu bytes", ChunkId.c_str(), ChunkSize).c_str());

            state.IsMTSU = false;

            if (state.Format == 0)
                ConvertHPS(std::span<const uint8_t>(&it[8], ChunkSize), state, container);
        }
#ifdef _DEBUG
        else
        {
            ::_putws(::FormatText(L"Unknown chunk \"%S\", %zu bytes", ChunkId.c_str(), ChunkSize).c_str());
        }
#endif
        it += (ptrdiff_t) 8 + ChunkSize;
    }

    return true;
}

/// <summary>
/// Processes a byte vector with MMF data.
/// </summary>
bool processor_t::ProcessMMF(std::vector<uint8_t> const & data, container_t & container)
{
    if (data.size() < 8)
        throw midi::exception("Insufficient SMAF data");

    const uint32_t Size = toInt32LE(data.data() + 4);

    if (data.size() < (size_t) Size + 8)
        throw midi::exception("Insufficient SMAF data");

    state_t State = { };

    container.Initialize(1u, 500);

    auto it = data.begin() + 8; // Skip past "MMMD" <size>.

    const auto Tail = data.begin() + (ptrdiff_t) Size;

    while (it < Tail)
    {
        if (Tail - it < 8)
            break;

        const ptrdiff_t ChunkSize = (ptrdiff_t) toInt32LE(it + 4);

        if ((Tail - it) < ChunkSize)
            throw midi::exception("Insufficient SMAF data");

        const std::string ChunkId(it, it + 4);

        // Is it a "Contents Info" chunk?
        if (::memcmp(&it[0], "CNTI", 4) == 0)
        {
            it += (ptrdiff_t) 8;

            uint8_t Class    = it[0]; // 0: "Yamaha"
            uint8_t Type     = it[1];
            uint8_t Encoding = it[2];
            uint8_t Status   = it[3];
            uint8_t Counts   = it[4];

            ::puts("Contents:");

            ::printf("- Class: %s\n", (Class == 0x00) ? "Yamaha" : "Other");

            if ((0x00 <= Type && Type <= 0x0F) || (0x30 <= Type && Type <= 0x33))
                ::printf("- Type: Ringtone (0x%02X)\n", Type);
            else
            if ((0x10 <= Type && Type <= 0x1F) || (0x40 <= Type && Type <= 0x42))
                ::printf("- Type: Karaoke (0x%02X)\n", Type);
            else
            if ((0x20 <= Type && Type <= 0x2F) || (0x50 <= Type && Type <= 0x53))
                ::printf("- Type: CM (0x%02X)\n", Type);
            else
                ::printf("- Type: Reserved (0x%02X)\n", Type);

            switch (Encoding)
            {
                case 0x00: ::puts("- Encoding Shift-JIS"); break;
                case 0x01: ::puts("- Encoding Latin-1"); break;
                case 0x02: ::puts("- Encoding EUC-KR"); break;
                case 0x03: ::puts("- Encoding GB-2312"); break;
                case 0x04: ::puts("- Encoding Big5"); break;
                case 0x05: ::puts("- Encoding KOI8-R"); break;
                case 0x06: ::puts("- Encoding TCVN-5773:1993"); break;
                case 0x20: ::puts("- Encoding UCS-2"); break;
                case 0x21: ::puts("- Encoding UCS-4"); break;
                case 0x22: ::puts("- Encoding UTF-7"); break;
                case 0x23: ::puts("- Encoding UTF-8"); break;
                case 0x24: ::puts("- Encoding UTF-16"); break;
                case 0x25: ::puts("- Encoding UTF-32"); break;
                default:   ::printf("- Encoding Unknown (0x%02X)\n", Encoding); break;
            }

            if (Status & 0x04)
                ::printf("- Status: Edit NG, ");
            else
                ::printf("- Status: Edit OK, ");

            if (Status & 0x02)
                ::printf("Save NG, ");
            else
                ::printf("Save OK, ");

            if (Status & 0x01)
                ::printf("Trans NG\n");
            else
                ::printf("Trans OK\n");

            ::printf("- Copy Count: %d\n", Counts);

            it += (ptrdiff_t) 5;

            std::string Text(&it[0], &it[ChunkSize - 5]);

            ::printf("Optional data: %s\n", Text.c_str());

            it += (ptrdiff_t) ChunkSize - 5;
        }
        else
        // Is it a "Optional Data" chunk?
        if (::memcmp(&it[0], "OPDA", 4) == 0)
        {
            ::puts("OPDA chunk");

            it += (ptrdiff_t) 8 + ChunkSize;
        }
        else
        // Is it a "Score Track" chunk?
        if (::memcmp(&it[0], "MTR", 3) == 0)
        {
            ::_putws(::FormatText(L"Chunk \"MTR_\", %zu bytes", ChunkSize).c_str());

            it += (ptrdiff_t) 8;

            ProcessMTR(std::span<const uint8_t>(&it[0], (size_t) ChunkSize), State, container);
            State.ChannelOffset += 4;

            it += (ptrdiff_t) ChunkSize;
        }

        else
        // Is it an "PCM Audio Track" chunk?
        if (::memcmp(&it[0], "ATR", 3) == 0)
        {
            ::puts("ATRx chunk");

            it += (ptrdiff_t) 8 + ChunkSize;
        }
        else
        // Is it a "Graphics Track" chunk?
        if (::memcmp(&it[0], "GTR", 3) == 0)
        {
            ::puts("GTRx chunk");

            it += (ptrdiff_t) 8 + ChunkSize;
        }
        else
        // Is it a "Master Track" chunk?
        if (::memcmp(&it[0], "MSTR", 4) == 0)
        {
            ::puts("MSTR chunk");

            it += (ptrdiff_t) 8 + ChunkSize;
        }
#ifdef _DEBUG
        else
        {
            ::_putws(::FormatText(L"Unknown chunk \"%S\", %zu bytes", ChunkId.c_str(), ChunkSize).c_str());
            it += (ptrdiff_t) 8 + ChunkSize;
        }
#endif
    }

//  uint16_t CRC = (uint16_t) ((it[0] << 8) | it[1]);

    return true;
}

}
