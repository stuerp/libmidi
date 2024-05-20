
/** $VER: mididump.cpp (2024.05.15) P. Stuer **/

#include <CppCoreCheck/Warnings.h>

#pragma warning(disable: 4100 4625 4626 4710 4711 4738 5045 ALL_CPPCORECHECK_WARNINGS)

#include <SDKDDKVer.h>

#define NOMINMAX

#include <WinSock2.h>
#include <Windows.h>
#include <wincodec.h>

#include <stdio.h>

#include <iostream>
#include <iomanip>

#include <filesystem>

using namespace std;

#include <MIDIProcessor.h>
#include <Encoding.h>

#include "Tables.h"
#include "SysEx.h"

void ProcessContainer(midi_container_t & container, bool asStream);
void ProcessStream(const std::vector<midi_item_t> & stream, const sysex_table_t & sysExMap, bool skipNormalEvents = false);
void ProcessTracks(const midi_container_t & container);

std::vector<uint8_t> ReadFile(std::wstring & filePath);

/// <summary>
/// 
/// </summary>
int midmain(int argc, wchar_t * argv[])
{
    if (argc < 2)
        return -1;

    std::wstring FilePath = argv[1];

    try
    {
        std::vector<uint8_t> Data = ReadFile(FilePath);

        midi_container_t Container;

        midi_processor_t::Process(Data, FilePath.c_str(), Container);

        ProcessContainer(Container, false);
    }
    catch (std::exception & e)
    {
        ::printf("%s\n", e.what());
    }

    return 0;
}

/// <summary>
/// 
/// </summary>
void ProcessContainer(midi_container_t & Container, bool asStream)
{
    if (asStream)
    {
        const uint32_t SubsongCount = (uint32_t) Container.GetSubSongCount();
        const uint32_t SubsongIndex = 0;

        const uint32_t Format = Container.GetFormat();
        const uint32_t TrackCount = (Format == 2) ? 1 : Container.GetTrackCount();

        ::printf("MIDI Format %d, %d tracks, %d subsongs\n", Format, TrackCount, SubsongCount);

        for (uint32_t i = 0; i < TrackCount; ++i)
        {
            uint32_t ChannelCount = Container.GetChannelCount(SubsongIndex);
            uint32_t Duration = Container.GetDuration(SubsongIndex, false);
            uint32_t DurationInMS = Container.GetDuration(SubsongIndex, true);

            ::printf("Track %2d: %d channels, %8d ticks, %8.2fs\n", i, ChannelCount, Duration, (float) DurationInMS / 1000.0f);

            midi_metadata_table_t MetaData;

            Container.GetMetaData(SubsongIndex, MetaData);

            for (size_t j = 0; j < MetaData.GetCount(); ++j)
            {
                const midi_metadata_item_t & Item = MetaData[j];

                ::printf("- %8d %s: %s\n", Item.Timestamp, Item.Name.c_str(), Item.Value.c_str());
            }
        }

        uint32_t LoopBegin = Container.GetLoopBeginTimestamp(0);
        uint32_t LoopEnd =  Container.GetLoopEndTimestamp(0);

        ::printf("Loop Begin: %d ticks\n", LoopBegin);
        ::printf("Loop End  : %d ticks\n", LoopEnd);

        std::vector<midi_item_t> Stream;
        sysex_table_t SysExMap;

        Container.SerializeAsStream(0, Stream, SysExMap, LoopBegin, LoopEnd, 0);

        ProcessStream(Stream, SysExMap);
    }
    else
        ProcessTracks(Container);
}

/// <summary>
/// 
/// </summary>
std::vector<uint8_t> ReadFile(std::wstring & filePath)
{
    FILE * fp = nullptr;

    if ((::_wfopen_s(&fp, filePath.c_str(), L"rb") != 0) || (fp == nullptr))
    {
        char ErrorMessage[64];

        ::strerror_s(ErrorMessage, _countof(ErrorMessage), errno);

        throw std::runtime_error(std::format("Failed to open \"{}\" for reading: {} ({})", WideToUTF8(filePath).c_str(), ErrorMessage, errno));
    }

    ::fseek(fp, 0, SEEK_END);

    size_t FileSize = (size_t) ::ftell(fp);

    ::fseek(fp, 0, SEEK_SET);

    std::vector<uint8_t> Data(FileSize);

    ::fread(Data.data(), 1, FileSize, fp);

    ::fclose(fp);

    return Data;
}
