
/** $VER: RunningNotes.cpp (2026.05.03) P. Stuer - Based on Valley Bell's rpc2mid (https://github.com/ValleyBell/MidiConverters). **/

#include "pch.h"

#include "RunningNotes.h"

namespace rcp
{

/// <summary>
/// Adds a note event to the "running notes" list, so that Note Off events can be inserted automatically by Check() while processing delays.
/// "length" specifies the number of ticks after which the note is turned off.
/// "velocity" specifies the velocity for the Note Off event. A value of 0x80 results in Note On with velocity 0.
/// Returns a pointer to the inserted struct or NULL if (NoteCnt >= NoteMax).
/// </summary>
void running_notes_t::Add(uint8_t channel, uint8_t note, uint8_t velocityOff, uint32_t deltaTime)
{
    if (_Count >= MAX_RUN_NOTES)
        return;

    running_note_t & rn = _Notes[_Count];

    rn.Channel = channel;
    rn.Code = note;
    rn.NoteOffVelocity = velocityOff;
    rn.DeltaTime = deltaTime;

    _Count++;
}

/// <summary>
/// Checks if any note expires within the N ticks specified by the "delay" parameter and
/// insert Note Off events when they do. In that case, the value of "delay" will be reduced.
/// Call this function from the delay handler and before extending notes.
/// Returns the number of expired notes.
/// </summary>
size_t running_notes_t::Check(midi_stream_t & stream, uint32_t & deltaTime)
{
    size_t ExpiredNotes = 0;

    while (_Count > 0)
    {
        uint32_t NewDeltaTime = deltaTime + 1;

        // 1. Check if we're going beyond a note's timeout.
        for (size_t i = 0; i < _Count; ++i)
        {
            running_note_t & n = _Notes[i];

            if (n.DeltaTime < NewDeltaTime)
                NewDeltaTime = n.DeltaTime;
        }

        if (NewDeltaTime > deltaTime)
            break; // The note is still playing. Continue processing the event.

        // 2. Advance all notes by X ticks.
        for (size_t i = 0; i < _Count; ++i)
            _Notes[i].DeltaTime -= NewDeltaTime;

        deltaTime -= NewDeltaTime;

        // 3. Send NoteOff for expired notes.
        for (size_t i = 0; i < _Count; ++i)
        {
            running_note_t & n = _Notes[i];

            if (n.DeltaTime > 0)
                continue;

            {
                stream.WriteVariableLengthQuantity(NewDeltaTime), NewDeltaTime = 0;

                stream.Ensure(3);

                if (n.NoteOffVelocity < 0x80)
                {
                    stream.Add((uint8_t) (midi::NoteOff | n.Channel));
                    stream.Add(n.Code);
                    stream.Add(n.NoteOffVelocity);
                }
                else
                {
                    stream.Add((uint8_t) (midi::NoteOn | n.Channel));
                    stream.Add(n.Code);
                    stream.Add(0);
                }
            }

            _Count--;

            if (_Count != 0)
                ::memmove(&n, &_Notes[(size_t) i + 1], ((size_t) _Count - i) * sizeof(running_note_t));

            i--;
            ExpiredNotes++;
        }
    }

    return ExpiredNotes;
}

/// <summary>
/// Writes Note Off events for all running notes.
/// </summary>
uint32_t running_notes_t::Flush(midi_stream_t & midiStream, bool cutNotes)
{
    uint32_t DeltaTime = midiStream.GetDuration();

    for (uint16_t i = 0; i < _Count; ++i)
    {
        if (_Notes[i].DeltaTime > DeltaTime)
        {
            if (cutNotes)
                _Notes[i].DeltaTime = DeltaTime; // Cut all notes at timestamp.
            else
                DeltaTime = _Notes[i].DeltaTime; // Remember the highest timestamp.
        }
    }

    Check(midiStream, DeltaTime);

    return DeltaTime;
}

}
