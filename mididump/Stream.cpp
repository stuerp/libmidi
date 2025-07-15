
/** $VER: Stream.cpp (2025.03.19) P. Stuer **/

#include <CppCoreCheck/Warnings.h>

#pragma warning(disable: 4100 4625 4626 4710 4711 4738 5045 ALL_CPPCORECHECK_WARNINGS)

#include <MIDIProcessor.h>
#include <Encoding.h>

#include "Tables.h"
#include "SysEx.h"

static uint8_t CCMSB = 255; // Control Change Most Significant Byte
static uint8_t CCLSB = 255; // Control Change Least Significant Byte

static uint8_t DELSB = 0;

/// <summary>
/// Processes MIDI stream events.
/// </summary>
uint32_t ProcessEvent(const midi::message_t & item, uint32_t timestamp, size_t index, const midi::sysex_table_t & sysExMap)
{
    char Timestamp[16];
    char Time[16];

    if (item.Time != timestamp)
    {
        ::_snprintf_s(Timestamp, _countof(Timestamp), "%8u ticks",  item.Time);
        ::_snprintf_s(Time, _countof(Time), "%8.2f s", (double) item.Time / 1000.);
    }
    else
    {
        ::strncpy_s(Timestamp, "", _countof(Timestamp));
        ::strncpy_s(Time, "", _countof(Time));
    }

    ::printf("%8d %-14s %-10s ", (int) index, Timestamp, Time);

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

        const uint32_t EventSize = (uint32_t) ((StatusCode >= midi::TimingClock   && StatusCode <= midi::MetaData) ? 1 :
                                              ((StatusCode == midi::ProgramChange || StatusCode == midi::ChannelPressure) ? 2 : 3));

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
            case midi::NoteOff:
                ::printf("Note Off, %3d, Velocity %3d\n", Event[1], Event[2]);
                break;

            case midi::NoteOn:
                ::printf("Note On , %3d, Velocity %3d\n", Event[1], Event[2]);
                break;

            case midi::KeyPressure:
                ::printf("Key Pressure %3d (Aftertouch)\n", Event[1]);
                break;

            case midi::ControlChange:
            {
                ::printf("Control Change %3d = %3d (%s)\n", Event[1], Event[2], ::WideToUTF8(ControlChangeMessages[Event[1]].Name).c_str());

                if (Event[1] == 98)     // Non-Registered Parameter LSB
                    CCLSB = Event[2];
                else
                if (Event[1] == 99)     // Non-Registered Parameter MSB
                    CCMSB = Event[2];
                else
                if (Event[1] == 100)    // Registered Parameter LSB
                    CCLSB = Event[2];
                else
                if (Event[1] == 101)    // Registered Parameter MSB
                    CCMSB = Event[2];
                else
                if (Event[1] == 38)     // LSB for CC 0 - 31.
                    DELSB = Event[2];

                if ((CCLSB != 255) && (CCMSB != 255))
                {
                    int RPN = (CCMSB << 7) | CCLSB; // Registered Parameter Number

                    if ((Event[1] == 6) || (Event[1] == 96) || (Event[1] == 97))
                    {
                        int Value = (Event[2] << 7) | DELSB;

                        if (Event[1] == 6)      // Data Entry
                            ::printf("Set RPN 0x%04X to %3d\n", RPN, Value);
                        else
                        if (Event[1] == 96)
                            ::printf("Increment RPN 0x%04X by %3d\n", RPN, Value);
                        else
                        if (Event[1] == 97)
                            ::printf("Decrement RPN 0x%04X by %3d\n", RPN, Value);

                        CCMSB = CCLSB = 255;
                        DELSB = 0;
                    }
                }
                break;
            }

            case midi::ProgramChange:
                ::printf("Program Change %3d, %s\n", Event[1] + 1, ::WideToUTF8(Instruments[Event[1]].Name).c_str());
                break;

            case midi::ChannelPressure:
                ::printf("Channel Pressure %3d (Aftertouch)\n", Event[1]);
                break;

            case midi::PitchBendChange:
                ::printf("Pitch Bend Change %02X:%02X\n", Event[1], Event[2]);
                break;

            case midi::SysEx:
                ::puts("SysEx");
                break;

            case midi::MIDITimeCodeQtrFrame:
                ::puts("MIDI Time Code Qtr Frame");
                break;

            case midi::SongPositionPointer:
                ::puts("Song Position Pointer");
                break;

            case midi::SongSelect:
                ::puts("Song Select");
                break;

            case midi::TuneRequest:
                ::puts("Tune Request");
                break;

            case midi::SysExEnd:
                ::puts("SysEx End");
                break;

            case midi::TimingClock:
                ::puts("Timing Clock");
                break;

            case midi::Start:
                ::puts("Start");
                break;

            case midi::Continue:
                ::puts("Continue");
                break;

            case midi::Stop:
                ::puts("Stop");
                break;

            case midi::ActiveSensing:
                ::puts("Active Sensing");
                break;

            case midi::MetaData:
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

            ::printf(", \"%s\", \"%s\", \"%s\", \"%s\"", SysEx.Manufacturer.c_str(), SysEx.Model.c_str(), SysEx.Command.c_str(), SysEx.Description.c_str());
        }

        ::putchar('\n');
    }

    return item.Time;
}

/// <summary>
/// Processes the stream.
/// </summary>
void ProcessStream(const std::vector<midi::message_t> & stream, const midi::sysex_table_t & sysExMap, const std::vector<uint8_t> & portNumbers, bool skipNormalEvents)
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
