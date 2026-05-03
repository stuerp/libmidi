
/** $VER: MIDIProcessorMMD.cpp (2026.05.03) P. Stuer **/

#include "pch.h"

#include "MIDIProcessor.h"

#include <MMD.h>

namespace midi
{

/// <summary>
/// Returns true if data points to an MMD sequence.
/// </summary>
bool processor_t::IsMMD(std::vector<uint8_t> const & data, const std::wstring & fileExtension) noexcept
{
    if (fileExtension.empty())
        return false;

    if (::_wcsicmp(fileExtension.c_str(), L"mmd"))
        return false;

    const size_t TrackCount = 18;
    const size_t HeaderSize = 2 + (TrackCount * 4);

    if (data.size() < HeaderSize)
        return false;

    // Verify the track offsets.
    const uint8_t * Data = data.data() + 2;

    for (size_t i = 0; i < TrackCount; ++i)
    {
        uint16_t TrackOffset = (uint16_t) ((Data[0x01] << 8) | (Data[0x00] << 0));

        if (TrackOffset >= data.size())
            return false;

        Data += 4;
    }

    return true;
}

/// <summary>
/// Processes the MMD data.
/// </summary>
bool processor_t::ProcessMMD(std::vector<uint8_t> const & data, const std::wstring & filePath, container_t & container)
{
    std::vector<uint8_t> Data;

    if (mmd::Convert(data.data(), (uint32_t) data.size(), Data) != 0)
        return false;

    container.FileFormat = FileFormat::MMD;

    return processor_t::Process(Data, filePath.c_str(), container);
}

}
