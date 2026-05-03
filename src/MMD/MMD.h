
/** $VER: MMD.h (2026.05.03) P. Stuer - Based on Valley Bell's mmd2mid (https://github.com/ValleyBell/MidiConverters). **/

#pragma once

#include <cstdint>

namespace mmd
{

uint8_t Convert(const uint8_t * srcData, uint32_t srcSize, std::vector<uint8_t> & dstData) noexcept;

}
