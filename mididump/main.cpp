
/** $VER: main.cpp (2025.03.19) P. Stuer **/

#include <CppCoreCheck/Warnings.h>

#pragma warning(disable: 4625 4626 ALL_CPPCORECHECK_WARNINGS)

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

static void ProcessDirectory(const WCHAR * directoryPath, const WCHAR * searchPattern);
static void ProcessFile(const WCHAR * filePath, uint64_t fileSize);

int midmain(int argc, wchar_t * argv[]);

const WCHAR * Filters[] = { L".mid", /*L".g36",*/ /*L".rmi", L".mxmf", L".xmf",*/ L".mmf", L".tst" };

int wmain(int argc, const wchar_t ** argv)
{
    ::printf("\xEF\xBB\xBF"); // UTF-8 BOM

    if (argc < 2)
    {
        ::printf("Insufficient arguments.\n");
        return -1;
    }

    if (!::PathFileExistsW(argv[1]))
    {
        ::printf("Failed to access \"%s\": path does not exist.\n", ::WideToUTF8(argv[1]).c_str());
        return -1;
    }

    WCHAR DirectoryPath[MAX_PATH];

    if (::GetFullPathNameW(argv[1], _countof(DirectoryPath), DirectoryPath, nullptr) == 0)
    {
        ::printf("Failed to expand \"%s\": Error %u.\n", ::WideToUTF8(argv[1]).c_str(), (uint32_t) ::GetLastError());
        return -1;
    }

    if (!::PathIsDirectoryW(argv[1]))
    {
        ::PathCchRemoveFileSpec(DirectoryPath, _countof(DirectoryPath));

        ProcessDirectory(DirectoryPath, ::PathFindFileNameW(argv[1]));
    }
    else
        ProcessDirectory(DirectoryPath, L"*.*");

    return 0;
}

static void ProcessDirectory(const WCHAR * directoryPath, const WCHAR * searchPattern)
{
    ::printf("\"%s\"\n", ::WideToUTF8(directoryPath).c_str());

    WCHAR PathName[MAX_PATH];

    if (!SUCCEEDED(::PathCchCombineEx(PathName, _countof(PathName), directoryPath, searchPattern, PATHCCH_ALLOW_LONG_PATHS)))
        return;

    WIN32_FIND_DATA fd = {};

    HANDLE hFind = ::FindFirstFileW(PathName, &fd);

    if (hFind == INVALID_HANDLE_VALUE)
        return;

    BOOL Success = TRUE;

    if (::wcscmp(fd.cFileName, L".") == 0)
    {
        Success = ::FindNextFileW(hFind, &fd);

        if (Success && ::wcscmp(fd.cFileName, L"..") == 0)
            Success = ::FindNextFileW(hFind, &fd);
    }

    while (Success)
    {
        if (SUCCEEDED(::PathCchCombineEx(PathName, _countof(PathName), directoryPath, fd.cFileName, PATHCCH_ALLOW_LONG_PATHS)))
        {
            if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                ProcessDirectory(PathName, searchPattern);
            else
            {
                uint64_t FileSize = (((uint64_t) fd.nFileSizeHigh) << 32) + fd.nFileSizeLow;

                ProcessFile(PathName, FileSize);
            }
        }

        Success = ::FindNextFileW(hFind, &fd);
    }

    ::FindClose(hFind);
}

static void ProcessFile(const WCHAR * filePath, uint64_t fileSize)
{
    ::printf("\n\"%s\", %" PRIu64 " bytes\n", ::WideToUTF8(filePath).c_str(), fileSize);

    const WCHAR * FileExtension;

    HRESULT hr = ::PathCchFindExtension(filePath, ::wcslen(filePath) + 1, &FileExtension);

    if (SUCCEEDED(hr))
    {
        for (const auto & Filter : Filters)
        {
            if (::_wcsicmp(FileExtension, Filter) == 0)
            {
                {
                    const wchar_t * argv[] = { L"mididump.exe", filePath };

                    midmain(_countof(argv), (wchar_t **) argv);
                }

                break;
            }
        }
    }
}
