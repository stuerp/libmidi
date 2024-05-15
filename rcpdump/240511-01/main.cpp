
/** $VER: main.cpp (2024.05.10) P. Stuer **/

#include <CppCoreCheck/Warnings.h>

#pragma warning(disable: 4625 4626 ALL_CPPCORECHECK_WARNINGS)

#include <SDKDDKVer.h>
#include <windows.h>

#include <pathcch.h>
#include <shlwapi.h>

#pragma comment(lib, "pathcch")
#pragma comment(lib, "shlwapi")

#include <stdio.h>
#include <stdint.h>

static void ProcessDirectory(const WCHAR * directoryPath, const WCHAR * searchPattern);
static void ProcessFile(const WCHAR * filePath, uint64_t fileSize);

const WCHAR * Argument = LR"(f:\Tst\rcp)";
const WCHAR * Filters[] = { L".rcp", L".r36", L".g18", L".g36", L".cm6", L".gsd" };

int rcpmain(int argc, char * argv[]);

int main()
{
    if (!::PathFileExistsW(Argument))
    {
        ::wprintf(L"Failed to access \"%s\": path does not exist.\n", Argument);
        return -1;
    }

    WCHAR DirectoryPath[MAX_PATH];

    if (::GetFullPathNameW(Argument, _countof(DirectoryPath), DirectoryPath, nullptr) == 0)
    {
        ::wprintf(L"Failed to expand \"%s\": Error %u.\n", Argument, ::GetLastError());
        return -1;
    }

    if (!::PathIsDirectoryW(Argument))
    {
        ::PathCchRemoveFileSpec(DirectoryPath, _countof(DirectoryPath));

        ProcessDirectory(DirectoryPath, ::PathFindFileNameW(Argument));
    }
    else
        ProcessDirectory(DirectoryPath, L"*.*");

    return 0;
}

static void ProcessDirectory(const WCHAR * directoryPath, const WCHAR * searchPattern)
{
    ::wprintf(L"\"%s\"\n", directoryPath);

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

static void ProcessFile(const WCHAR * srcFilePathW, uint64_t fileSize)
{
    const WCHAR * FileExtension;

    HRESULT hr = ::PathCchFindExtension(srcFilePathW, ::wcslen(srcFilePathW) + 1, &FileExtension);

    if (SUCCEEDED(hr))
    {
        for (const auto & Filter : Filters)
        {
            if (::wcscmp(FileExtension, Filter) == 0)
            {
                char DstFilePathA[MAX_PATH];

                {
                    WCHAR DstFilePathW[MAX_PATH];

                    ::wcscpy_s(DstFilePathW, _countof(DstFilePathW), srcFilePathW);

                    ::PathCchRenameExtension(DstFilePathW, _countof(DstFilePathW), L"mid");

                    const WCHAR * DstFileNameW = ::PathFindFileNameW(DstFilePathW);

                    int Length = ::WideCharToMultiByte(CP_ACP, 0, DstFileNameW, (int) ::wcslen(DstFileNameW), DstFilePathA, _countof(DstFilePathA), nullptr, nullptr);

                    DstFilePathA[Length] = '\0';
                }

                char SrcFilePathA[MAX_PATH];

                {
                    int Length = ::WideCharToMultiByte(CP_ACP, 0, srcFilePathW, (int) ::wcslen(srcFilePathW), SrcFilePathA, _countof(SrcFilePathA), nullptr, nullptr);

                    SrcFilePathA[Length] = '\0';
                }

                {
                    char * argv[] = { (char *) "rcp2mid.exe", SrcFilePathA, (char *) DstFilePathA /*"DEADBEEF.mid"*/ };

                    rcpmain(_countof(argv), argv);
                }

                break;
            }
        }
    }
}
