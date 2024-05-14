
/** $VER: Encoding.cpp (2024.05.13) P. Stuer **/

#include "framework.h"

#include "Encoding.h"

std::string WideToUTF8(const wchar_t * wide)
{
    int Size = ::WideCharToMultiByte(CP_UTF8, 0, wide, -1, nullptr, 0, nullptr, nullptr);

    std::string UTF8;

    UTF8.resize((size_t) Size);

    ::WideCharToMultiByte(CP_UTF8, 0, wide, -1, UTF8.data(), (int) UTF8.size(), nullptr, nullptr);

    return UTF8;
}

std::wstring UTF8ToWide(const char * utf8)
{
    int Size = ::MultiByteToWideChar(CP_UTF8, 0, utf8, -1, nullptr, 0);

    std::wstring Wide;

    Wide.resize((size_t) Size);

    ::MultiByteToWideChar(CP_UTF8, 0, utf8, -1, Wide.data(), (int) Wide.size());

    return Wide;
}

std::string FormatText(const char * format, ...)
{
   va_list vl;

   va_start(vl, format);

    std::string Text;

    Text.resize(256);

    ::vsprintf_s(Text.data(), Text.size(), format, vl);

    va_end(vl);

    return Text;
}
