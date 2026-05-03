
/** $VER: MMD.cpp (2026.05.03) P. Stuer - Based on Valley Bell's mmd2mid (https://github.com/ValleyBell/MidiConverters). **/

#include "pch.h"

#include <CppCoreCheck\Warnings.h>

#pragma warning(disable: 4100 4625 4626 4710 4711 4738 4820 5045 ALL_CPPCORECHECK_WARNINGS)

#include <cstdint>
#include <cstdlib>

#include <vector>

#include <string.h>

#include "MMD.h"

#include <MIDI.h>
#include <Support.h>

namespace mmd
{

struct mmd_t
{
    uint8_t Tempo;
    int8_t Transpose;
    const uint8_t * SysEx[8];
    const char * Title;
};

struct track_t
{
    uint32_t Offset;
    uint32_t Length;

    uint32_t LoopOffset;
    uint32_t LoopLength;
    uint16_t MaxLoopExpansions;     // Number of times to take the loops of this track

    int8_t Transpose;
    uint8_t Channel;
};

struct loop_t
{
    uint8_t Command[4];
    uint32_t Offset;
    uint32_t Length;
    uint16_t Count;
};

}

#include "MemoryStream.h"
#include "RunningNotes.h"

namespace mmd
{

static uint8_t GetDeltaTime(memory_stream_t * stream, uint32_t & deltaTime);

static running_notes_t _RunningNotes;

static uint8_t ParseTrack(const uint8_t * data, uint32_t size, const mmd_t * mmd, track_t * track);
static uint8_t ConvertTrack(const uint8_t * data, uint32_t size, const mmd_t * mmd, track_t * track, memory_stream_t * ms, midi_state_t * state, uint8_t trackNumber);
static size_t GetSysExSize(const uint8_t * data, size_t size, size_t startOffset) noexcept;
static void GetSysEx(const uint8_t * srcData, uint8_t param1, uint8_t param2, uint8_t channelNumber, std::vector<uint8_t> & dstData) noexcept;

inline uint32_t MMDTempo2MIDITemp(uint16_t bpm, uint8_t scale) noexcept;
static uint16_t ReadLE16(const uint8_t * data) noexcept;

const uint16_t MIDIResolution = 48u;

// Options
const uint16_t MaxLoopExpansions = 2u;  // Expand loops this many times.
const bool ExpandLoops = false;
const bool IgnoreMutedTracks = true;

/// <summary>
/// Converts the MMD data.
/// </summary>
uint8_t Convert(const uint8_t * srcData, uint32_t srcSize, std::vector<uint8_t> & dstData) noexcept
{
    memory_stream_t ms(0x20000, GetDeltaTime); // 128 KB

    uint32_t Offset = 2;

    track_t Tracks[18] = { };

    for (size_t i = 0; i < _countof(Tracks); ++i, Offset += 4)
    {
        track_t * Track = &Tracks[i];

        Track->Offset     = ReadLE16(&srcData[Offset]);
        Track->Length     = 0;

        Track->LoopOffset = 0;
        Track->LoopLength = 0;
        Track->MaxLoopExpansions  = 0;

        Track->Transpose  = (int8_t) srcData[Offset + 2];
        Track->Channel    = srcData[Offset + 3];


        if (Track->Offset >= srcSize)
            return 1; // Invalid track offset
    }

    Offset = ReadLE16(&srcData[0x4A]);

    mmd_t MMD =
    {
        .Tempo     = srcData[0x00],
        .Transpose = (int8_t) srcData[0x01]
    };

    for (size_t i = 0; i < _countof(MMD.SysEx); ++i)
        MMD.SysEx[i] = nullptr;

    // Princess Maker 1 (MMD v2.0) has the first track starting at 0x4A and no User SysEx pointers or data.
    if ((Tracks[0].Offset >= 0x4C) && (Offset != 0) && (Offset < srcSize))
    {
        for (size_t i = 0; (i < _countof(MMD.SysEx)) && (Offset + 0x01 < srcSize); ++i, Offset += 0x02)
        {
            const uint16_t SysExOffset = ReadLE16(&srcData[Offset]);

            if (SysExOffset >= srcSize)
                continue;

            MMD.SysEx[i] = &srcData[SysExOffset];
        }
    }

    if (Tracks[0].Offset > 0x50)
        MMD.Title = (const char *) &srcData[0x50];

    for (size_t i = 0; i < _countof(Tracks); ++i)
    {
        track_t * Track = &Tracks[i];

        ParseTrack(srcData, srcSize, &MMD, Track);

        Track->MaxLoopExpansions = (Track->LoopOffset != 0) ? MaxLoopExpansions : 0u;
    }

    if (ExpandLoops)
        AdjustTracks(Tracks, _countof(Tracks), (uint32_t) (MIDIResolution / 4));

    // Write the MIDI header with default values.
    ms.WriteHeader(0x0001, 0, MIDIResolution);

    uint8_t Result = 0;

    {
        midi_state_t State;
        uint8_t TrackNumber;

        for (TrackNumber = 0; TrackNumber < _countof(Tracks); ++TrackNumber)
        {
            ms.WriteTrackBegin(&State);

            if (TrackNumber == 0)
            {
                if ((MMD.Title != nullptr) && (MMD.Title[0] != '\0'))
                    ms.WriteMetaEvent(&State, midi::MetaDataType::TrackName, MMD.Title, (uint32_t) ::strlen(MMD.Title));

                const uint32_t Tempo = MMDTempo2MIDITemp(MMD.Tempo, 64);

                uint8_t Data[32] = { };

                WriteBE32(Data, Tempo);

                ms.WriteMetaEvent(&State, midi::MetaDataType::SetTempo, Data + 1, 3);
            }

            Result = ConvertTrack(srcData, srcSize, &MMD, &Tracks[TrackNumber], &ms, &State, TrackNumber);

            ms.WriteEvent(&State, midi::StatusCode::MetaData, midi::MetaDataType::EndOfTrack, 0x00);

            ms.WriteTrackEnd(&State);

            if (Result != 0)
                break;
        }

        // Update the MIDI header with the actual track count and length.
        {
            Offset = (uint32_t) ms.Offset;

            ms.Offset = 0;
            ms.WriteHeader(0x0001, TrackNumber, MIDIResolution);

            dstData.assign(ms.Data, ms.Data + Offset);
        }
    }

    return Result;
}

/// <summary>
/// 
/// </summary>
static uint8_t ParseTrack(const uint8_t * data, uint32_t size, const mmd_t * mmd, track_t * track)
{
    track->Length = 0;
    track->LoopOffset = 0;
    track->LoopLength = 0;

    if (track->Offset >= size)
        return 1;

    size_t Offset = track->Offset;

    bool EndOfTrack = false;

    uint8_t Command[4] = { };
    loop_t Loops[16] = { };

    uint8_t LoopIndex = 0;

    while (Offset < size && !EndOfTrack)
    {
        uint8_t CommandType = data[Offset];

        if (msc::InRange((int) CommandType, 0x80, 0x8F))
        {
            uint8_t CommandMask = (uint8_t) (CommandType & 0x0F);

            Offset++;

            for (size_t i = 0; i < _countof(Command); ++i, CommandMask <<= 1)
            {
                if (CommandMask & 0x08)
                    Command[i] = data[Offset++];
            }
        }
        else
        {
            for (size_t i = 0; i < _countof(Command); ++i)
                Command[i] = data[Offset++];
        }

        CommandType = Command[0];

        uint8_t CommandDelay = (CommandType >= 0xF0) ? 0u : Command[1];

        switch (CommandType)
        {
            case 0x98: // Send SysEx command
            {
                Offset += GetSysExSize(data, size, Offset);
                break;
            }

            case 0xF8: // Loop End
            {
                if (LoopIndex > 0)
                {
                    Loops[--LoopIndex].Count++;

                    if ((Command[1] == 0) || (Command[1] >= 0x7F))
                    {
                        track->LoopOffset = Loops[LoopIndex].Offset;
                        track->LoopLength = Loops[LoopIndex].Length;

                        EndOfTrack = true;
                    }
                    else
                    if (Loops[LoopIndex].Count < Command[1])
                    {
                        ::memcpy(Command, Loops[LoopIndex].Command, sizeof(Command));

                        Offset = Loops[LoopIndex++].Offset;
                    }
                }
                break;
            }

            case 0xF9: // Loop Begin
            {
                if (LoopIndex < _countof(Loops))
                {
                    ::memcpy(Loops[LoopIndex].Command, Command, sizeof(Command));

                    Loops[LoopIndex].Offset = (uint32_t) Offset;
                    Loops[LoopIndex].Length = track->Length;
                    Loops[LoopIndex].Count = 0;

                    if ((LoopIndex > 0) && (Loops[LoopIndex].Offset == Loops[LoopIndex - 1].Offset))
                        LoopIndex--; // Ignore this loop command.

                    LoopIndex++;
                }
                break;
            }

            case 0xFE: // Track End
            {
                EndOfTrack = true;
                break;
            }
        }

        track->Length += CommandDelay;
    }

    return 0;
}

/// <summary>
/// Converts an MMD track to MIDI events.
/// </summary>
static uint8_t ConvertTrack(const uint8_t * data, uint32_t size, const mmd_t * mmd, track_t * track, memory_stream_t * ms, midi_state_t * state, uint8_t trackNumber)
{
    if (track->Offset >= size)
        return 1;

    uint8_t PortNumber = 0;
    uint8_t ChannelNumber = 0;

    if (track->Channel == 0xFF)
    {
        PortNumber = IgnoreMutedTracks ? 0xFF : 0x00;
        ChannelNumber = 0;

        track->Channel = 0x00;
    }
    else
    {
        PortNumber = (uint8_t) (track->Channel >> 4);
        ChannelNumber = track->Channel & 0x0Fu;
    }

    if (PortNumber != 0xFF)
    {
        ms->WriteMetaEvent(state, midi::MetaDataType::MIDIPort, &PortNumber, 1);
        ms->WriteMetaEvent(state, midi::MetaDataType::ChannelPrefix, &ChannelNumber, 1);
    }

    int8_t Transpose = track->Transpose;

    if (Transpose & 0x80)
    {
        Transpose = 0; // Ignore transposition setting for rhythm tracks
    }
    else
    {
        Transpose = (Transpose & 0x40) ? (-0x80 + Transpose) : Transpose; // 7-bit -> 8-bit sign extension

        Transpose += mmd->Transpose; // Add global transposition
    }

    bool EndOfTrack = false;

    _RunningNotes._Count = 0;

    state->Channel = ChannelNumber;
    state->DeltaTime = 0;

    size_t Offset = track->Offset;
    size_t LoopIndex = 0;

    uint8_t Command[4] = { };
    loop_t Loops[16] = { };
    uint8_t GSParameters[6] = { }; // 0 device ID, 1 model ID, 2 address high, 3 address low

    while ((Offset < size) && !EndOfTrack)
    {
        uint8_t CommandType = data[Offset];

        if (msc::InRange((int) CommandType, 0x80, 0x8F))
        {
            uint8_t CommandMask = CommandType & 0x0Fu;

            Offset++;

            for (size_t i = 0; i < _countof(Command); ++i, CommandMask <<= 1)
            {
                if (CommandMask & 0x08)
                    Command[i] = data[Offset++];
            }
        }
        else
        {
            for (size_t i = 0; i < _countof(Command); ++i)
                Command[i] = data[Offset++];
        }

        CommandType = Command[0];

        uint8_t CommandDelay = (CommandType >= 0xF0) ? 0u : Command[1];

        if (CommandType < 0x80)
        {
            uint8_t Note = 0;

            const uint8_t Duration = Command[2];

            // Emit a note if the duration and the velocity are non-zero.
            bool EmitNote = (Duration > 0) && (Command[3] > 0);

            if (EmitNote)
            {
                _RunningNotes.Check(ms, state->DeltaTime);

                Note = (CommandType + Transpose) & 0x7Fu;

                for (size_t i = 0; i < _RunningNotes._Count; ++i)
                {
                    if (_RunningNotes._Items[i].Note == Note)
                    {
                        // The note is already playing. Set a new length.
                        _RunningNotes._Items[i].Length = (uint32_t) state->DeltaTime + Duration;

                        EmitNote = false; // Don't emit a new note.
                        break;
                    }
                }
            }

            if (EmitNote && (PortNumber != 0xFF))
            {
                ms->WriteEvent(state, midi::StatusCode::NoteOn, Note, Command[3]);

                _RunningNotes.Add(state->Channel, Note, 0x80, Duration);
            }
        }
        else
        {
            switch (CommandType)
            {
                case 0x90: case 0x91: case 0x92: case 0x93: // Send User SysEx
                case 0x94: case 0x95: case 0x96: case 0x97:
                {
                    if (PortNumber == 0xFF)
                        break;

                    const uint8_t * UserSysEx = mmd->SysEx[CommandType & 0x07];

                    if (UserSysEx == nullptr)
                        break; // Undefined User SysEx

                    std::vector<uint8_t> SysEx;

                    GetSysEx(UserSysEx, Command[2], Command[3], ChannelNumber, SysEx);

                    if (SysEx.size() > 1)
                        ms->WriteEvent(state, midi::StatusCode::SysEx, SysEx.data(), (uint32_t) SysEx.size());
                    break;
                }

                case 0x98: // Send SysEx
                {
                    const size_t SrcSize = GetSysExSize(data, size, Offset);

                    if (PortNumber != 0xFF)
                    {
                        std::vector<uint8_t> SysEx;

                        GetSysEx(&data[Offset], Command[2], Command[3], ChannelNumber, SysEx);

                        if (SysEx.size() > 1)
                            ms->WriteEvent(state, midi::StatusCode::SysEx, SysEx.data(), (uint32_t) SysEx.size());
                    }

                    Offset += SrcSize;
                    break;
                }

                case 0xC0: // DX7 Function
                case 0xC1: // DX Parameter
                case 0xC2: // DX RERF
                case 0xC3: // TX Function

                case 0xC7: // TX81Z V VCED
                case 0xC8: // TX81Z A ACED
                case 0xC9: // TX81Z P PCED

                case 0xCC: // DX7-2 R Remote SW
                case 0xCD: // DX7-2 A ACED
                case 0xCE: // DX7-2 P PCED
                {
                    if (PortNumber == 0xFF)
                        break;

                    static const uint8_t DXParameters[16] =
                    {
                        0x08, 0x00, 0x04, 0x11, 0xFF, 0x15, 0xFF, 0x12,
                        0x13, 0x10, 0xFF, 0xFF, 0x1B, 0x18, 0x19, 0x1A,
                    };

                    uint8_t Data[64] = { };

                    Data[0] = 0x43u; // Yamaha
                    Data[1] = 0x10u | state->Channel;
                    Data[2] = (uint8_t) (DXParameters[CommandType & 0x0F] | (Command[2] >> 7));
                    Data[3] = Command[2];
                    Data[4] = Command[3];
                    Data[5] = 0xF7u;

                    ms->WriteEvent(state, midi::StatusCode::SysEx, Data, 6);
                    break;
                }

                case 0xC5: // FB-01 P Parameter
                {
                    if (PortNumber == 0xFF)
                        break;

                    uint8_t Data[64] = { };

                    Data[0] = 0x43u; // Yamaha
                    Data[1] = 0x10u | state->Channel;
                    Data[2] = 0x15u;
                    Data[3] = Command[2];

                    if (Command[2] < 0x40u)
                    {
                        Data[4] = Command[3];
                        Data[5] = Command[3] & 0x0Fu;
                        Data[6] = (uint8_t) (Command[3] >> 4);
                        Data[7] = 0xF7u;

                        ms->WriteEvent(state, midi::StatusCode::SysEx, Data, 8);
                    }
                    else
                    {
                        Data[4] = Command[3] & 0x0Fu;
                        Data[5] = (uint8_t) (Command[3] >> 4);
                        Data[6] = 0xF7u;

                        ms->WriteEvent(state, midi::StatusCode::SysEx, Data, 7);
                    }
                    break;
                }

                case 0xC6: // FB-01 S System
                {
                    if (PortNumber == 0xFF)
                        break;

                    uint8_t Data[64] = { };

                    Data[0] = 0x43; // Yamaha
                    Data[1] = 0x75;
                    Data[2] = state->Channel;
                    Data[3] = 0x10;
                    Data[4] = Command[2];
                    Data[5] = Command[3];
                    Data[6] = 0xF7;

                    ms->WriteEvent(state, midi::StatusCode::SysEx, Data, 7);
                    break;
                }

                case 0xCA: // TX81Z S System
                case 0xCB: // TX81Z E EFFECT
                {
                    if (PortNumber == 0xFF)
                        break;

                    uint8_t Data[64] = { };

                    Data[0] = 0x43; // Yamaha
                    Data[1] = (uint8_t) (0x10 | state->Channel);
                    Data[2] = (uint8_t) (0x7B + (CommandType - 0xCA)); // command CA -> param = 7B, command CB -> param = 7C
                    Data[3] = Command[2];
                    Data[4] = Command[3];
                    Data[5] = 0xF7;

                    ms->WriteEvent(state, 0xF0, Data, 6);
                    break;
                }

                case 0xCF: // TX802 P PCED
                {
                    if (PortNumber == 0xFF)
                        break;

                    uint8_t Data[64] = { };

                    Data[0] = 0x43; // Yamaha
                    Data[1] = (uint8_t) (0x10 | state->Channel);
                    Data[2] = 0x1A;
                    Data[3] = Command[2];

                    if ((Command[2] < 0x1B) || (Command[2] >= 0x60))
                    {
                        Data[4] = Command[3];
                        Data[5] = 0xF7;

                        ms->WriteEvent(state, midi::StatusCode::SysEx, Data, 6);
                    }
                    else
                    {
                        Data[4] = (uint8_t) (Command[3] >> 7);
                        Data[5] = (uint8_t) (Command[3] & 0x7F);
                        Data[6] = 0xF7;

                        ms->WriteEvent(state, midi::StatusCode::SysEx, Data, 7);
                    }
                    break;
                }

            //  case 0xD0: // Yamaha Base Address
            //  case 0xD1: // Yamaha Device Data
            //  case 0xD2: // Yamaha Address / Parameter
            //  case 0xD3: // Yamaha XG Address / Parameter D0..D3 is not supported by MMD 2.2

                case 0xDC: // MKS-7
                {
                    if (PortNumber == 0xFF)
                        break;

                    uint8_t Data[64] = { };

                    Data[0] = 0x41; // Roland ID
                    Data[1] = 0x32;
                    Data[2] = state->Channel;
                    Data[3] = Command[2];
                    Data[4] = Command[3];
                    Data[5] = 0xF7;

                    ms->WriteEvent(state, midi::StatusCode::SysEx, Data, 6);
                    break;
                }

                case 0xDD: // Roland Base Address
                {
                    GSParameters[2] = Command[2];
                    GSParameters[3] = Command[3];
                    break;
                }

                case 0xDE: // Roland Parameter
                {
                    GSParameters[4] = Command[2];
                    GSParameters[5] = Command[3];

                    if (PortNumber == 0xFF)
                        break;

                    uint8_t Data[64] = { };

                    Data[0] = 0x41; // Roland
                    Data[1] = GSParameters[0];
                    Data[2] = GSParameters[1];
                    Data[3] = 0x12;

                    uint8_t Checksum = 0;

                    for (size_t i = 0; i < 4; ++i)
                    {
                        Data[4 + i] = GSParameters[2 + i];
                        Checksum += GSParameters[2 + i];
                    }

                    Data[8] = (uint8_t) ((0x100 - Checksum) & 0x7F);
                    Data[9] = 0xF7;

                    ms->WriteEvent(state, midi::StatusCode::SysEx, Data, 10);
                    break;
                }

                case 0xDF: // Roland Device
                {
                    GSParameters[0] = Command[2];
                    GSParameters[1] = Command[3];
                    break;
                }

                case 0xE2: // Set GS instrument
                {
                    if (PortNumber == 0xFF)
                        break;

                    ms->WriteEvent(state, midi::StatusCode::ControlChange, midi::Controller::BankSelect, Command[3]);
                    ms->WriteEvent(state, midi::StatusCode::ControlChange, midi::Controller::BankSelectLSB, 0x00);
                    ms->WriteEvent(state, midi::StatusCode::ProgramChange, Command[2], 0x00);
                    break;
                }

                case 0xE6: // MIDI channel number
                {
                    const uint8_t Byte = Command[2] - 1u; // It's same as in the track header, except 1 added.

                    if (Byte == 0xFF)
                    {
                        if (IgnoreMutedTracks)
                        {
                            PortNumber = 0xFF;
                            ChannelNumber = 0x00;
                        }
                    }
                    else
                    {
                        PortNumber  = (uint8_t) (Byte >> 4); // port ID
                        ChannelNumber = (uint8_t) (Byte & 0x0F); // channel ID

                        ms->WriteMetaEvent(state, midi::MetaDataType::MIDIPort, &ChannelNumber, 1);
                        ms->WriteMetaEvent(state, midi::MetaDataType::ChannelPrefix, &ChannelNumber, 1);
                    }

                    state->Channel = ChannelNumber;
                    break;
                }

                case 0xE7: // Tempo Modifier
                {
//                  if (Command[3] != 0)
//                      ::printf("Track %u: Gradual Tempo Change 0x%02X at 0x%04X.\n", trackNumber, Command[3], OldOffset);

                    const uint32_t Tempo = MMDTempo2MIDITemp(mmd->Tempo, Command[2]);

                    uint8_t Data[32] = { };

                    WriteBE32(Data, Tempo);

                    ms->WriteMetaEvent(state, midi::MetaDataType::SetTempo, Data + 1, 3);
                    break;
                }

                case 0xEA: // Channel Aftertouch
                {
                    if (PortNumber == 0xFF)
                        break;

                    ms->WriteEvent(state, midi::StatusCode::ChannelPressure, Command[2], 0x00);
                    break;
                }

                case 0xEB: // Control Change
                {
                    if (PortNumber == 0xFF)
                        break;

                    ms->WriteEvent(state, midi::StatusCode::ControlChange, Command[2], Command[3]);
                    break;
                }

                case 0xEC: // Instrument
                {
                    if (PortNumber == 0xFF)
                        break;

                    ms->WriteEvent(state, midi::StatusCode::ProgramChange, Command[2], 0x00);
                    break;
                }

                case 0xED: // Note Aftertouch
                {
                    if (PortNumber == 0xFF)
                        break;

                    ms->WriteEvent(state, midi::StatusCode::KeyPressure, Command[2], Command[3]);
                    break;
                }

                case 0xEE: // Pitch Bend
                {
                    if (PortNumber == 0xFF)
                        break;

                    ms->WriteEvent(state, midi::StatusCode::PitchBendChange, Command[2], Command[3]);
                    break;
                }

            //  case 0xF6: // Comment
            //  case 0xF7: // Continuation of previous command

                case 0xF8: // Loop End
                {
                    if (LoopIndex == 0)
                        break; // Loop End without Loop Begin

                    LoopIndex--;

                    Loops[LoopIndex].Count++;

                    bool TakeLoop = false;

                    if ((Command[1] == 0) || (Command[1] >= 0x7F))
                    {
                        // Infinite loop
                        if ((Loops[LoopIndex].Count < 0x80) && (PortNumber != 0xFF))
                            ms->WriteEvent(state, midi::StatusCode::ControlChange, 0x6F, (uint8_t) Loops[LoopIndex].Count); // Set an RPG Maker Loop marker.

                        if (Loops[LoopIndex].Count < track->MaxLoopExpansions)
                            TakeLoop = true;
                    }
                    else
                    {
                        if (Loops[LoopIndex].Count < Command[1])
                            TakeLoop = true;
                    }

                    if (TakeLoop)
                    {
                        ::memcpy(Command, Loops[LoopIndex].Command, sizeof(Command));

                        Offset = Loops[LoopIndex++].Offset;
                    }

                    break;
                }

                case 0xF9: // Loop Begin
                {
                    if (LoopIndex >= _countof(Loops))
                        break; // Too many nested loops

                    if ((Offset == track->LoopOffset) && (PortNumber != 0xFF))
                        ms->WriteEvent(state, midi::StatusCode::ControlChange, 0x6F, 0); // Set an RPG Maker Loop marker.

                    ::memcpy(Loops[LoopIndex].Command, Command, sizeof(Command));

                    Loops[LoopIndex].Offset = (uint32_t) Offset;
                    Loops[LoopIndex].Count = 0;

                    if ((LoopIndex > 0) && (Loops[LoopIndex].Offset == Loops[LoopIndex - 1].Offset))
                        LoopIndex--; // Ignore bad loop command

                    LoopIndex++;
                    break;
                }

                case 0xFE: // Track End
                case 0xFF:
                {
                    EndOfTrack = true;
                    break;
                }

                default:
                {
                    // Unknown MMD command
                    ms->WriteEvent(state, midi::StatusCode::ControlChange, 0x70, CommandType & 0x7Fu);
                    break;
                }
            }
        }

        state->DeltaTime += CommandDelay;
    }

    _RunningNotes.Flush(ms, state->DeltaTime);

    if (PortNumber == 0xFF)
        state->DeltaTime = 0;

    return 0;
}

/// <summary>
/// 
/// </summary>
static size_t GetSysExSize(const uint8_t * data, size_t size, size_t startOffset) noexcept
{
    size_t Offset = startOffset;

    while ((Offset < size) && (data[Offset] != 0xF7))
        Offset++;

    if (Offset < size)
        Offset++; // Include terminating F7 byte

    return Offset - startOffset;
}

/// <summary>
/// 
/// </summary>
static void GetSysEx(const uint8_t * srcData, uint8_t param1, uint8_t param2, uint8_t channelNumber, std::vector<uint8_t> & dstData) noexcept
{
    if (srcData == nullptr)
        return;

    uint8_t Checksum = 0;

    for (size_t SrcOffset = 0; ; ++SrcOffset)
    {
        uint8_t Data = srcData[SrcOffset];

        if (Data & 0x80)
        {
            switch (Data)
            {
                case 0x80: // Store data value (cmdMem[2])
                    Data = param1;
                    break;

                case 0x81: // Store data value (cmdMem[3])
                    Data = param2;
                    break;

                case 0x82: // Store data value (MIDI channel)
                    Data = channelNumber;
                    break;

                case 0x83: // Initialize Roland checksum.
                    Checksum = 0x00;
                    break;

                case 0x84: // Store the Roland checksum.
                    Data = (uint8_t) ((0x100 - Checksum) & 0x7F);
                    break;

                case 0xF7: // SysEx end
                    dstData.push_back(Data) ;

                    return;

                default:
                    break; // Unknown MMD command
            }
        }

        if (!(Data & 0x80))
        {
            dstData.push_back(Data);
            Checksum += Data;
        }
    }
}

/// <summary>
/// 
/// </summary>
static uint8_t GetDeltaTime(memory_stream_t * ms, uint32_t & deltaTime)
{
    _RunningNotes.Check(ms, deltaTime);

    if (deltaTime != 0)
    {
        for (size_t i = 0; i < _RunningNotes._Count; ++i)
            _RunningNotes._Items[i].Length -= (uint16_t) deltaTime;
    }

    return 0;
}

/// <summary>
/// Converts MMD tempo to MIDI tempo. (60 000 000.0 / bpm) * (scale / 64.0)
/// </summary>
inline uint32_t MMDTempo2MIDITemp(uint16_t bpm, uint8_t scale) noexcept
{
    const uint32_t Denominator = (uint32_t) (bpm * scale);

    return (60000000u * 64u) / Denominator;
}

/// <summary>
/// Reads a little-endian 16-bit unsigned integer from the given data.
/// </summary>
inline uint16_t ReadLE16(const uint8_t * data) noexcept
{
    return (uint16_t) ((data[0x01] << 8) | (data[0x00] << 0));
}

}
