
/** $VER: RCP.cpp (2024.05.09) P. Stuer **/

#include "RCP.h"
#include "RunningNotes.h"

#include <stdexcept>
#include <vector>

extern running_notes_t RunningNotes;

#define MCMD_INI_EXCLUDE	0x00 // exclude initial command
#define MCMD_INI_INCLUDE	0x01 // include initial command
#define MCMD_RET_CMDCOUNT	0x00 // return number of commands
#define MCMD_RET_DATASIZE	0x02 // return number of data bytes

static uint16_t ProcessRCPSysEx(const uint8_t * srcData, uint16_t srcSize, uint8_t * dstData, uint8_t param1, uint8_t param2, uint8_t channel);
static uint8_t DetermineShift(uint32_t value);

/// <summary>
/// 
/// </summary>
void rcp_file_t::ReadTrack(const uint8_t * data, uint32_t size, uint32_t offset, rcp_track_t * rcpTrack) const
{
    if (offset >= size)
        throw std::runtime_error("Invalid start of track position.");

    uint32_t Offset = offset;

    const uint32_t TrackHead = Offset;

    uint32_t TrackSize = 0;

    if (_Version == 2)
    {
        TrackSize = ReadLE16(data + Offset);
        Offset += 2;
    }
    else
    if (_Version == 3)
    {
        TrackSize = ReadLE32(data + Offset);
        Offset += 4;
    }

    const uint32_t TailOffset = std::min(TrackHead + TrackSize, size);

    if (Offset + 0x2A > size)
        throw std::runtime_error("Insufficient data to read track header.");

    Offset += 0x2A; // Skip the track header.

    rcpTrack->Offs = TrackHead;
    rcpTrack->Size = TrackSize;
    rcpTrack->Duration = 0;

    rcpTrack->LoopStartOffs = 0;
    rcpTrack->LoopStartTick = 0;

    std::vector<uint32_t> MeasureOffsets;

    MeasureOffsets.reserve(256);

#ifdef _DEBUG
    ::printf("%04X: Track begin\n", Offset);
#endif

    MeasureOffsets.push_back(Offset);

    bool EndOfTrack = false;

    uint32_t ParentOffs = 0;

    struct loop_t
    {
        uint32_t ParentOffs;
        uint32_t StartOffs;
        uint32_t StartTick;
        uint16_t Counter;
    };

    std::vector<loop_t> Loops;

    Loops.reserve(8);

    uint8_t LoopCount = 0;

    uint32_t LoopParentOffs[8] = { };
    uint32_t LoopStartOffs[8] = { };
    uint32_t LoopStartTick[8] = { };
    uint16_t LoopCounter[8] = { };

    while ((Offset < TailOffset) && !EndOfTrack)
    {
        uint32_t OldOffset = Offset; // Offset of the start of the command

        uint8_t  CmdType     = 0;
        uint16_t CmdP0       = 0;
        uint8_t  CmdP1       = 0;
        uint8_t  CmdP2       = 0;
        uint16_t CmdDuration = 0;

        if (_Version == 2)
        {
            CmdType     = data[Offset + 0x00];
            CmdP0       = data[Offset + 0x01];
            CmdP1       = data[Offset + 0x02];
            CmdP2       = data[Offset + 0x03];
            CmdDuration = CmdP1;

            Offset += 4;
        }
        else
        if (_Version == 3)
        {
            CmdType     = data[Offset + 0x00];
            CmdP2       = data[Offset + 0x01];
            CmdP0       = ReadLE16(&data[Offset + 0x02]);
            CmdP1       = data[Offset + 0x04];
            CmdDuration = ReadLE16(&data[Offset + 0x04]);

            Offset += 6;
        }

        switch (CmdType)
        {
            case 0xF8: // Loop End
            {
                if (LoopCount > 0)
                {
                    LoopCount--;

                    LoopCounter[LoopCount]++;

                    if (CmdP0 == 0)
                    {
                        rcpTrack->LoopStartOffs = LoopStartOffs[LoopCount];
                        rcpTrack->LoopStartTick = LoopStartTick[LoopCount];

                        EndOfTrack = 1;
                    }
                    else
                    {
                        if (LoopCounter[LoopCount] < CmdP0)
                        {
                            ParentOffs = LoopParentOffs[LoopCount];
                            Offset = LoopStartOffs[LoopCount];

                            LoopCount++;
                        }
                    }
                }

                CmdP0 = 0;
                break;
            }

            case 0xF9: // Loop Begin
            {
                if (LoopCount < 8)
                {
                    LoopParentOffs[LoopCount] = ParentOffs;
                    LoopStartOffs[LoopCount] = Offset;
                    LoopStartTick[LoopCount] = rcpTrack->Duration;
                    LoopCounter[LoopCount] = 0;

                    LoopCount++;
                }

                Loops.push_back({ ParentOffs, Offset, rcpTrack->Duration, 0 });

                CmdP0 = 0;
                break;
            }

            case 0xFC: // Repeat previous bar
            {
                if (ParentOffs)
                {
                    Offset = ParentOffs;
                    ParentOffs = 0x00;
                }
                else
                {
                    if (_Version == 2)
                        Offset -= 0x04;
                    else
                    if (_Version == 3)
                        Offset -= 0x06;
                    do
                    {
                        uint32_t prevPos = Offset;
                        uint32_t repeatPos;
                        uint16_t measureID = 0;

                        if (_Version == 2)
                        {
                            CmdP0 = data[Offset + 0x01];
                            CmdP1 = data[Offset + 0x02];
                            CmdP2 = data[Offset + 0x03];
                            measureID = (CmdP0 << 0) | ((CmdP1 & 0x03) << 8);
                            repeatPos = ((CmdP1 & ~0x03) << 0) | (CmdP2 << 8);
                            Offset += 0x04;
                        }
                        else
                         if (_Version == 3)
                        {
                            measureID = ReadLE16(&data[Offset + 0x02]);
                            repeatPos = 0x002E + (ReadLE16(&data[Offset + 0x04]) - 0x0030) * 0x06;
                            Offset += 0x06;
                        }
                        if (measureID >= MeasureOffsets.size())
                            break;

                        if (TrackHead + repeatPos == prevPos)
                            break; // prevent recursion

                        if (!ParentOffs) // necessary for following FC command chain
                            ParentOffs = Offset;
                        Offset = TrackHead + repeatPos;
                        prevPos = Offset;
                    } while (data[Offset] == 0xFC);
                }

                CmdP0 = 0;
                break;
            }

            case 0xFD: // Measure end
            {
                if (MeasureOffsets.size() >= 0x8000)
                {
                #ifdef _DEBUG
                    ::printf("Warning: too many measures in track.\n");
                #endif

                    EndOfTrack = 1;
                    break;
                }
                if (ParentOffs)
                {
                    Offset = ParentOffs;
                    ParentOffs = 0x00;
                }

                MeasureOffsets.push_back(Offset);

                CmdP0 = 0;

                if (_Options._WolfteamLoopMode && MeasureOffsets.size() == 2)
                {
                    LoopCount = 0;

                    LoopParentOffs[LoopCount] = ParentOffs;
                    LoopStartOffs[LoopCount] = Offset;
                    LoopStartTick[LoopCount] = rcpTrack->Duration;
                    LoopCounter[LoopCount] = 0;

                    LoopCount++;

                    Loops.clear();

                    Loops.push_back({ ParentOffs, Offset, rcpTrack->Duration, 0 });
                }
                break;
            }

            case 0xFE: // Track end
            {
                EndOfTrack = true;
                CmdP0 = 0;

                if (_Options._WolfteamLoopMode && (LoopCount > 0))
                {
                    LoopCount = 0;

                    rcpTrack->LoopStartOffs = LoopStartOffs[LoopCount];
                    rcpTrack->LoopStartTick = LoopStartTick[LoopCount];

                    Loops.clear();
                }
                break;
            }

            default:
            {
                if (CmdType >= 0xF0)
                    CmdP0 = 0;
            }
        }

        rcpTrack->Duration += CmdP0;
    }

#ifdef _DEBUG
    ::printf("%04X: Track End: %d measures, %d ticks.\n\n", Offset, (int) MeasureOffsets.size(), rcpTrack->Duration);
#endif
}

/// <summary>
/// 
/// </summary>
void rcp_file_t::ConvertTrack(const uint8_t * data, uint32_t size, uint32_t * offset, rcp_track_t * track, midi_stream_t & midiStream) const
{
    uint32_t Offset = *offset;

    if (Offset >= size)
        throw std::runtime_error("Invalid start of track position.");

    const uint32_t TrackHead = Offset;

    uint32_t TrackSize = 0;

    if (_Version == 2)
    {
        TrackSize = ReadLE16(&data[Offset]);

        // Bits 0/1 are used as 16/17, allowing for up to 256 KB per track. This is used by some ItoR.x conversions.
        TrackSize = (TrackSize & ~0x03) | ((TrackSize & 0x03) << 16);
        Offset += 2;
    }
    else
    if (_Version == 3)
    {
        TrackSize = ReadLE32(&data[Offset]);
        Offset += 4;
    }

    uint32_t TrackTail = std::min(TrackHead + TrackSize, size);

    if (Offset + 0x2A > size)
        throw std::runtime_error("Insufficient data to read track header.");

    uint8_t TrackId    = data[Offset + 0x00]; // track ID
    uint8_t RhythmMode = data[Offset + 0x01]; // rhythm mode
    uint8_t SrcChannel = data[Offset + 0x02]; // MIDI channel
    uint8_t DstChannel = 0x00;

    if (SrcChannel & 0x80)
    {
        // When the KeepDummyCh option is off, prevent events from being written to the MIDI file by setting midiDev to 0xFF.
        DstChannel = _Options._KeepDummyChannels ? 0x00 : 0xFF;
        SrcChannel = 0x00;
    }
    else
    {
        DstChannel = SrcChannel >> 4;
        SrcChannel &= 0x0F;
    }

    int8_t Transposition =          data[Offset + 0x03];
    int32_t StartTick    = (int8_t) data[Offset + 0x04];
    uint8_t TrackMute    =          data[Offset + 0x05];

    rcp_string_t TrackName;

    TrackName.Assign(&data[Offset + 0x06], 0x24);

    Offset += 0x2A; // Skip the track header.

    // Write a conductor track with the initial setup.
    {
        uint32_t OldTimestamp = midiStream.GetTimestamp();

        midiStream.SetTimestamp(0); // Make sure all events on the conductor track have timestamp 0.

        if (TrackName.Len > 0)
            midiStream.WriteMetaEvent(MetaDataTypes::TrackName, TrackName.Data, TrackName.Len);

        if (TrackMute && !_Options._KeepDummyChannels)
        {
            // Ignore muted tracks.
            *offset = TrackHead + TrackSize;

            return;
        }

        if (DstChannel != 0xFF)
        {
            midiStream.WriteMetaEvent(MetaDataTypes::MIDIPort,      &DstChannel, 1);
            midiStream.WriteMetaEvent(MetaDataTypes::ChannelPrefix, &SrcChannel, 1);
        }

        // For RhythmMode, values 0 (melody channel) and 0x80 (rhythm channel) are common.
        // Some songs use different values, but the actual meaning of the value is unknown.
    #ifdef _DEBUG
        if ((RhythmMode != 0) && (RhythmMode != 0x80))
            ::printf("Warning: Track %2u: Unknown Rhythm Mode 0x%02X.\n", TrackId, RhythmMode);
    #endif

        if (Transposition & 0x80)
        {
            Transposition = 0; //Ignore the transposition for rhythm channels.
        }
        else
        {
            Transposition = (Transposition & 0x40) ? (-0x80 + Transposition) : Transposition; // 7-bit -> 8-bit sign extension
            Transposition += _GlobalTransposition;
        }

        midiStream.SetTimestamp(OldTimestamp);
    }

    std::vector<uint32_t> BarOffsets;

    BarOffsets.reserve(256);

    buffer_t Text;

    uint8_t gsParams[6] = { }; // 0 device ID, 1 model ID, 2 address high, 3 address low
    uint8_t xgParams[6] = { }; // 0 device ID, 1 model ID, 2 address high, 3 address low

    uint32_t ParentOffs = 0;
    uint16_t RepeatingBarID = 0xFFFF;

    RunningNotes.Reset();

    midiStream.SetChannel(SrcChannel);

    uint16_t BarCount = 0;

    // Add "StartTick" offset to the initial timestamp.
    {
        uint32_t Timestamp = midiStream.GetTimestamp();

        if ((StartTick >= 0) || (-StartTick <= (int32_t) Timestamp))
        {
            Timestamp += StartTick;
            StartTick = 0;
        }
        else
        {
            StartTick += Timestamp;
            Timestamp = 0;
        }

        midiStream.SetTimestamp(Timestamp);
    }

    uint8_t Temp[256];

    BarOffsets.push_back(Offset);

    bool EndOfTrack = false;

    uint8_t LoopCount = 0;
    
    uint32_t LoopParentOffs[8] = { };
    uint32_t LoopStartOffs[8] = { };
    uint16_t LoopCounter[8] = { };

    while ((Offset < TrackTail) && !EndOfTrack)
    {
        uint32_t CmdOffset = Offset; // Offset of the start of the command

        uint8_t  CmdType     = 0;
        uint16_t CmdP0       = 0;
        uint8_t  CmdP1       = 0;
        uint8_t  CmdP2       = 0;
        uint16_t CmdDuration = 0;

        if (_Version == 2)
        {
            CmdType     = data[Offset + 0x00];
            CmdP0       = data[Offset + 0x01];
            CmdP1       = data[Offset + 0x02];
            CmdP2       = data[Offset + 0x03];
            CmdDuration = CmdP1;

            Offset += 4;
        }
        else
        if (_Version == 3)
        {
            CmdType     = data[Offset + 0x00];
            CmdP2       = data[Offset + 0x01];
            CmdP0       = ReadLE16(&data[Offset + 0x02]);
            CmdP1       = data[Offset + 0x04];
            CmdDuration = ReadLE16(&data[Offset + 0x04]);

            Offset += 6;
        }

        if (CmdType < 0x80)
        {
            uint32_t Timestamp = midiStream.GetTimestamp();

            RunningNotes.Check(midiStream, &Timestamp);

            midiStream.SetTimestamp(Timestamp);

            uint8_t Note = (CmdType + Transposition) & 0x7F;

            for (uint16_t i = 0; i < RunningNotes._Count; ++i)
            {
                if (RunningNotes._Notes[i].Note == Note)
                {
                    // The note is already playing. Remember its new length.
                    RunningNotes._Notes[i].Duration = midiStream.GetTimestamp() + CmdDuration;

                    CmdDuration = 0; // Prevens the note from being added.
                    break;
                }
            }

            // Duration == 0 means "no note".
            if ((CmdDuration > 0) && (DstChannel != 0xFF))
            {
                midiStream.WriteEvent(StatusCodes::NoteOn, Note, CmdP2);

                RunningNotes.Add(midiStream.GetChannel(), Note, 0x80, CmdDuration);
            }
        }
        else
        {
            switch (CmdType)
            {
                case 0x90: case 0x91: case 0x92: case 0x93: // send User SysEx (defined via header)
                case 0x94: case 0x95: case 0x96: case 0x97:
                {
                    if (DstChannel == 0xFF)
                        break;

                    {
                        const rcp_user_sysex_t * us = &_SysEx[CmdType & 0x07];

                        uint16_t Size = ProcessRCPSysEx(us->Data, us->Size, Temp, CmdP1, CmdP2, SrcChannel);

                        // Append 0xF7 byte (may be missing with UserSysEx of length 0x18)
                        if ((Size > 0) && Temp[Size - 1] != 0xF7)
                            Temp[Size++] = 0xF7;

                        if ((us->Name.Len > 0) && _Options._WriteSysExNames)
                            midiStream.WriteMetaEvent(MetaDataTypes::Text, us->Name.Data, us->Name.Len);

                        if (Size > 1)
                            midiStream.WriteEvent(StatusCodes::SysEx, Temp, Size);
                    #ifdef _DEBUG
                        else
                            ::printf("Warning: Track %2u, 0x%04X: Using empty User SysEx command %u.\n", TrackId, CmdOffset, CmdType & 0x07);
                    #endif
                    }
                    break;
                }

                case 0x98: // send SysEx
                {
                    uint16_t Size = GetMultiCmdDataSize(data, size, Offset, MCMD_INI_EXCLUDE | MCMD_RET_DATASIZE);

                    if (Text.Size < (uint32_t) Size)
                        Text.Grow((Size + 0x0F) & ~0x0F); // Round up to 0x10.

                    Size = ReadMultiCmdData(data, size, &Offset, Text.Data, Text.Size, MCMD_INI_EXCLUDE);

                    if (DstChannel == 0xFF)
                        break;

                    Size = ProcessRCPSysEx(Text.Data, Size, Text.Data, CmdP1, CmdP2, SrcChannel);

                    midiStream.WriteEvent(StatusCodes::SysEx, Text.Data, Size);
                    break;
                }

              //case 0x99: // execute external command
                case 0xC0: // DX7 Function
                case 0xC1: // DX Parameter
                case 0xC2: // DX RERF
                case 0xC3: // TX Function
                case 0xC5: // FB-01 P Parameter
                case 0xC7: // TX81Z V VCED
                case 0xC8: // TX81Z A ACED
                case 0xC9: // TX81Z P PCED
                case 0xCC: // DX7-2 R Remote SW
                case 0xCD: // DX7-2 A ACED
                case 0xCE: // DX7-2 P PCED
                case 0xCF: // TX802 P PCED
                {
                    if (DstChannel == 0xFF)
                        break;

                    static const uint8_t DX_PARAM[0x10] = { 0x08, 0x00, 0x04, 0x11, 0xFF, 0x15, 0xFF, 0x12, 0x13, 0x10, 0xFF, 0xFF, 0x1B, 0x18, 0x19, 0x1A, };

                    Temp[0] = 0x43; // Yamaha ID
                    Temp[1] = 0x10 | midiStream.GetChannel();
                    Temp[2] = DX_PARAM[CmdType & 0x0F];
                    Temp[3] = CmdP1;
                    Temp[4] = CmdP2;
                    Temp[5] = 0xF7;

                    midiStream.WriteEvent(StatusCodes::SysEx, Temp, 6);
                    break;
                }

                case 0xC6: // FB-01 S System
                {
                    if (DstChannel == 0xFF)
                        break;

                    Temp[0] = 0x43; // Yamaha ID
                    Temp[1] = 0x75;
                    Temp[2] = midiStream.GetChannel();
                    Temp[3] = 0x10;
                    Temp[4] = CmdP1;
                    Temp[5] = CmdP2;
                    Temp[6] = 0xF7;

                    midiStream.WriteEvent(StatusCodes::SysEx, Temp, 7);
                    break;
                }

                case 0xCA: // TX81Z S System
                case 0xCB: // TX81Z E EFFECT
                {
                    if (DstChannel == 0xFF)
                        break;

                    Temp[0] = 0x43; // Yamaha ID
                    Temp[1] = 0x10 | midiStream.GetChannel();
                    Temp[2] = 0x10;
                    Temp[3] = 0x7B + (CmdType - 0xCA); // command CA -> param = 7B, command CB -> param = 7C
                    Temp[4] = CmdP1;
                    Temp[5] = CmdP2;
                    Temp[6] = 0xF7;

                    midiStream.WriteEvent(StatusCodes::SysEx, Temp, 7);
                    break;
                }

                case 0xD0: // Yamaha Base Address
                {
                    xgParams[2] = CmdP1;
                    xgParams[3] = CmdP2;
                    break;
                }

                case 0xD1: // Yamaha Device Data
                {
                    xgParams[0] = CmdP1;
                    xgParams[1] = CmdP2;
                    break;
                }

                case 0xD2: // Yamaha Address / Parameter
                {
                    xgParams[4] = CmdP1;
                    xgParams[5] = CmdP2;

                    if (DstChannel == 0xFF)
                        break;

                    Temp[0] = 0x43; // Yamaha ID
                    ::memcpy(Temp + 1, &xgParams[0], 6);
                    Temp[7] = 0xF7;

                    midiStream.WriteEvent(StatusCodes::SysEx, Temp, 8);
                    break;
                }

                case 0xD3: // Yamaha XG Address / Parameter
                {
                    xgParams[4] = CmdP1;
                    xgParams[5] = CmdP2;

                    if (DstChannel == 0xFF)
                        break;

                    Temp[0] = 0x43; // Yamaha ID
                    Temp[1] = 0x10; // Parameter Change
                    Temp[2] = 0x4C; // XG
                    ::memcpy(Temp + 3, &xgParams[2], 4);
                    Temp[7] = 0xF7;

                    midiStream.WriteEvent(StatusCodes::SysEx, Temp, 8);
                    break;
                }

                case 0xDC: // MKS-7
                {
                    if (DstChannel == 0xFF)
                        break;

                    Temp[0] = 0x41; // Roland ID
                    Temp[1] = 0x32;
                    Temp[2] = midiStream.GetChannel();
                    Temp[3] = CmdP1;
                    Temp[4] = CmdP2;
                    Temp[5] = 0xF7;

                    midiStream.WriteEvent(StatusCodes::SysEx, Temp, 6);
                    break;
                }

                case 0xDD: // Roland Base Address
                {
                    gsParams[2] = CmdP1;
                    gsParams[3] = CmdP2;
                    break;
                }

                case 0xDE: // Roland Parameter
                {
                    gsParams[4] = CmdP1;
                    gsParams[5] = CmdP2;

                    if (DstChannel == 0xFF)
                        break;

                    Temp[0] = 0x41; // Roland ID
                    Temp[1] = gsParams[0];
                    Temp[2] = gsParams[1];
                    Temp[3] = 0x12;

                    uint8_t CheckSum = 0;

                    for (uint8_t i = 0; i < 4; ++i)
                    {
                        Temp[4 + i] = gsParams[2 + i];
                        CheckSum += gsParams[2 + i]; // add to checksum
                    }

                    Temp[8] = (0x100 - CheckSum) & 0x7F;
                    Temp[9] = 0xF7;

                    midiStream.WriteEvent(StatusCodes::SysEx, Temp, 10);
                    break;
                }

                case 0xDF: // Roland Device
                {
                    gsParams[0] = CmdP1;
                    gsParams[1] = CmdP2;
                    break;
                }

                case 0xE1: // Set XG instrument
                {
                    if (DstChannel == 0xFF)
                        break;

                    midiStream.WriteEvent(StatusCodes::ControlChange, 0x20, CmdP2);
                    midiStream.WriteEvent(StatusCodes::ProgramChange, CmdP1, 0x00);
                    break;
                }

                case 0xE2: // Set GS instrument
                {
                    if (DstChannel == 0xFF)
                        break;

                    midiStream.WriteEvent(StatusCodes::ControlChange, 0x00, CmdP2);
                    midiStream.WriteEvent(StatusCodes::ProgramChange, CmdP1, 0x00);
                    break;
                }

                case 0xE5: // Key Scan
                {
                    ::printf("Warning: Track %2u, 0x%04X: Key Scan command found.\n", TrackId, CmdOffset);
                    break;
                }

                case 0xE6: // MIDI channel
                {
                    CmdP1--; // It's same as in the track header, except 1 added.

                    if (CmdP1 & 0x80)
                    {
                        // Ignore the message when the KeepDummyChannels option is off. Else set midiDev to 0xFF to prevent events from being written.
                        if (!_Options._KeepDummyChannels)
                        {
                            DstChannel = 0xFF;
                            SrcChannel = 0x00;
                        }
                    }
                    else
                    {
                        DstChannel = CmdP1 >> 4; // port ID
                        SrcChannel = CmdP1 & 0x0F; // channel ID

                        midiStream.WriteMetaEvent(MetaDataTypes::MIDIPort,      &DstChannel, 1);
                        midiStream.WriteMetaEvent(MetaDataTypes::ChannelPrefix, &SrcChannel, 1);
                    }

                    midiStream.SetChannel(SrcChannel);
                    break;
                }

                case 0xE7: // Tempo Modifier
                {
                #ifdef _DEBUG
                    if (CmdP2 != 0)
                        ::printf("Warning: Track %2u, 0x%04X: Gradual Tempo Change. Speed 0x40, P2 = %u.\n", TrackId, CmdOffset, CmdP2);
                #endif

                    uint32_t Ticks = BPM2Ticks(_Tempo, CmdP1);

                    midiStream.WriteMetaEvent(MetaDataTypes::SetTempo, Ticks, 3u);
                    break;
                }

                case 0xEA: // Channel Pressure
                {
                    if (DstChannel == 0xFF)
                        break;

                    midiStream.WriteEvent(StatusCodes::ChannelPressure, CmdP1, 0);
                    break;
                }

                case 0xEB: // Control Change
                {
                    if (DstChannel == 0xFF)
                        break;

                    midiStream.WriteEvent(StatusCodes::ControlChange, CmdP1, CmdP2);
                    break;
                }

                case 0xEC: // Instrument
                {
                    if (DstChannel == 0xFF)
                        break;

                    if (CmdP1 < 0x80)
                    {
                        midiStream.WriteEvent(StatusCodes::ProgramChange, CmdP1, 0);
                    }
                    else
                    if (CmdP1 < 0xC0 && (SrcChannel >= 1 && SrcChannel < 9))
                    {
                        // Set MT-32 instrument from user bank used by RCP files from Granada X68000.
                        uint8_t partMemOfs = (SrcChannel - 1) << 4;

                        ::memcpy(Temp, MT32_PATCH_CHG, 0x07);

                        Temp[0x00] = (CmdP1 >> 6) & 0x03;
                        Temp[0x01] = (CmdP1 >> 0) & 0x3F;

                        midiStream.WriteRolandSysEx(MT32_SYX_HDR, 0x030000 | partMemOfs, Temp, 7, 0);
                    }
                    break;
                }

                case 0xED: // Note Aftertouch
                {
                    if (DstChannel == 0xFF)
                        break;

                    midiStream.WriteEvent(StatusCodes::KeyPressure, CmdP1, CmdP2);
                    break;
                }

                case 0xEE: // Pitch Bend
                {
                    if (DstChannel == 0xFF)
                        break;

                    midiStream.WriteEvent(StatusCodes::PitchBendChange, CmdP1, CmdP2);
                    break;
                }

                case 0xF5: // Key Signature Change
                {
                    RCP2MIDIKeySignature((uint8_t) CmdP0, Temp);
                    midiStream.WriteMetaEvent(MetaDataTypes::KeySignature, Temp, 2);

                    CmdP0 = 0;
                    break;
                }

                case 0xF6: // Comment
                {
                    uint16_t Size = GetMultiCmdDataSize(data, size, Offset, MCMD_INI_INCLUDE | MCMD_RET_DATASIZE);

                    if (Text.Size < Size)
                        Text.Grow((Size + 0x0F) & ~0x0F); // Round up to 0x10.

                    Size = ReadMultiCmdData(data, size, &Offset, Text.Data, Text.Size, MCMD_INI_INCLUDE);
                    Size = GetTrimmedLength((char *) Text.Data, Size, ' ', false);

                    midiStream.WriteMetaEvent(MetaDataTypes::Text, Text.Data, Size);

                    CmdP0 = 0;
                    break;
                }

                case 0xF7: // Continuation of previous command
                {
                #ifdef _DEBUG
                    ::printf("Warning: Track %2u, 0x%04X: Unexpected continuation command.\n", TrackId, CmdOffset);
                #endif
                    break;
                }

                case 0xF8: // Loop End
                {
                    if (LoopCount != 0)
                    {
                        LoopCount--;

                        LoopCounter[LoopCount]++;

                        bool TakeLoop = false;

                        // Loops == 0 -> infinite, but some songs also use very high values (like 0xFF) for that.
                        if (CmdP0 == 0 || CmdP0 >= 0x7F)
                        {
                            // Infinite loop
                            if (LoopCounter[LoopCount] < 0x80 && DstChannel != 0xFF)
                                midiStream.WriteEvent(StatusCodes::ControlChange, 0x6F, (uint8_t)LoopCounter[LoopCount]);

                            if (LoopCounter[LoopCount] < track->LoopCount)
                                TakeLoop = true;
                        }
                        else
                        {
                            if (LoopCounter[LoopCount] < CmdP0)
                                TakeLoop = true;
                        }

                        if (_Options._WriteBarMarkers)
                        {
                            uint32_t Length = ::sprintf((char*) Temp, "Loop %u End (%u/%u)", 1 + LoopCount, LoopCounter[LoopCount], CmdP0);

                            midiStream.WriteMetaEvent(MetaDataTypes::CueMarker, Temp, Length);
                        }

                        if (TakeLoop)
                        {
                            ParentOffs = LoopParentOffs[LoopCount];
                            Offset = LoopStartOffs[LoopCount];

                            LoopCount++;
                        }
                    }
                    else
                    {
                    #ifdef _DEBUG
                        ::printf("Warning: Track %2u, 0x%04X: Loop End without Loop Start.\n", TrackId, CmdOffset);
                    #endif

                        if (_Options._WriteBarMarkers)
                            midiStream.WriteMetaEvent(MetaDataTypes::CueMarker, "Bad Loop End");
                    }

                    CmdP0 = 0;
                    break;
                }

                case 0xF9: // Loop Begin
                {
                    if (_Options._WriteBarMarkers)
                    {
                        uint32_t Size = ::sprintf((char*) Temp, "Loop %u Start", LoopCount + 1);

                        midiStream.WriteMetaEvent(MetaDataTypes::CueMarker, Temp, Size);
                    }

                    if (LoopCount < 8)
                    {
                        if (Offset == track->LoopStartOffs && DstChannel != 0xFF)
                            midiStream.WriteEvent(StatusCodes::ControlChange, 0x6F, 0);

                        LoopParentOffs[LoopCount] = ParentOffs; // required by YS-2･018.RCP
                        LoopStartOffs[LoopCount] = Offset;
                        LoopCounter[LoopCount] = 0;

                        LoopCount++;
                    }
                #ifdef _DEBUG
                    else
                        ::printf("Error: Track %u, 0x%04X: Trying to do more than 8 nested loops.\n", TrackId, CmdOffset);
                #endif

                    CmdP0 = 0;
                    break;
                }

                case 0xFC: // Repeat previous bar
                {
                    // Behaviour of the FC command:
                    // - Already in "repeating bar" mode: return to parent bar (same as FD)
                    // - Else follow chain of FC commands: i.e. "FC -> FC -> FC -> non-FC command" is a valid sequence that is followed to the end.
                    if (ParentOffs == 0)
                    {
                        if (_Version == 2)
                            Offset -= 4;
                        else
                        if (_Version == 3)
                            Offset -= 6;

                        uint16_t BarID;

                        uint32_t RepeatOffs;
                        uint32_t CachedOffs;

                        do
                        {
                            if (_Version == 2)
                            {
                                CmdP0 = data[Offset + 0x01];
                                CmdP1 = data[Offset + 0x02];
                                CmdP2 = data[Offset + 0x03];

                                BarID = (CmdP0 << 0) | ((CmdP1 & 0x03) << 8);
                                RepeatOffs = ((CmdP1 & ~0x03) << 0) | (CmdP2 << 8);

                                Offset += 4;
                            }
                            else
                            if (_Version == 3)
                            {
                                CmdP0 = ReadLE16(&data[Offset + 0x02]);
                                CmdDuration = ReadLE16(&data[Offset + 0x04]);
                                BarID = CmdP0;

                                // Why has the first command ID 0x30?
                                RepeatOffs = 0x002E + (CmdDuration - 0x0030) * 0x06; // Calculate offset from command ID

                                Offset += 6;
                            }

                            if (_Options._WriteBarMarkers)
                            {
                                uint32_t Size = ::sprintf((char *) Temp, "Repeat Bar %u", 1 + BarID);

                                midiStream.WriteMetaEvent(MetaDataTypes::CueMarker, Temp, Size);
                            }

                            if (BarID >= BarOffsets.size())
                            {
                            #ifdef _DEBUG
                                ::printf("Warning: Track %2u, 0x%04X: Trying to repeat invalid bar %u. Max %u bars.\n", TrackId, CmdOffset, BarID, BarCount + 1);
                            #endif
                                break;
                            }

                            CachedOffs = BarOffsets[BarID] - TrackHead;

                        //  if (cachedPos != repeatPos)
                        //      printf("Warning: Track %2u: Repeat Measure %u: offset mismatch (file: 0x%04X != expected 0x%04X) at 0x%04X!\n", trkID, measureID, repeatPos, cachedPos, prevPos);

                            if (TrackHead + RepeatOffs == CmdOffset)
                                break; // prevent recursion (just for safety)

                            if (!ParentOffs) // necessary for following FC command chain
                                ParentOffs = Offset;

                            RepeatingBarID = BarID;

                            // YS3-25.RCP relies on using the actual offset. (*Some* of its bar numbers are off by 1.)
                            Offset = TrackHead + RepeatOffs;
                            CmdOffset = Offset;
                        }
                        while (data[Offset] == 0xFC);
                    }
                    else
                    {
                    #ifdef _DEBUG
                        ::printf("Warning: Track %2u, 0x%04X: Leaving recursive repeat bar.\n", TrackId, CmdOffset);
                    #endif
                        Offset = ParentOffs;

                        ParentOffs = 0x00;
                        RepeatingBarID = 0xFFFF;
                    }

                    CmdP0 = 0;
                    break;
                }

                case 0xFD: // Bar End
                {
                    if (BarOffsets.size() >= 0x8000) // Prevent infinite loops
                    {
                        EndOfTrack = true;
                        break;
                    }

                    if (ParentOffs)
                    {
                        Offset = ParentOffs;
                        ParentOffs = 0x00;
                        RepeatingBarID = 0xFFFF;
                    }

                    BarOffsets.push_back(Offset);
                    BarCount++;

                    CmdP0 = 0;

                    if (_Options._WriteBarMarkers)
                    {
                        uint32_t Size = ::sprintf((char *) Temp, "Bar %u", 1 + BarCount);

                        midiStream.WriteMetaEvent(MetaDataTypes::CueMarker, Temp, Size);
                    }

                    if (_Options._WolfteamLoopMode && (BarOffsets.size() == 2))
                    {
                        LoopCount = 0;

                        if (DstChannel != 0xFF)
                            midiStream.WriteEvent(StatusCodes::ControlChange, 0x6F, 0);

                        LoopParentOffs[LoopCount] = ParentOffs;
                        LoopStartOffs[LoopCount] = Offset;
                        LoopCounter[LoopCount] = 0;

                        LoopCount++;
                    }
                    break;
                }

                case 0xFE: // Track end
                {
                    EndOfTrack = true;
                    CmdP0 = 0;

                    if (_Options._WolfteamLoopMode)
                    {
                        LoopCount = 0;
                        LoopCounter[LoopCount]++;

                        if (LoopCounter[LoopCount] < 0x80 && DstChannel != 0xFF)
                            midiStream.WriteEvent(StatusCodes::ControlChange, 0x6F, (uint8_t) LoopCounter[LoopCount]);

                        if (LoopCounter[LoopCount] < _Options._RCPLoopCount)
                        {
                            ParentOffs = LoopParentOffs[LoopCount];
                            Offset = LoopStartOffs[LoopCount];
                            LoopCount++;
                            EndOfTrack = false;
                        }
                    }
                    break;
                }

                default:
                #ifdef _DEBUG
                    ::printf("Warning: Track %2u, 0x%04X: Unknown command 0x%02X.\n", TrackId, CmdOffset, CmdType);
                #endif
                    break;
            }
        }

        {
            uint32_t Timestamp = midiStream.GetTimestamp() + CmdP0;

            if ((StartTick < 0) && (Timestamp > 0))
            {
                StartTick += Timestamp;

                if (StartTick >= 0)
                {
                    Timestamp = StartTick;
                    StartTick = 0;
                }
                else
                    Timestamp = 0;
            }

            midiStream.SetTimestamp(Timestamp);
        }
    }

    if (DstChannel == 0xFF)
        midiStream.SetTimestamp(0);

    midiStream.SetTimestamp(RunningNotes.Flush(midiStream, false));

    *offset = TrackHead + TrackSize;
}

/// <summary>
/// 
/// </summary>
uint16_t rcp_file_t::GetMultiCmdDataSize(const uint8_t * data, uint32_t size, uint32_t offset, uint8_t flags) const
{
    uint16_t Size = (flags & MCMD_INI_INCLUDE) ? 1 : 0;

    if (_Version == 2)
    {
        for (uint32_t inPos = offset; inPos < size && data[inPos] == 0xF7; inPos += 0x04)
            Size++;

        if (flags & MCMD_RET_DATASIZE)
            Size *= 2; // 2 data bytes per command
    }
    else
    if (_Version == 3)
    {
        for (uint32_t inPos = offset; inPos < size && data[inPos] == 0xF7; inPos += 0x06)
            Size++;

        if (flags & MCMD_RET_DATASIZE)
            Size *= 5; // 5 data bytes per command
    }

    return Size;
}

/// <summary>
/// 
/// </summary>
uint16_t rcp_file_t:: ReadMultiCmdData(const uint8_t * srcData, uint32_t srcSize, uint32_t * srcOffset, uint8_t * dstData, uint32_t dstSize, uint8_t flags) const
{
    uint32_t SrcOffset = *srcOffset;
    uint32_t DstOffset = 0;

    if (_Version == 2)
    {
        if (flags & MCMD_INI_INCLUDE)
        {
            if (DstOffset + 0x02 > dstSize)
                return 0x00;

            dstData[DstOffset++] = srcData[SrcOffset - 0x02];
            dstData[DstOffset++] = srcData[SrcOffset - 0x01];
        }

        for (; SrcOffset < srcSize && srcData[SrcOffset] == 0xF7; SrcOffset += 0x04)
        {
            if (DstOffset + 0x02 > dstSize)
                break;

            dstData[DstOffset++] = srcData[SrcOffset + 0x02];
            dstData[DstOffset++] = srcData[SrcOffset + 0x03];
        }
    }
    else
    if (_Version == 3)
    {
        if (flags & MCMD_INI_INCLUDE)
        {
            if (DstOffset + 0x05 > dstSize)
                return 0x00;

            ::memcpy(&dstData[DstOffset], &srcData[SrcOffset - 0x05], 0x05);
            DstOffset += 0x05;
        }

        for (; SrcOffset < srcSize && srcData[SrcOffset] == 0xF7; SrcOffset += 0x06)
        {
            if (DstOffset + 0x05 > dstSize)
                break;

            ::memcpy(&dstData[DstOffset], &srcData[SrcOffset + 0x01], 0x05);
            DstOffset += 0x05;
        }
    }

    *srcOffset = SrcOffset;

    return (uint16_t) DstOffset;
}

/// <summary>
/// Converts the time signature from RCP to MIDI.
/// </summary>
void RCP2MIDITimeSignature(uint8_t numerator, uint8_t denominator, uint8_t data[4])
{
    uint8_t base2 = DetermineShift(denominator);

    data[0] = numerator;   // numerator
    data[1] = base2;       // log2(denominator)
    data[2] = 96 >> base2; // metronome pulse
    data[3] = 8;           // 32nd notes per 1/4 note
}

/// <summary>
/// Converts the key signature from RCP to MIDI.
/// </summary>
void RCP2MIDIKeySignature(uint8_t keySignature, uint8_t data[2])
{
    int8_t Key;

    if (keySignature & 0x08)
        Key = -(keySignature & 0x07); // flats
    else
        Key =   keySignature & 0x07; // sharps

    data[0] = (uint8_t) Key;  // main key (number of sharps/flats)
    data[1] = (keySignature & 0x10) >> 4; // major (0) / minor (1)
}

/// <summary>
/// 
/// </summary>
static uint8_t DetermineShift(uint32_t value)
{
    uint8_t shift = 0;

    value >>= 1;

    while (value)
    {
        shift++;
        value >>= 1;
    }

    return shift;
}

/// <summary>
/// 
/// </summary>
static uint16_t ProcessRCPSysEx(const uint8_t * srcData, uint16_t srcSize, uint8_t * dstData, uint8_t param1, uint8_t param2, uint8_t channel)
{
    uint8_t Checksum = 0;

    uint16_t j = 0;

    for (uint16_t i = 0; i < srcSize; ++i)
    {
        uint8_t Data = srcData[i];

        if (Data & 0x80)
        {
            switch (Data)
            {
                case 0x80: // put data value (cmdP1)
                    Data = param1;
                    break;

                case 0x81: // put data value (cmdP2)
                    Data = param2;
                    break;

                case 0x82: // put data value (midChn)
                    Data = channel;
                    break;

                case 0x83: // initialize Roland Checksum
                    Checksum = 0;
                    break;

                case 0x84: // put Roland Checksum
                    Data = (0x100 - Checksum) & 0x7F;
                    break;

                case 0xF7: // SysEx end
                    dstData[j++] = Data;

                    return j;

                default:
                    ::printf("Unknown SysEx command 0x%02X found in SysEx data!\n", Data);
                    break;
            }
        }

        if (!(Data & 0x80))
        {
            dstData[j++] = Data;

            Checksum += Data;
        }
    }

    return j;
}
