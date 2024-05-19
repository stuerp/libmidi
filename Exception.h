
/** $VER: Exception.h (2024.05.16) P. Stuer **/

#pragma once

#include "framework.h"

std::string GetErrorMessage(DWORD errorCode, const std::string & errorMessage);

#pragma warning(disable: 4820) // 'x' bytes padding added after data member 'y'

class MIDIException : public std::runtime_error
{
public:
    MIDIException(const std::string & errorMessage) : std::runtime_error(errorMessage) { }
};

class Win32Exception : public std::runtime_error
{
public:
    Win32Exception(DWORD errorCode, const std::string & errorMessage) : std::runtime_error(GetErrorMessage(errorCode, errorMessage)), _ErrorCode(errorCode) { }

    DWORD GetErrorCode() const { return _ErrorCode; }

private:
    DWORD _ErrorCode;
};
