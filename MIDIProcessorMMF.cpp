
/** $VER: MIDIProcessorMMF.cpp (2025.03.19) Mobile Music File / Synthetic-music Mobile Application Format (https://docs.fileformat.com/audio/mmf/) () **/

#include "pch.h"

#include "MIDIProcessor.h"
#include "Encoding.h"

namespace midi
{

static inline uint32_t toInt32LE(const uint8_t * data)
{
    return static_cast<uint32_t>(data[0] << 24) | static_cast<uint32_t>(data[1] << 16) | static_cast<uint32_t>(data[2] << 8) | static_cast<uint32_t>(data[3]);
}

static inline uint32_t toInt32LE(std::vector<uint8_t>::const_iterator data)
{
    return static_cast<uint32_t>(data[0] << 24) | static_cast<uint32_t>(data[1] << 16) | static_cast<uint32_t>(data[2] << 8) | static_cast<uint32_t>(data[3]);
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

enum FormatType
{
    HandyPhoneStandard,         // Size: 2, Compressed: No

    MobileStandard_Compress,    // Size: 16, Compressed: Yes (Huffman encoding)
    MobileStandard_NoCompress,  // Size: 16, Compressed: No

    SEQU,                       // Size: 32, Compressed: No
};

enum SequenceType
{
    StreamSequence, // Sequence Data is one continuous sequence data. Seek Point and Phrase List are used to refer to meaningful positions in a sequence from the outside.
    SubSequence     // Sequence Data is a continuous representation of multiple phrase data. Phrase List is used to recognize individual phrases from the outside.
};

/*
     * <pre>
     * - formatType = 0x00
     * 0x00  1 ms
     * 0x01  2 ms
     * 0x02  4 ms
     * 0x03  5 ms

     * 0x04 ~ 0x0F Reserved

     * 0x10 10 ms
     * 0x11 20 ms
     * 0x12 40 ms
     * 0x13 50 ms

     * 0x14 ~ 0xFF Reserved
*/
const int TimeBaseTable[] =
{
         1,  2,  4,  5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        10, 20, 40, 50
};

class ChannelStatus
{
public:
    enum Status
    {
        OFF,
        ON
    };

    enum ChannelType
    {
        NoCare,
        Melody,
        NoMelody,
        Rhythm
    };

    int Channel; // SMAF channel

    bool KeyControlStatus;
    bool LED;
    bool Vibration;
    ChannelType Type;

    FormatType formatType; // Internal use

    ChannelStatus(int channel, uint8_t value)
    {
        Channel = channel;
        Type    = (ChannelType) (value & 0x03);

        setKeyControlStatusForHandyPhoneStandard((value & 0x08) >> 3);

        Vibration = (((value & 0x04) >> 2) != 0);

        formatType = FormatType::HandyPhoneStandard;
    }

    ChannelStatus(int channel, int value)
    {
        Channel   = channel;
        Type      = (ChannelType) (value & 0x03);
        Vibration = (((value & 0x20) >> 5) != 0);
        LED       = (((value & 0x10) >> 4) != 0);

        setKeyControlStatusForMobileStandard((value & 0xc0) >> 6);

        formatType = FormatType::MobileStandard_NoCompress;
    }

    void setKeyControlStatusForHandyPhoneStandard(int value)
    {
        KeyControlStatus = (value == 1);
    }

    void setKeyControlStatusForMobileStandard(int value)
    {
        KeyControlStatus = (value == 0x02);
    }
};

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
            uint8_t CodeType = it[2];
            uint8_t Status   = it[3];
            uint8_t Counts   = it[4];

            it += (ptrdiff_t) 5;

//          std::vector<uint8_t> Option(&it[0], &it[ChunkSize - 5]);

            std::string Text(&it[0], &it[ChunkSize - 5]);

            ::printf("Options: %s\n", Text.c_str());

            it += (ptrdiff_t) ChunkSize - 5;
        }
        else
        // Is it a "Optional Data" chunk?
        if (::memcmp(&it[0], "OPDA", 4) == 0)
        {
            it += (ptrdiff_t) 8 + ChunkSize;
        }
        else
        // Is it a "Score Track" chunk?
        if (::memcmp(&it[0], "MTR", 3) == 0)
        {
            it += (ptrdiff_t) 8;

            uint8_t FormatType       = it[0];
            uint8_t SequenceType     = it[1];
            uint8_t DurationTimeBase = it[2];
            uint8_t GateTimeTimeBase = it[3];

            it += (ptrdiff_t) 4;

            switch (FormatType)
            {
                case HandyPhoneStandard:
                {
                    uint8_t Buffer[2];
    
                    Buffer[0] = it[4];
                    Buffer[0] = it[5];

                    std::vector<ChannelStatus> ChannelStatuses;

                    for (int i = 0; i < 4; ++i)
                        ChannelStatuses.push_back(ChannelStatus(i, ((Buffer[i / 2] & (0xF0 >> (4 * (i % 2)))) >> (4 * ((i + 1) % 2)))));

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
        }
        else
        // Is it an "PCM Audio Track" chunk?
        if (::memcmp(&it[0], "ATR", 3) == 0)
        {
            it += (ptrdiff_t) 8 + ChunkSize;
        }
        else
        // Is it a "Graphics Track" chunk?
        if (::memcmp(&it[0], "GTR", 3) == 0)
        {
            it += (ptrdiff_t) 8 + ChunkSize;
        }
        else
        // Is it a "Master Track" chunk?
        if (::memcmp(&it[0], "MSTR", 4) == 0)
        {
            it += (ptrdiff_t) 8 + ChunkSize;
        }
#ifdef _DEBUG
        else
        {
            ::OutputDebugStringW(::FormatText(L"Unknown chunk \"%S\", %zu bytes\n", ChunkId.c_str(), ChunkSize).c_str());
            it += (ptrdiff_t) 8 + ChunkSize;
        }
#endif
    }

    uint16_t CRC = (uint16_t) ((it[0] << 8) | it[1]);

    return true;
}

}
