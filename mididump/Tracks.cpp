﻿
/** $VER: Tracks.cpp (2025.04.06) P. Stuer **/

#include <CppCoreCheck/Warnings.h>

#pragma warning(disable: 4100 4625 4626 4710 4711 4738 5045 ALL_CPPCORECHECK_WARNINGS)

#include <MIDIProcessor.h>
#include <Encoding.h>

#include "Tables.h"
#include "SysEx.h"

/// <summary>
/// Returns true of the input value is in the interval between min and max.
/// </summary>
template <class T>
inline static T InRange(T value, T minValue, T maxValue)
{
    return (minValue <= value) && (value <= maxValue);
}

/// <summary>
/// Processes a metadata message.
/// </summary>
static void ProcessMetaData(const midi::event_t & me) noexcept
{
    ::printf("Meta Data                    ");

    switch (me.Data[1])
    {
        case midi::MetaDataTypes::SequenceNumber:
        {
            ::printf(" Sequence Number");
            break;
        }

        case midi::MetaDataTypes::Text:
        {
            ::printf(" Text \"%s\"", (me.Data.size() > 2) ? TextToUTF8((const char *) me.Data.data() + 2, me.Data.size() - 2).c_str() : "");
            break;
        }

        case midi::MetaDataTypes::Copyright:
        {
            ::printf(" Copyright \"%s\"", (me.Data.size() > 2) ? TextToUTF8((const char *) me.Data.data() + 2, me.Data.size() - 2).c_str() : "");
            break;
        }

        case midi::MetaDataTypes::TrackName:
        {
            ::printf(" Track Name \"%s\"", (me.Data.size() > 2) ? TextToUTF8((const char *) me.Data.data() + 2, me.Data.size() - 2).c_str() : "");
            break;
        }

        case midi::MetaDataTypes::InstrumentName:
        {
            ::printf(" Instrument Name \"%s\"", (me.Data.size() > 2) ? TextToUTF8((const char *) me.Data.data() + 2, me.Data.size() - 2).c_str() : "");
            break;
        }

        case midi::MetaDataTypes::Lyrics:
        {
            ::printf(" Lyrics \"%s\"", (me.Data.size() > 2) ? TextToUTF8((const char *) me.Data.data() + 2, me.Data.size() - 2).c_str() : "");
            break;
        }

        case midi::MetaDataTypes::Marker:
        {
            ::printf(" Marker \"%s\"", (me.Data.size() > 2) ? TextToUTF8((const char *) me.Data.data() + 2, me.Data.size() - 2).c_str() : "");
            break;
        }

        case midi::MetaDataTypes::CueMarker:
        {
            ::printf(" Cue Marker \"%s\"", (me.Data.size() > 2) ? TextToUTF8((const char *) me.Data.data() + 2, me.Data.size() - 2).c_str() : "");
            break;
        }

        case midi::MetaDataTypes::DeviceName:
        {
            ::printf(" Device Name \"%s\"", (me.Data.size() > 2) ? TextToUTF8((const char *) me.Data.data() + 2, me.Data.size() - 2).c_str() : "");
            break;
        }

        case midi::MetaDataTypes::ChannelPrefix:
        {
            ::printf(" Channel Prefix");
            break;
        }

        case midi::MetaDataTypes::MIDIPort:
        {
            ::printf(" Set MIDI Port %d", me.Data[2]);
            break;
        }

        case midi::MetaDataTypes::EndOfTrack:
        {
            ::printf(" End of Track");
            break;
        }

        case midi::MetaDataTypes::SetTempo:
        {
            const uint32_t Tempo = ((uint32_t) me.Data[2] << 16) | ((uint32_t) me.Data[3] << 8) | (uint32_t) me.Data[4];

            ::printf(" Set Tempo (%d μs/quarter note, %d bpm)", Tempo, (int) ((60 * 1000 * 1000) / Tempo));
            break;
        }

        case midi::MetaDataTypes::SMPTEOffset:
        {
            ::printf(" Set SMPTE Offset");
            break;
        }

        case midi::MetaDataTypes::TimeSignature:
        {
            ::printf(" Time Signature %d/%d, %d ticks per beat, %d 32nd notes per MIDI quarter note", me.Data[2], 1 << me.Data[3], me.Data[4], me.Data[5]);
            break;
        }

        case midi::MetaDataTypes::KeySignature:
        {
            static const char * MajorScales[] =
            {
                "Cb", "Db", "Eb", "Fb", "Gb", "Ab", "Bb",   // 7 flats
                "Gb", "Ab", "Bb", "Cb", "Db", "Eb", "F",    // 6 flats
                "Db", "Eb", "F",  "Gb", "Ab", "Bb", "C",    // 5 flats
                "Ab", "Bb", "C",  "Db", "Eb", "F",  "G",    // 4 flats
                "Eb", "F",  "G",  "Ab", "Bb", "C",  "D",    // 3 flats
                "Bb", "C",  "D",  "Eb", "F",  "G",  "A",    // 2 flats
                "F",  "G",  "A",  "Bb", "C",  "D",  "E",    // 1 flat

                "C",  "D",  "E",  "F",  "G",  "A",  "B",    // No flats or sharps

                "G",  "A",  "B",  "C",  "D",  "E",  "F#",   // 1 sharp
                "D",  "E",  "F#", "G",  "A",  "B",  "C#",   // 2 sharps
                "A",  "B",  "C#", "D",  "E",  "F#", "G#",   // 3 sharps
                "E",  "F#", "G#", "A",  "B",  "C#", "D#",   // 4 sharps
                "B",  "C#", "D#", "E",  "F#", "G#", "A#",   // 5 sharps
                "F#", "G#", "A#", "B",  "C#", "D#", "E#",   // 6 sharps
                "C#", "D#", "E#", "F#", "G#", "A#", "B#",   // 7 sharps
            };

            static const char * MinorScales[] =
            {
                "Ab", "Bb", "Cb", "Db", "Eb", "Fb", "Gb",   // 7 flats
                "Eb", "F",  "Gb", "Ab", "Bb", "Cb", "Db",   // 6 flats
                "Bb", "C",  "Db", "Eb", "F",  "Gb", "Ab",   // 5 flats
                "F",  "G",  "Ab", "Bb", "C",  "Db", "Eb",   // 4 flats
                "C",  "D",  "Eb", "F",  "G",  "Ab", "Bb",   // 3 flats
                "G",  "A",  "Bb", "C",  "D",  "Eb", "F",    // 2 flats
                "D",  "E",  "F",  "G",  "A",  "Bb", "C",    // 1 flat

                "A",  "B",  "C",  "D",  "E",  "F",  "G",    // No flats or sharps

                "E",  "F#", "G",  "A",  "B",  "C",  "D",    // 1 sharp
                "B",  "C#", "D",  "E",  "F#", "G",  "A",    // 2 sharps
                "F#", "G#", "A",  "B",  "C#", "D",  "E",    // 3 sharps
                "C#", "D#", "E",  "F#", "G#", "A",  "B",    // 4 sharps
                "G#", "A#", "B",  "C#", "D#", "E",  "F#",   // 5 sharps
                "D#", "E#", "F#", "G#", "A#", "B",  "C#",   // 6 sharps
                "A#", "B#", "C#", "D#", "E#", "F#", "G#",   // 7 sharps
            };

            ::printf(" Key Signature %s %s", (me.Data[3] == 0) ? MajorScales[49 + (((char) me.Data[2]) * 7)] : MinorScales[49 + (((char) me.Data[2]) * 7)], (me.Data[3] == 0) ? "major" : "minor");
            break;
        }

        case midi::MetaDataTypes::SequencerSpecific:
        {
            ::printf(" Sequencer Specific");
            break;
        }

        default:
            break;
    }
}

/// <summary>
/// Processes a SysEx message.
/// </summary>
static void ProcessSysEx(const midi::event_t & me) noexcept
{
    sysex_t SysEx(me.Data);

    SysEx.Identify();

    ::printf(" \"%s\", \"%s\"", ::WideToUTF8(SysEx.GetManufacturer()).c_str(), ::WideToUTF8(SysEx.GetDescription()).c_str());
}

/// <summary>
/// Processes a Control Change message.
/// </summary>
static void ProcessControlChange(const midi::event_t & me) noexcept
{
    int Value = (int) me.Data[1];

    switch (me.Data[0])
    {
        case 0x00: ::printf(" Bank Select (MSB) %d", Value); break; // Allows user to switch bank for patch selection. Program change used with Bank Select. MIDI can access 16,384 patches per MIDI channel
        case 0x01: ::printf(" Modulation Depth (MSB) %d", Value); break; // Generally this CC controls a vibrato effect (pitch, loudness, brighness). What is modulated is based on the patch.
        case 0x02: ::printf(" Breath Controller (MSB) %d", Value); break; // Often times associated with aftertouch messages. It was originally intended for use with a breath MIDI controller in which blowing harder produced higher MIDI control values. It can be used for modulation as well.
        case 0x03: ::printf(" Undefined"); break;
        case 0x04: ::printf(" Foot Controller (MSB) %d", Value); break; // Often used with aftertouch messages. It can send a continuous stream of values based on how the pedal is used.
        case 0x05: ::printf(" Portamento Time (MSB) %d", Value); break; // Controls portamento rate to slide between 2 notes played subsequently.
        case 0x06: ::printf(" Data Entry (MSB) %d", Value); break; // Sets the Value for NRPN or RPN parameters.
        case 0x07: ::printf(" Channel Volume (MSB) %d", Value); break; // Control the volume of the channel
        case 0x08: ::printf(" Balance (MSB) %d", Value); break; // Controls the left and right balance, generally for stereo patches. 0 = hard left, 64 = center, 127 = hard right.
        case 0x09: ::printf(" Undefined"); break;
        case 0x0A: ::printf(" Pan (MSB) %d", Value); break; // Controls the left and right balance, generally for mono patches. 0 = hard left, 64 = center, 127 = hard right.
        case 0x0B: ::printf(" Expression (MSB) %d", Value); break; // Expression is a percentage of volume (CC7).
        case 0x0C: ::printf(" Effect Controller 1 (MSB) %d", Value); break; // Usually used to control a parameter of an effect within the synth/workstation.
        case 0x0D: ::printf(" Effect Controller 2 (MSB) %d", Value); break; // Usually used to control a parameter of an effect within the synth/workstation.

        case 0x0E: ::printf(" Undefined"); break;
        case 0x0F: ::printf(" Undefined"); break;

        case 0x10: ::printf(" General Purpose Controller 1 (MSB)"); break;
        case 0x11: ::printf(" General Purpose Controller 2 (MSB)"); break;
        case 0x12: ::printf(" General Purpose Controller 3 (MSB)"); break;
        case 0x13: ::printf(" General Purpose Controller 4 (MSB)"); break;

        case 0x20: ::printf(" Bank Select (LSB) %d", Value); break; // Allows user to switch bank for patch selection. Program change used with Bank Select. MIDI can access 16,384 patches per MIDI channel
        case 0x21: ::printf(" Modulation Depth (LSB) %d", Value); break; // Generally this CC controls a vibrato effect (pitch, loudness, brighness). What is modulated is based on the patch.
        case 0x22: ::printf(" Breath Controller (LSB) %d", Value); break; // Often times associated with aftertouch messages. It was originally intended for use with a breath MIDI controller in which blowing harder produced higher MIDI control values. It can be used for modulation as well.
        case 0x23: ::printf(" Undefined"); break;
        case 0x24: ::printf(" Foot Controller (LSB) %d", Value); break; // Often used with aftertouch messages. It can send a continuous stream of values based on how the pedal is used.
        case 0x25: ::printf(" Portamento Time (LSB) %d", Value); break; // Controls portamento rate to slide between 2 notes played subsequently.
        case 0x26: ::printf(" Data Entry (LSB) %d", Value); break; // Sets the Value for NRPN or RPN parameters.
        case 0x27: ::printf(" Channel Volume (LSB) %d", Value); break; // Control the volume of the channel
        case 0x28: ::printf(" Balance (LSB) %d", Value); break; // Controls the left and right balance, generally for stereo patches. 0 = hard left, 64 = center, 127 = hard right.
        case 0x29: ::printf(" Undefined"); break;
        case 0x2A: ::printf(" Pan (LSB) %d", Value); break; // Controls the left and right balance, generally for mono patches. 0 = hard left, 64 = center, 127 = hard right.
        case 0x2B: ::printf(" Expression (LSB) %d", Value); break; // Expression is a percentage of volume (CC7).
        case 0x2C: ::printf(" Effect Controller 1 (LSB) %d", Value); break; // Usually used to control a parameter of an effect within the synth/workstation.
        case 0x2D: ::printf(" Effect Controller 2 (LSB) %d", Value); break; // Usually used to control a parameter of an effect within the synth/workstation.

        case 0x2E: ::printf(" Undefined"); break;
        case 0x2F: ::printf(" Undefined"); break;

        case 0x30: ::printf(" General Purpose Controller 1 (LSB)"); break;
        case 0x31: ::printf(" General Purpose Controller 2 (LSB)"); break;
        case 0x32: ::printf(" General Purpose Controller 3 (LSB)"); break;
        case 0x33: ::printf(" General Purpose Controller 4 (LSB)"); break;

        case 0x40: ::printf(" Damper Pedal / Sustain Pedal %s", (Value < 64 ? "off" : "on")); break; // On/Off switch that controls sustain. (See also Sostenuto CC 66) 0 to 63 = Off, 64 to 127 = On
        case 0x41: ::printf(" Portamento %s", (Value < 64 ? "off" : "on")); break; // On/Off switch 0 to 63 = Off, 64 to 127 = On
        case 0x42: ::printf(" Sostenuto %s", (Value < 64 ? "off" : "on")); break; // On/Off switch. Like the Sustain controller (CC 64), However it only holds notes that were “On” when the pedal was pressed. People use it to “hold” chords” and play melodies over the held chord. 0 to 63 = Off, 64 to 127 = On
        case 0x43: ::printf(" Soft Pedal %s", (Value < 64 ? "off" : "on")); break; // Lowers the volume of notes played. 0 to 63 = Off, 64 to 127 = On
        case 0x44: ::printf(" Legato Footswitch: %s", (Value < 64 ? "Normal" : "Legato")); break; // Turns Legato effect between 2 subsequent notes On or Off. 0 to 63 = Off, 64 to 127 = On
        case 0x45: ::printf(" Hold 2 %s", (Value < 64 ? "off" : "on")); break; // Another way to “hold notes” (see MIDI CC 64 and MIDI CC 66). However notes fade out according to their release parameter rather than when the pedal is released.

        // GM2
        case 0x46: ::printf(" Sound Controller 1 (Sound Variation) %d", Value); break; // Usually controls the way a sound is produced. Default = Sound Variation.
        case 0x47: ::printf(" Sound Controller 2 (Timbre /Harmonic Intensity) %d", Value); break; // Allows shaping the Voltage Controlled Filter (VCF). Default = Resonance - also(Timbre or Harmonics)
        case 0x48: ::printf(" Sound Controller 3 (Release Time) %s", (Value < 64 ? "shorter" : "longer")); break; // Controls release time of the Voltage controlled Amplifier (VCA). Default = Release Time.
        case 0x49: ::printf(" Sound Controller 4 (Attack Time) %s", (Value < 64 ? "shorter" : "longer")); break; // Controls the “Attack’ of a sound. The attack is the amount of time it takes forthe sound to reach maximum amplitude.
        case 0x4A: ::printf(" Sound Controller 5 (Brightness) %s", (Value < 64 ? "lower" : "higher")); break; // Controls VCFs cutoff frequency of the filter.
        case 0x4B: ::printf(" Sound Controller 6 (Decay Time) %s", (Value < 64 ? "shorter" : "longer")); break; // Generic – Some manufacturers may use to further shave their sounds.
        case 0x4C: ::printf(" Sound Controller 7 (Vibrato Rate) %s", (Value < 64 ? "slower" : "faster")); break; // Generic – Some manufacturers may use to further shave their sounds.
        case 0x4D: ::printf(" Sound Controller 8 (Vibrato Depth) %s", (Value < 64 ? "reduced" : "increased")); break; // Generic – Some manufacturers may use to further shave their sounds.
        case 0x4E: ::printf(" Sound Controller 9 (Vibrato Delay) %s", (Value < 64 ? "shorter" : "longer")); break; // Generic – Some manufacturers may use to further shave their sounds.
        case 0x4F: ::printf(" Sound Controller 10 (Metronome Rate) %d", Value); break; // Generic – Some manufacturers may use to further shave their sounds.

        case 0x50: ::printf(" Generic On/Off switch: %s", (Value < 64 ? "off" : "one")); break;
        case 0x51: ::printf(" Generic On/Off switch: %s", (Value < 64 ? "off" : "one")); break;
        case 0x52: ::printf(" Generic On/Off switch: %s", (Value < 64 ? "off" : "one")); break;
        case 0x53: ::printf(" Generic On/Off switch: %s", (Value < 64 ? "off" : "one")); break;
        case 0x54: ::printf(" Portamento Amount %d", Value); break;

        case 0x58: ::printf(" High Resolution Velocity Prefix %d", Value); break; // Extends range of values, thereby creating a greater degree of precision.

        case 0x5B: ::printf(" Effect 1 Depth (Reverb) Send Level %d", Value); break;
        case 0x5C: ::printf(" Effect 2 Depth (Tremolo) Send Level %d", Value); break;
        case 0x5D: ::printf(" Effect 3 Depth (Chorus) Send Level %d", Value); break;
        case 0x5E: ::printf(" Effect 4 Depth (Detune) Send Level %d", Value); break;
        case 0x5F: ::printf(" Effect 5 Depth (Phaser) Send Level %d", Value); break;

        case 0x60: ::printf(" Data Increment"); break;
        case 0x61: ::printf(" Data Decrement"); break;

        case 0x62: ::printf(" Select NRPN parameter (LSB) %d", Value); break;
        case 0x63: ::printf(" Select NRPN parameter (MSB) %d", Value); break;

        case 0x64: ::printf(" Select RPN parameter (LSB) %d", Value); break;
        case 0x65: ::printf(" Select RPN parameter (MSB) %d", Value); break;

        case 0x6E: ::printf(" LeapFrog Start of Loop marker (%d %d)", (int) me.Data[0], Value); break;
        case 0x6F: ::printf(" LeapFrog End of Loop marker (%d %d)", (int) me.Data[0], Value); break;

        // Channel Mode Messages
        case 0x78: ::printf(" All Sound Off"); break; // Mutes all sounding notes. It does so regardless of release time or sustain. (See MIDI CC 123)
        case 0x79: ::printf(" Reset All Controllers"); break; // It will reset all controllers to their default.
        case 0x7A: ::printf(" Local Control"); break; // Turns internal connection of a MIDI keyboard/workstation, etc. On or Off. If you use a computer, you will most likely want local control off to avoid notes being played twice. Once locally and twice whent the note is sent back from the computer to your keyboard.
        case 0x7B: ::printf(" All Notes Off"); break; // Mutes all sounding notes. Release time will still be maintained, and notes held by sustain will not turn off until sustain pedal is depressed.
        case 0x7C: ::printf(" Omni Off"); break;
        case 0x7D: ::printf(" Omni On"); break;
        case 0x7E: ::printf(" Mono On (Poly Off)"); break; // Sets device mode to Monophonic. [Channel Mode Message] Mono Mode On (+ poly off, + all notes off). This equals the number of channels, or zero if the number of channels equals the number of voices in the receiver.
        case 0x7F: ::printf(" Poly On (Mono Off)"); break; // Sets device mode to Polyphonic. [Channel Mode Message] Poly Mode On (+ mono off, +all notes off).

        default:
            if (InRange((int) me.Data[0], 20, 31))
                ::printf(" Undefined");
            else
            if (InRange((int) me.Data[0], 85, 87))
                ::printf(" Undefined");
            else
            if (InRange((int) me.Data[0], 89, 90))
                ::printf(" Undefined");
            else
                ::printf(" Unknown CC %02X %02X (CC %d)", (int) me.Data[0], Value, (int) me.Data[0]); break;
    }
}

/// <summary>
/// Processes a Program Change message.
/// </summary>
static void ProcessProgramChange(const midi::event_t & me) noexcept
{
    int Value = (int) me.Data[0] + 1;

    for (const auto & it : Instruments)
    {
        if (it.Id == Value)
        {
            ::printf(" \"%s\"", ::WideToUTF8(it.Name).c_str());
            break;
        }
    }
}

/// <summary>
/// Processes MIDI events.
/// </summary>
static uint32_t ProcessEvent(const midi::event_t & event, uint32_t time, size_t index) noexcept
{
    char TimeInTicks[16];
    char TimeInMs[16];

    if (event.Time!= time)
    {
        ::_snprintf_s(TimeInTicks, _countof(TimeInTicks), "%8u ticks",  event.Time);
        ::_snprintf_s(TimeInMs, _countof(TimeInMs), "%8.2fs", (double) event.Time / 1000.);
    }
    else
        TimeInTicks[0] = TimeInMs[0] = '\0';

    if (event.Type != midi::event_t::event_type_t::Extended)
        ::printf("%8d %-14s %-10s (%2d) ", (int) index, TimeInTicks, TimeInMs, event.ChannelNumber + 1);
    else
        ::printf("%8d %-14s %-10s      ", (int) index, TimeInTicks, TimeInMs);

    if (event.Type != midi::event_t::event_type_t::Extended)
        ::printf(" %02X", (event.Type + 8) << 4);

    const int ByteCount = 16;

    int i = 1;

    for (const auto & d : event.Data)
    {
        if (i++ < ByteCount)
            ::printf(" %02X", d);
        else
        {
            ::printf(" ..");
            break;
        }
    }

    if (event.Type == midi::event_t::event_type_t::Extended)
        ::printf("   ");

    ::printf("%*.s", std::max(0, (ByteCount - (int) event.Data.size()) * 3), "");

    ::putchar(' ');

    switch (event.Type)
    {
        case midi::event_t::event_type_t::NoteOff:
        {
            ::printf("Note Off                      Note %3d, Velocity %3d", event.Data[0], event.Data[1]);
            break;
        }

        case midi::event_t::event_type_t::NoteOn:
        {
            ::printf("Note On                       Note %3d, Velocity %3d", event.Data[0], event.Data[1]);
            break;
        }

        case midi::event_t::event_type_t::KeyPressure:
        {
            ::printf("Key Pressure                  A0");
            break;
        }

        case midi::event_t::event_type_t::ControlChange:
        {
            ::printf("Control Change               "); ProcessControlChange(event); break;
        }

        case midi::event_t::event_type_t::ProgramChange:
        {
            ::printf("Program Change               "); ProcessProgramChange(event); break;
            break;
        }

        case midi::event_t::event_type_t::ChannelPressure:
        {
            ::printf("Channel Pressure              D0");
            break;
        }

        case midi::event_t::event_type_t::PitchBendChange:
        {
            ::printf("Pitch Bend Change             Value %d", 8192 - ((int) (event.Data[1] & 0x7F) << 7) | (event.Data[0] & 0x7F));
            break;
        }

        case midi::event_t::event_type_t::Extended:
        {
            switch (event.Data[0])
            {
                case midi::SysEx:                ::printf("SysEx                        "); ProcessSysEx(event); break;

                case midi::MIDITimeCodeQtrFrame: ::printf("MIDI Time Code Qtr Frame     "); break;
                case midi::SongPositionPointer:  ::printf("Song PositionPointer         "); break;
                case midi::SongSelect:           ::printf("Song Select                  "); break;

                case midi::TuneRequest:          ::printf("Tune Request                 "); break;
                case midi::SysExEnd:             ::printf("SysEx End                    "); break;
                case midi::TimingClock:          ::printf("Timing Clock                 "); break;

                case midi::Start:                ::printf("Start                        "); break;
                case midi::Continue:             ::printf("Continue                     "); break;
                case midi::Stop:                 ::printf("Stop                         "); break;

                case midi::ActiveSensing:        ::printf("Active Sensing               "); break;
                case midi::MetaData:             ProcessMetaData(event); break;

                default:                        ::printf("Unknown event type %02X      ", event.Data[0]); break;
            }

            break;
        }
    }

    ::putchar('\n');

    return event.Time;
}

/// <summary>
/// Processes all tracks.
/// </summary>
void ProcessTracks(const midi::container_t & container)
{
    const uint32_t SubsongIndex = 0;

    uint32_t TrackIndex = 0;

    for (const auto & Track : container)
    {
        uint32_t ChannelCount = container.GetChannelCount(SubsongIndex);
        uint32_t Duration = container.GetDuration(SubsongIndex, false);
        uint32_t DurationInMS = container.GetDuration(SubsongIndex, true);

        ::printf("\nTrack %2d: %d channels, %8d ticks, %8.2fs\n", TrackIndex + 1, ChannelCount, Duration, (float) DurationInMS / 1000.0f);
        ::puts("Index    - Ticks        - ms      - Ch - Data");

        uint32_t Time = std::numeric_limits<uint32_t>::max();
        size_t i = 0;

        for (const auto & Event : Track)
            Time = ProcessEvent(Event, Time, i++);

        ++TrackIndex;
    }
}
