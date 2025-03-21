
/** $VER: MIDIProcessorRCP.cpp (2025.03.19) P. Stuer - Based on Valley Bell's rpc2mid (https://github.com/ValleyBell/MidiConverters). **/

#include "pch.h"

#include "MIDIProcessor.h"

#include <RCP.h>

#include <filesystem>

namespace midi
{

/// <summary>
/// Returns true if data points to an RCP sequence.
/// </summary>
bool processor_t::IsRCP(std::vector<uint8_t> const & data, const wchar_t * fileExtension) noexcept
{
    if (fileExtension == nullptr)
        return false;

    if (data.size() < 28)
        return false;

    if (::strncmp((const char *) data.data(), "RCM-PC98V2.0(C)COME ON MUSIC", 28) == 0)
    {
        if (::_wcsicmp(fileExtension, L"rcp") == 0)
            return true;

        if (::_wcsicmp(fileExtension, L"r36") == 0)
            return true;

        return false;
    }

    if (data.size() < 31)
        return false;

    if (::strncmp((const char *) data.data(), "COME ON MUSIC RECOMPOSER RCP3.0", 31) == 0)
    {
        if (::_wcsicmp(fileExtension, L"g18") == 0)
            return true;

        if (::_wcsicmp(fileExtension, L"g36") == 0)
            return true;

        return false;
    }

    return false;
}

/// <summary>
/// Processes the sequence data.
/// </summary>
bool processor_t::ProcessRCP(std::vector<uint8_t> const & data, const wchar_t * filePath, container_t & container)
{
    rcp_converter_t RCPConverter;

    RCPConverter.SetFilePath(filePath);

    rcp_converter_options_t & Options = RCPConverter._Options;

    Options._RCPLoopCount       = _Options._LoopExpansion;

    Options._WriteBarMarkers    = _Options._WriteBarMarkers;
    Options._WriteSysExNames    = _Options._WriteSysExNames;
    Options._ExtendLoops        = _Options._ExtendLoops;
    Options._WolfteamLoopMode   = _Options._WolfteamLoopMode;
    Options._KeepDummyChannels  = _Options._KeepDummyChannels;
    Options._IncludeControlData = _Options._IncludeControlData;

    buffer_t SrcData;

    SrcData.Copy(data.data(), data.size());

    buffer_t DstData;

    RCPConverter.Convert(SrcData, DstData);

    std::vector<uint8_t> Data;

    Data.insert(Data.end(), DstData.Data, DstData.Data + DstData.Size);

    processor_t::Process(Data, filePath, container);

    return true;
}

}