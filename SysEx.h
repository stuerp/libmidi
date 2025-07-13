
/** $VER: SysEx.h (2025.06.21) **/

#pragma once

#include "pch.h"

#include "MIDI.h"

namespace midi
{

class sysex_t
{
public:
    static bool IsSystemOn(const uint8_t * data) noexcept
    {
        return IsEqual(data, GM1SystemOn) || IsEqual(data, GM2SystemOn) || IsEqual(data, GSReset) || IsEqual(data, XGSystemOn);
    }

    static bool IsEqual(const uint8_t * a, const uint8_t * b) noexcept
    {
        while ((*a != StatusCodes::SysExEnd) && (*b != StatusCodes::SysExEnd) && (*a == *b))
        {
            a++;
            b++;
        }

        return (*a == *b);
    }

    static bool IsGMSystemOn(const uint8_t * data, size_t size) noexcept
    {
        return (size == _countof(GM1SystemOn) && ::memcmp(data, GM1SystemOn, _countof(GM1SystemOn)) == 0);
    }

    static bool IsGM2SystemOn(const uint8_t * data, size_t size) noexcept
    {
        return (size == _countof(GM2SystemOn) && ::memcmp(data, GM2SystemOn, _countof(GM2SystemOn)) == 0);
    }

    static bool IsGSSystemOn(const uint8_t * data, size_t size) noexcept
    {
        if (size != _countof(GSReset))
            return false;

        if (::memcmp(data, GSReset, 5) != 0)
            return false;

        if (::memcmp(data + 7, GSReset + 7, 2) != 0)
            return false;

        if (((data[5] + data[6] + 1) & 127) != data[9])
            return false;

        if (data[10] != GSReset[10])
            return false;

        return true;
    }

    static bool IsXGSystemOn(const uint8_t * data, size_t size) noexcept
    {
        return (size == _countof(XGSystemOn) && ::memcmp(data, XGSystemOn, _countof(XGSystemOn)) == 0);
    }

    static void SetRolandCheckSum(uint8_t * data, size_t size) noexcept
    {
        uint8_t Checksum = 0;
        size_t i;

        for (i = 5; (i + 1 < size) && (data[i + 1] != StatusCodes::SysExEnd); ++i)
            Checksum += data[i];

        data[i] = (uint8_t) ((128 - Checksum) & 127);
    }

    static const uint8_t GM1SystemOn[6];
    static const uint8_t GM1SystemOff[6];

    static const uint8_t GM2SystemOn[6];

    static const uint8_t D50Reset[10];
    static const uint8_t MT32Reset[10];

    static const uint8_t GSReset[11];
    static const uint8_t GSToneMapNumber[11];

    static const uint8_t XGSystemOn[9];
    static const uint8_t XGReset[9];
};

}
