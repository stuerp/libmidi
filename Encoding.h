
/** $VER: Encoding.h (2024.05.13) P. Stuer **/

#pragma once

#include "framework.h"

std::string WideToUTF8(const wchar_t * wide);

inline std::string WideToUTF8(const std::wstring & wide)
{
    return WideToUTF8(wide.c_str());
}

std::wstring UTF8ToWide(const char * utf8);

inline std::wstring UTF8ToWide(const std::string & utf8)
{
    return UTF8ToWide(utf8.c_str());
}

std::string FormatText(const char * format, ...);
