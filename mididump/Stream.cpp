
/** $VER: Stream.cpp (2024.08.12) P. Stuer **/

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
/// Processes the stream.
/// </summary>
void ProcessStream(const std::vector<midi_item_t> & stream, const sysex_table_t & sysExMap, const std::vector<uint8_t> & portNumbers, bool skipNormalEvents)
{
    ::printf("%u messages, %u unique SysEx messages, %u ports\n", (uint32_t) stream.size(), (uint32_t) sysExMap.Size(), (uint32_t) portNumbers.size());

    uint32_t Time = std::numeric_limits<uint32_t>::max();
    size_t i = 0;

    for (const auto & mi : stream)
    {
        // Skip all normal MIDI events.
        if (skipNormalEvents && !mi.IsSysEx())
            continue;

        Time = ProcessEvent(mi, Time, i++, sysExMap);
    }
}

/// <summary>
/// Processes MIDI stream events.
/// </summary>
uint32_t ProcessEvent(const midi_item_t & item, uint32_t timestamp, size_t index, const sysex_table_t & sysExMap)
{
    char Timestamp[16];
    char Time[16];

    if (item.Time != timestamp)
    {
        ::_snprintf_s(Timestamp, _countof(Timestamp), "%8u",  item.Time);
        ::_snprintf_s(Time, _countof(Time), "%8.2f", (double) item.Time / 1000.);
    }
    else
    {
        ::strncpy_s(Timestamp, "", _countof(Timestamp));
        ::strncpy_s(Time, "", _countof(Time));
    }

    ::printf("%8d %-8s %-8s ", (int) index, Timestamp, Time);

    // MIDI Event
    if (!item.IsSysEx())
    {
        uint8_t Event[3]
        {
            (uint8_t) (item.Data),
            (uint8_t) (item.Data >> 8),
            (uint8_t) (item.Data >> 16)
        };

        const uint8_t StatusCode = (const uint8_t) (Event[0] & 0xF0u);

        const uint32_t EventSize = (uint32_t) ((StatusCode >= StatusCodes::TimingClock && StatusCode <= StatusCodes::MetaData) ? 1 :
                                              ((StatusCode == StatusCodes::ProgramChange || StatusCode == StatusCodes::ChannelPressure) ? 2 : 3));

        uint8_t PortNumber = (item.Data >> 24) & 0x7F;

        ::printf("%02X", Event[0]);

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

        ::printf(" Port %d, Channel %2d, ", PortNumber, Channel);

        switch (StatusCode)
        {
            case NoteOff:
                ::printf("Note Off, %3d, Velocity %3d\n", Event[1], Event[2]);
                break;

            case NoteOn:
                ::printf("Note On , %3d, Velocity %3d\n", Event[1], Event[2]);
                break;

            case KeyPressure:
                ::printf("Key Pressure %3d (Aftertouch)\n", Event[1]);
                break;

            case ControlChange:
                ::printf("Control Change %3d = %d (%s)\n", Event[1], Event[2], WideToUTF8(ControlChangeMessages[Event[1]].Name).c_str());
                break;

            case ProgramChange:
                ::printf("Program Change %3d, %s\n", Event[1] + 1, WideToUTF8(Instruments[Event[1]].Name).c_str());
                break;

            case ChannelPressure:
                ::printf("Channel Pressure %3d (Aftertouch)\n", Event[1]);
                break;

            case PitchBendChange:
                ::printf("Pitch Bend Change %02X:%02X\n", Event[1], Event[2]);
                break;

            case SysEx:
                ::puts("SysEx");
                break;

            case MIDITimeCodeQtrFrame:
                ::puts("MIDI Time Code Qtr Frame");
                break;

            case SongPositionPointer:
                ::puts("Song Position Pointer");
                break;

            case SongSelect:
                ::puts("Song Select");
                break;

            case TuneRequest:
                ::puts("Tune Request");
                break;

            case SysExEnd:
                ::puts("SysEx End");
                break;

            case TimingClock:
                ::puts("Timing Clock");
                break;

            case Start:
                ::puts("Start");
                break;

            case Continue:
                ::puts("Continue");
                break;

            case Stop:
                ::puts("Stop");
                break;

            case ActiveSensing:
                ::puts("Active Sensing");
                break;

            case MetaData:
                ::puts("Meta Data");
                break;

            default:
                ::printf("<Unknown Status Code: 0x%02X>\n", StatusCode);
        }
    }
    // SysEx Index
    else
    {
        const uint32_t Index = item.Data & 0xFFFFFFu;

        const uint8_t * MessageData;
        size_t MessageSize;
        uint8_t Port;

        sysExMap.GetItem(Index, MessageData, MessageSize, Port);

        // Show the message in the output.
        {
            ::printf("SysEx");

            for (size_t j = 0; j < MessageSize; ++j)
                ::printf(" %02X", MessageData[j]);
        }

        ::printf(" Port %d", Port);

        // Identify the SysEx message.
        if (MessageSize > 2)
        {
            sysex_t SysEx(MessageData, MessageSize);

            SysEx.Identify();

            ::printf(", \"%s\", \"%s\"", WideToUTF8(SysEx.GetManufacturer()).c_str(), WideToUTF8(SysEx.GetDescription()).c_str());
        }

        ::putchar('\n');
    }

    return item.Time;
}
