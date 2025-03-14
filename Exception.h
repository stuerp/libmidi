
/** $VER: Exception.h (2024.08.20) P. Stuer **/

#pragma once

#include "framework.h"

std::string GetErrorMessage(DWORD errorCode, const std::string & errorMessage);

#pragma warning(disable: 4820) // 'x' bytes padding added after data member 'y'

class midi_exception : public std::runtime_error
{
public:
    midi_exception(const std::string & errorMessage) : std::runtime_error(errorMessage) { }
};

class Win32Exception : public std::runtime_error
{
public:
    Win32Exception(DWORD errorCode, const std::string & errorMessage) : std::runtime_error(GetErrorMessage(errorCode, errorMessage)), _ErrorCode(errorCode) { }

    DWORD GetErrorCode() const { return _ErrorCode; }

private:
    DWORD _ErrorCode;
};
