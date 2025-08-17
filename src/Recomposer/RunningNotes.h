
/** $VER: RunningNotes.h (2025.06.09) P. Stuer - Based on Valley Bell's rpc2mid (https://github.com/ValleyBell/MidiConverters). **/

#pragma once

#include "pch.h"

#include "MIDIStream.h"

namespace rcp
{

#define MAX_RUN_NOTES 32 // Should be more than enough even for the MIDI sequences

struct running_note_t
{
    running_note_t() : Channel(), Code(), NoteOffVelocity(), Duration()
    {
    }

    uint8_t Channel;
    uint8_t Code;
    uint8_t NoteOffVelocity;
    uint32_t Duration;
};

class running_notes_t
{
public:
    running_notes_t()
    {
        Reset();
    }

    void Reset()
    {
        _Count = 0;

        _Notes.clear();

        for (size_t i = 0; i < MAX_RUN_NOTES; ++i)
            _Notes.push_back(running_note_t());
    }

    void Add(uint8_t channel, uint8_t note, uint8_t velOff, uint32_t length);
    size_t Check(midi_stream_t & midiStream, uint32_t & timestamp);
    uint32_t Flush(midi_stream_t & midiStream, bool shorten);

public:
    size_t _Count;
    std::vector<running_note_t> _Notes;
};

}
