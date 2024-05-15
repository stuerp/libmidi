
/** $VER: rcpmain.cpp (2024.05.09) P. Stuer **/

#include <CppCoreCheck/Warnings.h>

#pragma warning(disable: 4625 4626 ALL_CPPCORECHECK_WARNINGS)

#define _CRT_SECURE_NO_WARNINGS

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

#include "RCP.h"

/// <summary>
/// 
/// </summary>
int rcpmain(int argc, char * argv[])
{
    int Result = 0;

    if (argc < 3)
    {
        printf("Usage: rcp2mid.exe [options] input.bin output.mid\n");
        printf("Input file formats:\n");
        printf("    RCP/R36/G36 Recomposer sequence file\n");
        printf("    CM6         Recomposer MT-32/CM-64 control file\n");
        printf("    GSD         Recomposer SC-55 control file\n");
        printf("Output file formats:\n");
        printf("    Sequence files are converted into MIDIs.\n");
        printf("    Control files can be converted to raw SYX or MIDI.\n");
        printf("        The file extension of the output file specifies the format.");
        printf("\n");
        printf("Options:\n");
        printf("    -Loops n    Loop each track at least n times. (default: 2)\n");
        printf("    -NoLpExt    No Loop Extension\n");
        printf("                Do not fill short tracks to the length of longer ones.\n");
        printf("    -WtLoop     Wolfteam Loop mode (loop from measure 2 on)\n");
        printf("    -KeepDummyCh convert data with MIDI channel set to -1\n");
        printf("                channel -1 is invalid, some RCPs use it for muting\n");

        return 0;
    }

    int argbase = 1;

    rcp_converter_t RCPConverter;

    rcp_converter_options_t & Options = RCPConverter._Options;

    while (argbase < argc && argv[argbase][0] == '-')
    {
        if (!::_stricmp(argv[argbase] + 1, "Loops"))
        {
            argbase++;

            if (argbase < argc)
            {
                Options._RCPLoopCount = (uint16_t) ::strtoul(argv[argbase], nullptr, 0);

                if (Options._RCPLoopCount == 0)
                    Options._RCPLoopCount = 2;
            }
        }
        else
        if (!::_stricmp(argv[argbase] + 1, "NoLpExt"))
            Options._NoLoopExtension = true;
        else
        if (!::_stricmp(argv[argbase] + 1, "WtLoop"))
            Options._WolfteamLoopMode = true;
        else
        if (!::_stricmp(argv[argbase] + 1, "KeepDummyCh"))
            Options._KeepDummyChannels = true;
        else
            break;
        argbase++;
    }

    if (argc < argbase + 2)
    {
        printf("Not enough arguments.\n");
        return 0;
    }

    time_t Time;
	
    ::time(&Time);

    char TimeText[32]; ::strftime(TimeText, _countof(TimeText), "%Y-%m-%d %H:%M:%S", ::localtime(&Time));

    const char * FilePath = argv[argbase];

    RCPConverter.SetFilePath(FilePath);

    ::printf("\n%s, \"%s\"\n", TimeText, FilePath);

    buffer_t SrcData;

    if (!SrcData.ReadFile(FilePath))
        throw std::runtime_error("Unable to read file.");

    try
    {
        buffer_t DstData;

        const char * FileExtension = ::GetFileExtension(argv[argbase + 1]);

        RCPConverter.Convert(SrcData, DstData, FileExtension);

        DstData.WriteFile(argv[argbase + 1]);

#ifdef _DEBUG
    {
        char RefFilePath[260];

        ::strcpy_s(RefFilePath, _countof(RefFilePath), FilePath);

        char * FileExtension = (char *) ::GetFileExtension(RefFilePath);

        if (FileExtension != nullptr)
        {
            ::strcpy(FileExtension, "mid");

            buffer_t RefFile;

            if (!RefFile.ReadFile(RefFilePath))
                throw std::runtime_error(std::format("Reference file not found: \"{}\".", RefFilePath));

            if (RefFile.Size != DstData.Size)
                throw std::runtime_error(std::format("Conversion error in \"{}\". File size mismatch.", FilePath));

            if (::memcmp(RefFile.Data, DstData.Data, RefFile.Size) != 0)
                throw std::runtime_error(std::format("Conversion error in \"{}\". File content mismatch.", FilePath));
        }
    }
#endif
    }
    catch (std::exception & e)
    {
        ::printf("Exception: %s\n", e.what());
    }

    ::putwchar('\n');

    return Result;
}
