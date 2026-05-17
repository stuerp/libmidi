
/** $VER: MIDIProcessorMIDI2.cpp (2026.05.17) MIDI 2.0 Format (https://midi.org/specs) **/

#include "pch.h"

#include "MIDIProcessor.h"
#include "MIDIContainer.h"

#include "MIDI2.h"

using namespace midi2;

namespace midi
{

bool processor_t::IsMIDI2(std::vector<uint8_t> const & data) noexcept
{
    if ((data.size() < 8) || ((data.size() % 8) != 0))
        return false;

    if (::memcmp(data.data(), "SMF2CLIP", 8) != 0)
        return false;

    return true;
}

bool processor_t::ProcessMIDI2(std::vector<uint8_t> const & data, container_t & container)
{
    auto Packets = ump_t::FromBytes(data.data() + 8, data.size() - 8);

    return false;
}

}
