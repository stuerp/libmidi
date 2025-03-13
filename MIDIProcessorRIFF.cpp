
/** $VER: MIDIProcessorRIFF.cpp (2025.03.13) **/

#include "framework.h"

#include "MIDIProcessor.h"
#include "Encoding.h"

#include <map>

static const std::map<const std::string, const std::string> RIFFToTagMap =
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

static inline uint32_t toInt32LE(const uint8_t * data)
{
    return static_cast<uint32_t>(data[0]) | static_cast<uint32_t>(data[1] << 8) | static_cast<uint32_t>(data[2] << 16) | static_cast<uint32_t>(data[3] << 24);
}

static inline uint32_t toInt32LE(std::vector<uint8_t>::const_iterator data)
{
    return static_cast<uint32_t>(data[0]) | static_cast<uint32_t>(data[1] << 8) | static_cast<uint32_t>(data[2] << 16) | static_cast<uint32_t>(data[3] << 24);
}

static bool GetCodePage(std::vector<uint8_t>::const_iterator it, std::vector<uint8_t>::const_iterator chunkTail, uint32_t & codePage) noexcept;

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

bool midi_processor_t::ProcessRIFF(std::vector<uint8_t> const & data, midi_container_t & container)
{
    bool FoundDataChunk = false;
    bool FoundINFOChunk = false;

    midi_metadata_table_t MetaData;
    std::vector<uint8_t> Temp;

    uint32_t Size = (uint32_t) (data[4] | (data[5] << 8) | (data[6] << 16) | (data[7] << 24));

    const auto Tail = data.begin() + (ptrdiff_t) (8 + Size);

    auto it = data.begin() + 12;

    while (it < Tail)
    {
        if (Tail - it < 8)
            return false;

        uint32_t ChunkSize = toInt32LE(it + 4);

        if ((uint32_t) (Tail - it) < ChunkSize)
            return false;

        std::string ChunkId;

        ChunkId.assign(it, it + 4);

        // Is it a "data" chunk?
        if (ChunkId == "data")
        {
            if (FoundDataChunk)
                return false; // throw exception_io_data( "Multiple RIFF data chunks found" );

            std::vector<uint8_t> Data;

            Data.assign(it + 8, it + (ptrdiff_t) (8 + ChunkSize));

            if (!ProcessSMF(Data, container))
                return false;

            FoundDataChunk = true;

            it += (ptrdiff_t) (8 + ChunkSize);

            if ((ChunkSize & 1) && (it < Tail))
                ++it;
        }
        else
        // Is it a "DISP" chunk?
        if (ChunkId == "DISP")
        {
            uint32_t type = toInt32LE(it + 8);

            if (type == CF_TEXT)
            {
                std::string DisplayName;

                DisplayName.assign(it + 12, it + (ptrdiff_t) (8 + ChunkSize));

                MetaData.AddItem(midi_metadata_item_t(0, "display_name", DisplayName.c_str()));
            }

            it += (ptrdiff_t) (8 + ChunkSize);

            if ((ChunkSize & 1) && (it < Tail))
                ++it;
        }
        else
        // Is it a "LIST" chunk?
        if (ChunkId == "LIST")
        {
            auto ChunkTail = it + (ptrdiff_t) (8 + ChunkSize);

            // Is it a "INFO" chunk?
            if (::memcmp(&it[8], "INFO", 4) == 0)
            {
                if (FoundINFOChunk)
                    return false; // throw exception_io_data( "Multiple RIFF LIST INFO chunks found" );

                if (ChunkTail - it < 12)
                    return false;

                it += 12;

                uint32_t CodePage = ~0u;

                bool FoundIALBChunk = false;

                std::string ProductName;

                GetCodePage(it, ChunkTail, CodePage);

                while (it != ChunkTail)
                {
                    if (ChunkTail - it < 4)
                        return false;

                    uint32_t ValueSize = toInt32LE(it + 4);

                    if ((uint32_t) (ChunkTail - it) < 8 + ValueSize)
                        return false;

                    ChunkId.assign(it, it + 4);

                    if (ChunkId == "IENC")
                    {
                        // Skip
                    }
                    else
                    if (ChunkId == "IPIC")
                    {
                        Temp.resize(ValueSize);

                        std::copy(it + 8, it + (ptrdiff_t) (8 + ValueSize), Temp.begin());

                        container.SetArtwork(Temp);
                    }
                    else
                    if ((ChunkId == "DBNK") && (ValueSize == 2))
                    {
                        const uint8_t * Data = &it[8];

                        container.SetBankOffset((Data[1] << 8) | Data[0]);
                    }
                    else
                    {
                        if (ChunkId == "IALB")
                            FoundIALBChunk = true;

                        std::string Text;

                        if (CodePage != ~0u)
                            Text = CodePageToUTF8(CodePage, (const char *) &it[8], ValueSize);
                        else
                            Text.assign(it + 8, it + (ptrdiff_t) (8 + ValueSize));

                        if (ChunkId == "IPRD")
                            ProductName = Text;

                        {
                            const auto it2 = RIFFToTagMap.find(ChunkId);

                            if (it2 != RIFFToTagMap.end())
                                ChunkId = it2->second;
                        }

                        MetaData.AddItem(midi_metadata_item_t(0, ChunkId.c_str(), Text.c_str()));
                    }

                    it += (ptrdiff_t) (8 + ValueSize);

                    if ((ValueSize & 1) && (it < ChunkTail))
                        ++it;
                }

                // Use the product name also as album name if no IALB chunk was found in the INFO list.
                if (!FoundIALBChunk && !ProductName.empty())
                    MetaData.AddItem(midi_metadata_item_t(0, "album", ProductName.c_str()));

                FoundINFOChunk = true;
            }
            else
                return false; // Unknown LIST chunk.

            it = ChunkTail;

            if ((ChunkSize & 1) && (it < ChunkTail))
                ++it;
        }
        else
        // Is it a "RIFF" chunk? According to the standard this should not be possible but it is how embedded SoundFonts are implemented. Sloppy design...
        if (ChunkId == "RIFF")
        {
            const auto ChunkTail = it + (ptrdiff_t) (8 + ChunkSize);

            // Is it a "sfbk" chunk?
            if ((::memcmp(&it[8], "sfbk", 4) == 0) || (::memcmp(&it[8], "DLS ", 4) == 0))
            {
                Temp.resize(8 + ChunkSize);
                std::copy(it, ChunkTail, Temp.begin());

                container.SetSoundFontData(Temp);
            }

            it = ChunkTail;

            if ((ChunkSize & 1) && (it < ChunkTail))
                ++it;
        }
        else
        {
            it += (ptrdiff_t) ChunkSize;

            if ((ChunkSize & 1) && (it != Tail))
                ++it;
        }
    }

    container.SetExtraMetaData(MetaData);

    return true;
}

/// <summary>
/// Gets the code page from the IENC chunk, if present.
/// </summary>
bool GetCodePage(std::vector<uint8_t>::const_iterator it, std::vector<uint8_t>::const_iterator chunkTail, uint32_t & codePage) noexcept
{
    while (it != chunkTail)
    {
        if (chunkTail - it < 4)
            return false;

        uint32_t ValueSize = toInt32LE(it + 4);

        if ((chunkTail - it) < (ptrdiff_t) (8 + ValueSize))
            return false;

        std::string ValueData;

        ValueData.assign(it, it + 4);

        if (ValueData == "IENC")
        {
            std::string Encoding;

            Encoding.resize(ValueSize);
            std::copy(it + 8, it + (ptrdiff_t) (8 + ValueSize), Encoding.begin());

            GetCodePageFromEncoding(Encoding, codePage);

            return true;
        }

        it += (ptrdiff_t) (8 + ValueSize);

        if ((ValueSize & 1) && (it < chunkTail))
            ++it;
    }

    return false;
}
