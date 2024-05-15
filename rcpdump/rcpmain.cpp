
/** $VER: rcpmain.cpp (2024.05.12) P. Stuer **/

#include <CppCoreCheck/Warnings.h>

#pragma warning(disable: 4625 4626 ALL_CPPCORECHECK_WARNINGS)

#include <SDKDDKVer.h>

#define NOMINMAX

#include <WinSock2.h>
#include <windows.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <string.h>
#include <ctype.h>

#include <stdexcept>
#include <algorithm>
#include <vector>
#include <cassert>
#include <format>

#include <MIDIProcessor.h>
#include <Recomposer\RCP.h>
#include <Encoding.h>

/// <summary>
/// 
/// </summary>
int rcpmain(int argc, wchar_t * argv[])
{
    int Result = 0;

    if (argc < 3)
    {
        ::printf("Usage: rcp2mid.exe [options] input.bin output.mid\n");
        ::printf("Input file formats:\n");
        ::printf("    RCP/R36/G36 Recomposer sequence file\n");
        ::printf("    CM6         Recomposer MT-32/CM-64 control file\n");
        ::printf("    GSD         Recomposer SC-55 control file\n");
        ::printf("Output file formats:\n");
        ::printf("    Sequence files are converted into MIDIs.\n");
        ::printf("    Control files can be converted to raw SYX or MIDI.\n");
        ::printf("        The file extension of the output file specifies the format.");
        ::printf("\n");
        ::printf("Options:\n");
        ::printf("    -Loops n            Loop each track at least n times. (default: 2)\n");
        ::printf("    -NoLoopExtension    No Loop Extension. Do not fill short tracks to the length of longer ones.\n");
        ::printf("    -WolfteamLoop       Wolfteam Loop mode (loop from measure 2 on)\n");
        ::printf("    -KeepMutedChannels  Convert data with MIDI channel set to -1. Some RCPs use it for muting.\n");

        return 0;
    }

    rcp_converter_t RCPConverter;

    rcp_converter_options_t & Options = RCPConverter._Options;

    int argbase = 1;

    {
        while (argbase < argc && argv[argbase][0] == '-')
        {
            if (!::_wcsicmp(argv[argbase] + 1, L"Loops"))
            {
                argbase++;

                if (argbase < argc)
                {
                    Options._RCPLoopCount = (uint16_t) ::wcstoul(argv[argbase], nullptr, 0);

                    if (Options._RCPLoopCount == 0)
                        Options._RCPLoopCount = 2;
                }
            }
            else
            if (!::_wcsicmp(argv[argbase] + 1, L"NoLoopExtension"))
                Options._NoLoopExtension = true;
            else
            if (!::_wcsicmp(argv[argbase] + 1, L"WolfteamLoop"))
                Options._WolfteamLoopMode = true;
            else
            if (!::_wcsicmp(argv[argbase] + 1, L"KeepMutedChannels"))
                Options._KeepDummyChannels = true;
            else
                break;

            argbase++;
        }

        if (argc < argbase + 1)
        {
            ::printf("Not enough arguments.\n");
            return 0;
        }
    }

    time_t Time;
	
    ::time(&Time);

    tm TM;

    ::localtime_s(&TM, &Time);

    char TimeText[32]; ::strftime(TimeText, _countof(TimeText), "%Y-%m-%d %H:%M:%S", &TM);

    const wchar_t * FilePath = argv[argbase];

    RCPConverter.SetFilePath(FilePath);

    ::printf("\n%s, \"%S\"\n", TimeText, FilePath);

    buffer_t SrcData;

    try
    {
        SrcData.ReadFile(FilePath);

        buffer_t DstData;

        const wchar_t * FileExtension = GetFileExtension(argv[argbase + 1]);

        RCPConverter.Convert(SrcData, DstData, FileExtension);

        if (argc < argbase + 2)
        {
            DstData.WriteFile(argv[argbase + 1]);

            #ifdef _RCP_VERBOSE
            {
                ::printf("Src: %d bytes, Dst: %d bytes\n", SrcData.Size, DstData.Size);

                wchar_t RefFilePath[260];

                ::wcscpy_s(RefFilePath, _countof(RefFilePath), FilePath);

                wchar_t * FileExtension = (wchar_t *) GetFileExtension(RefFilePath);

                if (FileExtension != nullptr)
                {
                    ::wcscpy_s(FileExtension, _countof(RefFilePath) - 3, L"mid");

                    buffer_t RefFile;

                    try
                    {
                        RefFile.ReadFile(RefFilePath);
                    }
                    catch (std::runtime_error & e)
                    {
                        std::string Text(std::format("Reference file \"{}\" not found: ", WideToUTF8(RefFilePath).c_str()));

                        throw std::runtime_error(Text + e.what());
                    }
    
                    if (RefFile.Size != DstData.Size)
                        throw std::runtime_error(std::format("Conversion error in \"{}\". File size mismatch.", WideToUTF8(FilePath).c_str()));

                    if (::memcmp(RefFile.Data, DstData.Data, RefFile.Size) != 0)
                        throw std::runtime_error(std::format("Conversion error in \"{}\". File content mismatch.", WideToUTF8(FilePath).c_str()));
                }

                MIDIContainer Container;

                std::vector<uint8_t> Data;

                Data.insert(Data.end(), DstData.Data, DstData.Data + DstData.Size);

                if (!MIDIProcessor::Process(Data, FilePath, Container))
                    throw std::runtime_error(std::format("MIDIProcesser failed: \"{}\".", WideToUTF8(FilePath).c_str()));
            }
            #endif
        }
    }
    catch (std::exception & e)
    {
        ::printf("Exception: %s\n", e.what());
    }

    ::putwchar('\n');

    return Result;
}
