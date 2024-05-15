
/** $VER: Support.cpp (2024.05.10) P. Stuer **/

#include "Support.h"

/// <summary>
/// 
/// </summary>
uint32_t MulDivCeil(uint32_t val, uint32_t mul, uint32_t div)
{
    return (uint32_t)(((uint64_t) val * mul + div - 1) / div);
}

/// <summary>
/// 
/// </summary>
uint32_t MulDivRound(uint32_t val, uint32_t mul, uint32_t div)
{
    return (uint32_t)(((uint64_t) val * mul + div / 2) / div);
}

/// <summary>
/// 
/// </summary>
uint16_t ReadLE16(const uint8_t * data)
{
    return (data[1] << 8) | (data[0] << 0);
}

/// <summary>
/// 
/// </summary>
uint32_t ReadLE32(const uint8_t * data)
{
    return	(data[3] << 24) | (data[2] << 16) | (data[1] << 8) | (data[0] << 0);
}

/// <summary>
/// 
/// </summary>
uint16_t GetTrimmedLength(const char * data, uint16_t size, char trimChar, bool leaveLast)
{
    uint16_t i;

    for (i = size; i > 0; --i)
    {
        if (data[i - 1] != trimChar)
            break;
    }

    if (leaveLast && (i < size))
        i++;

    return i;
}

/// <summary>
/// Converts RCP BPM to MIDI ticks.
/// </summary>
uint32_t BPM2Ticks(uint16_t bpm, uint8_t scale)
{
//  formula: (60 000 000.0 / bpm) * (scale / 64.0)
    const uint32_t div = bpm * scale;

    return 60000000U * 64U / div;
}

/// <summary>
/// Gets the address of the file name in the specified file path.
/// </summary>
const char * GetFileName(const char * filePath)
{
    const char * Sep1 = ::strrchr(filePath, '/');
    const char * Sep2 = ::strrchr(filePath, '\\');

    if (Sep1 == nullptr)
        Sep1 = Sep2;
    else
    if ((Sep2 != nullptr) && (Sep2 > Sep1))
        Sep1 = Sep2;

    return (Sep1 != nullptr) ? (Sep1 + 1) : filePath;
}

/// <summary>
/// Gets the address of the file extension in the specified file name.
/// </summary>
const char * GetFileExtension(const char * fileName)
{
    const char * Dot = ::strrchr(::GetFileName(fileName), '.');

    return (Dot != nullptr) ? (Dot + 1) : "";
}
