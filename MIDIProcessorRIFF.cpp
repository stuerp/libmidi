
/** $VER: MIDIProcessorRIFF.cpp (2024.08.04) **/

#include "framework.h"

#include "MIDIProcessor.h"

static inline uint32_t toInt32LE(const uint8_t * data)
{
    return static_cast<uint32_t>(data[0]) | static_cast<uint32_t>(data[1] << 8) | static_cast<uint32_t>(data[2] << 16) | static_cast<uint32_t>(data[3] << 24);
}

static inline uint32_t toInt32LE(std::vector<uint8_t>::const_iterator data)
{
    return static_cast<uint32_t>(data[0]) | static_cast<uint32_t>(data[1] << 8) | static_cast<uint32_t>(data[2] << 16) | static_cast<uint32_t>(data[3] << 24);
}

bool midi_processor_t::IsRIFF(std::vector<uint8_t> const & data)
{
    if (data.size() < 20)
        return false;

    if (::memcmp(data.data(), "RIFF", 4) != 0)
        return false;

    uint32_t Size = (uint32_t) (data[4] | (data[5] << 8) | (data[6] << 16) | (data[7] << 24));

    if ((Size < 12) || (data.size() < Size + 8))
        return false;

    if (::memcmp(&data[8], "RMID", 4) != 0 || ::memcmp(&data[12], "data", 4) != 0)
        return false;

    uint32_t DataSize = toInt32LE(&data[16]);

    if ((DataSize < 18) || data.size() < DataSize + 20 || Size < DataSize + 12)
        return false;

    std::vector<uint8_t> Data;

    Data.assign(data.begin() + 20, data.begin() + 20 + 18);

    return IsSMF(Data);
}
/*
bool MIDIProcessor::GetTrackCountFromRIFF(std::vector<uint8_t> const & data, size_t & trackCount)
{
    trackCount = 0;

    uint32_t Size = (uint32_t)(data[4] | (data[5] << 8) | (data[6] << 16) | (data[7] << 24));

    auto it = data.begin() + 12;

    auto body_end = data.begin() + 8 + Size;

    std::vector<uint8_t> extra_buffer;

    while (it != body_end)
    {
        if (body_end - it < 8)
            return false;

        uint32_t ChunkSize = (uint32_t)(it[4] | (it[5] << 8) | (it[6] << 16) | (it[7] << 24));

        if ((uint32_t) (body_end - it) < ChunkSize)
            return false;

        if (::memcmp(&it[0], "data", 4) == 0)
        {
            std::vector<uint8_t> Data;

            Data.assign(it + 8, it + 8 + ChunkSize);

            return GetTrackCount(Data, trackCount);
        }
        else
        {
            it += ChunkSize;

            if (ChunkSize & 1 && it != body_end)
                ++it;
        }
    }

    return false;
}
*/
static const char * RIFFToTagMap[][2] =
{
    { "IALB", "album" },
    { "IARL", "archival_location" },
    { "IART", "artist" },
    { "ITRK", "tracknumber" },
    { "ICMS", "commissioned" },
    { "ICMP", "composer" },
    { "ICMT", "comment" },
    { "ICOP", "copyright" },
    { "ICRD", "creation_date" },
    { "IENC", "encoding" },
    { "IENG", "engineer" },
    { "IGNR", "genre" },
    { "IKEY", "keywords" },
    { "IMED", "medium" },
    { "INAM", "title" },
    { "IPRD", "product" },
    { "ISBJ", "subject" },
    { "ISFT", "software" },
    { "ISRC", "source" },
    { "ISRF", "source_form" },
    { "ITCH", "technician" }
};

bool midi_processor_t::ProcessRIFF(std::vector<uint8_t> const & data, midi_container_t & container)
{
    bool FoundDataChunk = false;
    bool FoundInfoChunk = false;

    midi_metadata_table_t MetaData;
    std::vector<uint8_t> Temp;

    uint32_t Size = (uint32_t) (data[4] | (data[5] << 8) | (data[6] << 16) | (data[7] << 24));

    const auto Tail = data.begin() + 8 + (int) Size;

    auto it = data.begin() + 12;

    while (it < Tail)
    {
        if (Tail - it < 8)
            return false;

        uint32_t ChunkSize = toInt32LE(it + 4);

        if ((uint32_t) (Tail - it) < ChunkSize)
            return false;

        // Is it a "data" chunk?
        if (::memcmp(&it[0], "data", 4) == 0)
        {
            if (FoundDataChunk)
                return false; // throw exception_io_data( "Multiple RIFF data chunks found" );

            std::vector<uint8_t> Data;

            Data.assign(it + 8, it + 8 + (int) ChunkSize);

            if (!ProcessSMF(Data, container))
                return false;

            FoundDataChunk = true;

            it += 8 + (int) ChunkSize;

            if ((ChunkSize & 1) && (it < Tail))
                ++it;
        }
        else
        // Is it a "DISP" chunk?
        if (::memcmp(&it[0], "DISP", 4) == 0)
        {
            uint32_t type = toInt32LE(it + 8);

            if (type == CF_TEXT)
            {
                Temp.resize(ChunkSize - 4 + 1);
                std::copy(it + 12, it + 8 + (int) ChunkSize, Temp.begin());
                Temp[ChunkSize - 4] = '\0';

                MetaData.AddItem(midi_metadata_item_t(0, "display_name", (const char *) Temp.data()));
            }

            it += (int) (8 + ChunkSize);

            if ((ChunkSize & 1) && (it < Tail))
                ++it;
        }
        else
        // Is it a "LIST" chunk?
        if (::memcmp(&it[0], "LIST", 4) == 0)
        {
            auto ChunkTail = it + (int) (8 + ChunkSize);

            // Is it a "INFO" chunk?
            if (::memcmp(&it[8], "INFO", 4) == 0)
            {
                if (FoundInfoChunk)
                    return false; // throw exception_io_data( "Multiple RIFF LIST INFO chunks found" );

                if (ChunkTail - it < 12)
                    return false;

                it += 12;

                while (it != ChunkTail)
                {
                    if (ChunkTail - it < 4)
                        return false;

                    uint32_t ValueSize = toInt32LE(it + 4);

                    if ((uint32_t) (ChunkTail - it) < 8 + ValueSize)
                        return false;

                    std::string ValueData;

                    ValueData.assign(it, it + 4);

                    for (size_t i = 0; i < _countof(RIFFToTagMap); ++i)
                    {
                        if (::memcmp(&it[0], RIFFToTagMap[i][0], 4) == 0)
                        {
                            ValueData = RIFFToTagMap[i][1];
                            break;
                        }
                    }

                    Temp.resize(ValueSize + 1);
                    std::copy(it + 8, it + 8 + (int) ValueSize, Temp.begin());
                    Temp[ValueSize] = '\0';

                    MetaData.AddItem(midi_metadata_item_t(0, ValueData.c_str(), (const char *) Temp.data()));

                    it += (int) (8 + ValueSize);

                    if ((ValueSize & 1) && (it < ChunkTail))
                        ++it;
                }

                FoundInfoChunk = true;
            }
            else
                return false; // Unknown LIST chunk.

            it = ChunkTail;

            if ((ChunkSize & 1) && (it < ChunkTail))
                ++it;
        }
        else
        // Is it a "RIFF" chunk? According to the standard this should not be possible it is how embedded SoundFonts are implemented... Sloppy programming...
        if (::memcmp(&it[0], "RIFF", 4) == 0)
        {
            auto ChunkTail = it + (int) (8 + ChunkSize);

            // Is it a "sfbk" chunk?
            if (::memcmp(&it[8], "sfbk", 4) == 0)
            {
                Temp.resize(8 + ChunkSize);
                std::copy(it, it + (int) (8 + ChunkSize), Temp.begin());

                container.SetSoundFontData(Temp);
            }

            it = ChunkTail;

            if ((ChunkSize & 1) && (it < ChunkTail))
                ++it;
        }
        else
        {
            it += (int) ChunkSize;

            if ((ChunkSize & 1) && (it != Tail))
                ++it;
        }
/*
        if (FoundDataChunk && FoundInfoChunk)
            break;
*/
    }

    container.SetExtraMetaData(MetaData);

    return true;
}
