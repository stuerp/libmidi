
/** $VER: main.cpp (2025.07.26) P. Stuer **/

#include <CppCoreCheck/Warnings.h>

#pragma warning(disable: 4625 4626 4820 ALL_CPPCORECHECK_WARNINGS)

#include <SDKDDKVer.h>

#define NOMINMAX

#include <WinSock2.h>
#include <windows.h>

#include <pathcch.h>
#include <shlwapi.h>

#pragma comment(lib, "pathcch")
#pragma comment(lib, "shlwapi")

#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

#include <Encoding.h>

#include <map>
#include <filesystem>

namespace fs = std::filesystem;

void ExamineFile(const fs::path & filePath, const std::map<std::string, std::string> & args);

static void ProcessDirectory(const fs::path & directoryPath);
static void ProcessFile(const fs::path & filePath);

const std::vector<fs::path> Filters = { ".mid", /*".g36",*/ ".rmi", /*".mxmf", ".xmf",*/ ".mmf", ".tst" };

std::map<std::string, std::string> Arguments;

int main(int argc, const char ** argv)
{
    ::printf("\xEF\xBB\xBF"); // UTF-8 BOM

    if (argc < 2)
    {
        ::printf("Insufficient arguments.\n");

        return -1;
    }

    for (int i = 1; i < argc; ++i)
    {
        if (argv[i][0] == '-')
        {
            if (::_stricmp(argv[i], "-stream") == 0)
                Arguments["AsStream"] = "";
        }

        Arguments["midifile"] = argv[i];
    }

    if (!::fs::exists(Arguments["midifile"]))
    {
        ::printf("Failed to access \"%s\": path does not exist.\n", Arguments["midifile"].c_str());

        return -1;
    }

    fs::path Path = std::filesystem::canonical(Arguments["midifile"]);

    if (fs::is_directory(Path))
        ProcessDirectory(Path);
    else
        ProcessFile(Path);

    return 0;
}

/// <summary>
/// Returns true if the string matches one of the list.
/// </summary>
static bool IsOneOf(const fs::path & item, const std::vector<fs::path> & list) noexcept
{
    for (const auto & Item : list)
    {
        if (::_stricmp(item.string().c_str(), Item.string().c_str()) == 0)
            return true;
    }

    return false;
}

static void ProcessDirectory(const fs::path & directoryPath)
{
    ::printf("\"%s\"\n", directoryPath.string().c_str());

    for (const auto & Entry : fs::directory_iterator(directoryPath))
    {
        if (Entry.is_directory())
        {
            ProcessDirectory(Entry.path());
        }
        else
        if (IsOneOf(Entry.path().extension(), Filters))
        {
            ProcessFile(Entry.path());
        }
    }
}

static void ProcessFile(const fs::path & filePath)
{
    FILE * fp = nullptr;

    fs::path StdOut = filePath;

    if ((::freopen_s(&fp, StdOut.replace_extension(".log").string().c_str(), "w", stdout) != 0) || (fp == nullptr))
        return;

    ::printf("\xEF\xBB\xBF"); // UTF-8 BOM

    auto FileSize = fs::file_size(filePath);

    ::printf("\n\"%s\", %" PRIu64 " bytes\n", filePath.string().c_str(), (uint64_t) FileSize);

    ExamineFile(filePath, Arguments);

    ::fclose(fp);
}
