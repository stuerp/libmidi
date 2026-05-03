
/** $VER: RunningNotes.h (2026.05.03) P. Stuer - Based on Valley Bell's mmd2mid (https://github.com/ValleyBell/MidiConverters). **/

#pragma once

#include <CppCoreCheck\Warnings.h>

#pragma warning(disable: 4100 4625 4626 4710 4711 4738 5045 ALL_CPPCORECHECK_WARNINGS)

#include <cstdint>
#include <cstring>

#include "MemoryStream.h"

namespace mmd
{

struct running_note_t
{
    uint8_t Channel;
    uint8_t Note;
    uint8_t Velocity;
    uint32_t Length;
};

class running_notes_t
{
public:
    running_notes_t() : _Count() { }

    running_note_t * Add(uint8_t channel, uint8_t note, uint8_t velocity, uint32_t length) noexcept;
    size_t Check(memory_stream_t * file, uint32_t * delay) noexcept;
    void Flush(memory_stream_t * file, uint32_t * delay) noexcept;

public:
    static const size_t MaxItems = 32;

    running_note_t _Items[MaxItems];
    size_t _Count;
};

/// <summary>
/// Adds a note event to the "running notes" list, so that Note Off events can be inserted automatically by Check() while processing delays.
/// "length" specifies the number of ticks after which the note is turned off.
/// "velocity" specifies the velocity for the Note Off event. A value of 0x80 results in Note On with velocity 0.
/// Returns a pointer to the inserted struct or NULL if (NoteCnt >= NoteMax).
/// </summary>
running_note_t * running_notes_t::Add(uint8_t channel, uint8_t note, uint8_t velocity, uint32_t length) noexcept
{
    if (_Count >= MaxItems)
        return nullptr;

    running_note_t * rn = &_Items[_Count++];

    rn->Channel  = channel;
    rn->Note     = note;
    rn->Velocity = velocity;
    rn->Length   = length;

    return rn;
}

/// <summary>
/// Checks if any note expires within the N ticks specified by the "delay" parameter and
/// insert Note Off events when they do. In that case, the value of "delay" will be reduced.
/// Call this function from the delay handler and before extending notes.
/// Returns the number of expired notes.
/// </summary>
size_t running_notes_t::Check(memory_stream_t * file, uint32_t * delay) noexcept
{
    size_t ExpiredNotes = 0;

    while (_Count > 0)
    {
        // 1. Check if we're going beyond a note's timeout.
        uint32_t Delay = *delay + 1;

        for (size_t i = 0; i < _Count; ++i)
        {
            running_note_t * tn = &_Items[i];

            if (tn->Length < Delay)
                Delay = tn->Length;
        }

        if (Delay > *delay)
            break; // Not timed-out. Do the event.

        // 2. Reduce the remaining length of each note.
        for (size_t i = 0; i < _Count; ++i)
            _Items[i].Length -= (uint32_t) Delay;

        (*delay) -= Delay;

        // 3. Add a Note Off event for expired notes.
        for (size_t i = 0; i < _Count; ++i)
        {
            running_note_t * tn = &_Items[i];

            if (tn->Length > 0)
                continue;

            file->WriteVariableLengthValue(Delay);
            Delay = 0;

            file->Grow(3u);

            if (tn->Velocity < 0x80)
            {
                file->Data[file->Offset++] = 0x80u | tn->Channel;
                file->Data[file->Offset++] = tn->Note;
                file->Data[file->Offset++] = tn->Velocity;
            }
            else
            {
                file->Data[file->Offset++] = 0x90u | tn->Channel;
                file->Data[file->Offset++] = tn->Note;
                file->Data[file->Offset++] = 0x00u;
            }

            _Count--;
            ::memmove(tn, &_Items[i + 1], (_Count - i) * sizeof(_Items[0]));
            i--;

            ExpiredNotes++;
        }
    }

    return ExpiredNotes;
}

/// <summary>
/// Writes Note Off events for all running notes.
/// </summary>
void running_notes_t::Flush(memory_stream_t * file, uint32_t * delay) noexcept
{
    for (size_t i = 0; i < _Count; ++i)
    {
        if (_Items[i].Length > *delay)
            *delay = _Items[i].Length; // Set "delay" to longest note.
    }

    Check(file, delay);
}

/// <summary>
/// Adjusts the value of LoopCount so that all tracks play for the approximately same time.
/// Ignore loops that are shorter than minLoopTicks.
/// Returns the number of adjusted tracks.
/// </summary>
static uint16_t AdjustTracks(track_t * tracks, uint16_t trackCount, uint32_t minLoopTicks) noexcept
{
    uint32_t MaxLength = 0;

    // Determine the longest track.
    for (uint16_t TrackIndex = 0; TrackIndex < trackCount; ++TrackIndex)
    {
        const track_t * Track = &tracks[TrackIndex];

        uint32_t TrackLength = Track->Length;

        if (Track->LoopCount != 0)
        {
            const uint32_t LoopLength = Track->Length - Track->LoopLength;

            TrackLength += (LoopLength * (Track->LoopCount - 1));
        }

        if (MaxLength < TrackLength)
            MaxLength = TrackLength;
    }

    uint16_t AdjustedTrackCount = 0;

    for (uint16_t TrackIndex = 0; TrackIndex < trackCount; ++TrackIndex)
    {
        track_t * Track = &tracks[TrackIndex];

        const uint32_t LoopLength = (Track->LoopCount != 0) ? (Track->Length - Track->LoopCount) : 0;

        if (LoopLength < minLoopTicks)
            continue; // Ignore tracks with very short loops

        // heuristic: The track needs additional loops, if the longest track is longer than the current track + 1/4 loop.
        uint32_t TrackLength = Track->Length + LoopLength * (Track->LoopCount - 1);

        if (TrackLength + LoopLength / 4 < MaxLength)
        {
            TrackLength = MaxLength - Track->LoopLength; // desired length of the loop

            Track->LoopCount = (uint16_t)((TrackLength + LoopLength / 3) / LoopLength);

            ++AdjustedTrackCount;
        }
    }

    return AdjustedTrackCount;
}

}
