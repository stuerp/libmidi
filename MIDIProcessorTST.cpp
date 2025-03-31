
/** $VER: MIDIProcessorTST.cpp (2025.03.31) Test File **/

#include "pch.h"

#include "MIDIProcessor.h"
#include "Encoding.h"

namespace midi
{

/// <summary>
/// Returns true if the byte vector contains TST data.
/// </summary>
bool processor_t::IsTST(std::vector<uint8_t> const & data, const std::wstring & fileExtension) noexcept
{
    if (::_wcsicmp(fileExtension.c_str(), L"tst"))
        return false;

    return true;
};

/// <summary>
/// Processes a byte vector with TST data.
/// </summary>
bool processor_t::ProcessTST(std::vector<uint8_t> const & data, container_t & container)
{
    container.Initialize(1u, 500);

    track_t Track;

    uint8_t Data[2] = { 60, 0x7Fu };

    Track.AddEvent(event_t(      0, event_t::NoteOn, (uint32_t) 0, Data, 2));

    Data[0] = 62;

    Track.AddEvent(event_t(   1000, event_t::NoteOn, (uint32_t) 0, Data, 2));

    Data[0] = 64;

    Track.AddEvent(event_t(   2000, event_t::NoteOn, (uint32_t) 0, Data, 2));

    Data[0] = StatusCodes::MetaData;
    Data[1] = MetaDataTypes::EndOfTrack;

    Track.AddEvent(event_t(   2500, event_t::Extended, (uint32_t) 0, Data, 2));

    container.AddTrack(Track);

    return true;
}

}
