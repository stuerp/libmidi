
/** $VER: Stream.cpp (2024.05.15) P. Stuer **/

#include <CppCoreCheck/Warnings.h>

#pragma warning(disable: 4100 4625 4626 4710 4711 4738 5045 ALL_CPPCORECHECK_WARNINGS)

#include <SDKDDKVer.h>

#define NOMINMAX

#include <WinSock2.h>
#include <Windows.h>
#include <wincodec.h>

#include <MIDIProcessor.h>
#include <Encoding.h>

#include "Tables.h"
#include "SysEx.h"

uint32_t ProcessEvent(const midi_item_t & event, uint32_t timestamp, size_t index, const sysex_table_t & sysExMap);

/// <summary>
/// 
/// </summary>
void ProcessStream(const std::vector<midi_item_t> & stream, const sysex_table_t & sysExMap, bool skipNormalEvents)
{
    ::printf("%u messages, %u unique SysEx messages\n", (uint32_t) stream.size(), (uint32_t) sysExMap.Size());

    uint32_t Time = std::numeric_limits<uint32_t>::max();
    size_t i = 0;

    for (const auto & me : stream)
    {
        // Skip all normal MIDI events.
        if (skipNormalEvents && ((me.Data & 0x80000000u) == 0))
            continue;

        Time = ProcessEvent(me, Time, i++, sysExMap);
    }
}

/// <summary>
/// Processes MIDI stream events.
/// </summary>
uint32_t ProcessEvent(const midi_item_t & me, uint32_t timestamp, size_t index, const sysex_table_t & sysExMap)
{
    char Timestamp[16];
    char Time[16];

    if (me.Time != timestamp)
    {
        ::_snprintf_s(Timestamp, _countof(Timestamp), "%8u",  me.Time);
        ::_snprintf_s(Time, _countof(Time), "%8.2f", (double) me.Time / 1000.);
    }
    else
    {
        ::strncpy_s(Timestamp, "", _countof(Timestamp));
        ::strncpy_s(Time, "", _countof(Time));
    }

    ::printf("%8d %-8s %-8s ", (int) index, Timestamp, Time);

    // MIDI Event
    if ((me.Data & 0x80000000u) == 0)
    {
        uint8_t Event[3]
        {
            (uint8_t) (me.Data),
            (uint8_t) (me.Data >> 8),
            (uint8_t) (me.Data >> 16)
        };

        const uint8_t StatusCode = (const uint8_t) (Event[0] & 0xF0u);

        const uint32_t EventSize = (uint32_t) ((StatusCode >= StatusCodes::TimingClock && StatusCode <= StatusCodes::MetaData) ? 1 :
                                              ((StatusCode == StatusCodes::ProgramChange || StatusCode == StatusCodes::ChannelPressure) ? 2 : 3));

        uint8_t PortNumber = (me.Data >> 24) & 0x7F;

        ::printf("(%02X) %02X", PortNumber, Event[0]);

        if (EventSize > 1)
        {
            ::printf(" %02X", (int) Event[1]);

            if (EventSize > 2)
                ::printf(" %02X", (int) Event[2]);
            else
                ::printf("   ");
        }
        else
            ::printf("      ");

        int Channel = (Event[0] & 0x0F) + 1;

        switch (StatusCode)
        {
            case NoteOff:
                ::printf(" Channel %2d, Note Off, %3d, Velocity %3d\n", Channel, Event[1], Event[2]);
                break;

            case NoteOn:
                ::printf(" Channel %2d, Note On , %3d, Velocity %3d\n", Channel, Event[1], Event[2]);
                break;

            case KeyPressure:
                ::printf(" Channel %2d, Key Pressure %3d (Aftertouch)\n", Channel, Event[1]);
                break;

            case ControlChange:
                ::printf(" Channel %2d, Control Change %3d = %d (%s)\n", Channel, Event[1], Event[2], WideToUTF8(ControlChangeMessages[Event[1]].Name).c_str());
                break;

            case ProgramChange:
                ::printf(" Channel %2d, Program Change %3d, %s\n", Channel, Event[1] + 1, WideToUTF8(Instruments[Event[1]].Name).c_str());
                break;

            case ChannelPressure:
                ::printf(" Channel %2d, Channel Pressure %3d (Aftertouch)\n", Channel, Event[1]);
                break;

            case PitchBendChange:
                ::printf(" Channel %2d, Pitch Bend Change %02X:%02X\n", Channel, Event[1], Event[2]);
                break;

            case SysEx:
                ::printf(" Channel %2d, SysEx\n", Channel);
                break;

            case MIDITimeCodeQtrFrame:
                ::printf(" Channel %2d, MIDI Time Code Qtr Frame\n", Channel);
                break;

            case SongPositionPointer:
                ::printf(" Channel %2d, Song Position Pointer\n", Channel);
                break;

            case SongSelect:
                ::printf(" Channel %2d, Song Select\n", Channel);
                break;

            case TuneRequest:
                ::printf(" Channel %2d, Tune Request\n", Channel);
                break;

            case SysExEnd:
                ::printf(" Channel %2d, SysEx End\n", Channel);
                break;

            case TimingClock:
                ::printf(" Channel %2d, Timing Clock\n", Channel);
                break;

            case Start:
                ::printf(" Channel %2d, Start\n", Channel);
                break;

            case Continue:
                ::printf(" Channel %2d, Continue\n", Channel);
                break;

            case Stop:
                ::printf(" Channel %2d, Stop\n", Channel);
                break;

            case ActiveSensing:
                ::printf(" Channel %2d, Active Sensing\n", Channel);
                break;

            case MetaData:
                ::printf(" Channel %2d, Meta Data\n", Channel);
                break;

            default:
                ::printf("<Unknown Status Code: 0x%02X>\n", StatusCode);
        }
    }
    // SysEx Index
    else
    {
        const uint32_t Index = me.Data & 0xFFFFFFu;

        const uint8_t * MessageData;
        size_t MessageSize;
        uint8_t Port;

        sysExMap.GetItem(Index, MessageData, MessageSize, Port);

        // Show the message in the output.
        {
            ::printf("(%02X) SysEx", Port);

            for (size_t j = 0; j < MessageSize; ++j)
                ::printf(" %02X", MessageData[j]);
        }

        // Identify the SysEx message.
        if (MessageSize > 2)
        {
            sysex_t SysEx(MessageData, MessageSize);

            SysEx.Identify();

            ::printf(", \"%s\", \"%s\"\n", WideToUTF8(SysEx.GetManufacturer()).c_str(), WideToUTF8(SysEx.GetDescription()).c_str());
        }

        ::putchar('\n');
    }

    return me.Time;
}
