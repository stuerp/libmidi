
/** $VER: RunningNotes.cpp (2024.05.09) P. Stuer **/

#include "RunningNotes.h"

// Adds a note event to the "runNotes" list, so that NoteOff events can be inserted automatically by Check() while processing delays.
// "length" specifies the number of ticks after which the note is turned off.
// "velOff" specifies the velocity for the Note Off event. A value of 0x80 results in Note On with velocity 0.
// Returns a pointer to the inserted struct or NULL if (NoteCnt >= NoteMax).
running_note_t * running_notes_t::Add(uint8_t channel, uint8_t note, uint8_t velocityOff, uint32_t duration)
{
    running_note_t * rn;

    if (_Count >= MAX_RUN_NOTES)
        return nullptr;

    rn = &_Notes[_Count];

    rn->Channel = channel;
    rn->Note = note;
    rn->NoteOffVelocity = velocityOff;
    rn->Duration = duration;

    _Count++;

    return rn;
}

// Checks, if any note expires within the N ticks specified by the "delay" parameter and
// insert respective Note Off events. (In that case, the value of "delay" will be reduced.)
// Call this function from the delay handler and before extending notes.
// Returns the number of expired notes.
uint16_t running_notes_t::Check(midi_stream_t & midiStream, uint32_t * duration)
{
    uint16_t expiredNotes = 0;

    while (_Count > 0)
    {
        uint32_t NewDuration = *duration + 1;

        // 1. Check if we're going beyond a note's timeout.
        for (size_t i = 0; i < _Count; ++i)
        {
            running_note_t * n = &_Notes[i];

            if (n->Duration < NewDuration)
                NewDuration = n->Duration;
        }

        if (NewDuration > *duration)
            break; // not beyond the timeout - do the event

        // 2. advance all notes by X ticks
        for (size_t i = 0; i < _Count; ++i)
            _Notes[i].Duration -= NewDuration;

        (*duration) -= NewDuration;

        // 3. send NoteOff for expired notes
        for (size_t i = 0; i < _Count; ++i)
        {
            running_note_t * n = &_Notes[i];

            if (n->Duration > 0)
                continue;

            // turn note off, if going beyond the Timeout
            midiStream.EncodeVariableLengthQuantity(NewDuration);
            NewDuration = 0;

            midiStream.Grow(3);

            if (n->NoteOffVelocity < 0x80)
            {
                midiStream._Data[midiStream._Offs++] = 0x80 | n->Channel;
                midiStream._Data[midiStream._Offs++] = n->Note;
                midiStream._Data[midiStream._Offs++] = n->NoteOffVelocity;
            }
            else
            {
                midiStream._Data[midiStream._Offs++] = 0x90 | n->Channel;
                midiStream._Data[midiStream._Offs++] = n->Note;
                midiStream._Data[midiStream._Offs++] = 0x00;
            }

            _Count--;
            ::memmove(n, &_Notes[(size_t) i + 1], ((size_t) _Count - i) * sizeof(running_note_t));

            i--;
            expiredNotes++;
        }
    }

    return expiredNotes;
}

// Writes Note Off events for all running notes.
// "cutNotes" = false -> all notes are played fully (even if "delay" is smaller than the longest note)
// "cutNotes" = true -> notes playing after "delay" ticks are cut there
uint32_t running_notes_t::Flush(midi_stream_t & midiStream, bool shorten)
{
    uint32_t Timestamp = midiStream.GetTimestamp();

    for (uint16_t i = 0; i < _Count; i++)
    {
        if (_Notes[i].Duration > Timestamp)
        {
            if (shorten)
                _Notes[i].Duration = Timestamp; // Cut all notes at timestamp.
            else
                Timestamp = _Notes[i].Duration; // Remember the highest timestamp.
        }
    }

    Check(midiStream, &Timestamp);

    return Timestamp;
}
