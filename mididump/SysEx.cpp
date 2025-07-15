
/** $VER: SysEx.cpp (2025.07.13) P. Stuer **/

#include <CppCoreCheck/Warnings.h>

#pragma warning(disable: 4100 4625 4626 4710 4711 4738 5045 ALL_CPPCORECHECK_WARNINGS)

#include "Encoding.h"
#include "Tables.h"
#include "SysEx.h"

static const char * IdentifyGSReverbMacro(uint8_t value) noexcept
{
    switch (value)
    {
        case 0x00: return "Room 1"; break;
        case 0x01: return "Room 2"; break;
        case 0x02: return "Room 3"; break;
        case 0x03: return "Hall 1"; break;
        case 0x04: return "Hall 2"; break;
        case 0x05: return "Plate"; break;
        case 0x06: return "Delay"; break;
        case 0x07: return "Panning Delay"; break;

        default: return "<Unknown>";
    }
}

static const char * IdentifyGSChorusMacro(uint8_t value) noexcept
{
    switch (value)
    {
        case 0x00: return "Chorus 1"; break;
        case 0x01: return "Chorus 2"; break;
        case 0x02: return "Chorus 3"; break;
        case 0x03: return "Chorus 4"; break;
        case 0x04: return "Feedback Chorus"; break;
        case 0x05: return "Flanger"; break;
        case 0x06: return "Short Delay"; break;
        case 0x07: return "Short Delay (FB)"; break;

        default: return "<Unknown>";
    }
}

static const char * IdentifyGSDelayMacro(uint8_t value) noexcept
{
    switch (value)
    {
        case 0x00: return "Delay 1"; break;
        case 0x01: return "Delay 2"; break;
        case 0x02: return "Delay 3"; break;
        case 0x03: return "Delay 4"; break;
        case 0x04: return "Pan Delay 1"; break;
        case 0x05: return "Pan Delay 2"; break;
        case 0x06: return "Pan Delay 3"; break;
        case 0x07: return "Pan Delay 4"; break;
        case 0x08: return "Delay to Reverb"; break;
        case 0x09: return "Pan Repeat"; break;

        default: return "<Unknown>";
    }
}

/// <summary>
/// Converts a block number to a part number.
/// </summary>
static int GSBlockToPart(int value) noexcept
{
    if (value < 10)
        return value;

    if (value == 0)
        return 10;

    return value + 1;
}

static const wchar_t * IdentifyGSRhythmPart(uint8_t value) noexcept
{
    switch (value)
    {
        case 0x00: return L"Off"; break;
        case 0x01: return L"Map 1"; break;
        case 0x02: return L"Map 2"; break;

        default: return L"<Unknown>";
    }
}

static const wchar_t * IdentifyGSToneMap(uint8_t value) noexcept
{
    switch (value)
    {
        case 0x00: return L"Selected"; break;
        case 0x01: return L"SC-55 Map"; break;
        case 0x02: return L"SC-88 Map"; break;
        case 0x03: return L"SC-88Pro Map"; break;
        case 0x04: return L"SC-8820 Map"; break;

        default: return L"<Unknown>";
    }
}

/// <summary>
/// 
/// </summary>
void sysex_t::IdentifyGSMessage()
{
    const uint32_t Address = (uint32_t) (_Data[5] << 16) | (_Data[6] << 8) | _Data[7];
#ifdef later
    switch (Address)
    {
        // System parameters
        case 0x00007F: _Description = "GS System Mode Set"; break;

        // Patch parameters (Patch Common)
        case 0x400000: _Description += ::FormatText(L"GS Master Tune %d", (int) Iter[3]); break;                                                   // MASTER TUNE (-100.0 .. +100.0 cents)
        case 0x400004: _Description += ::FormatText(L"GS Master Volume %d", (int) Iter[3]); break;                                                 // MASTER VOLUME (0 .. 127)
        case 0x400006: _Description += ::FormatText(L"GS Master Key Shift %d semitones", (int) Iter[3]); break;                                    // MASTER KEY-SHIFT (-24 .. +24 semitones)
        case 0x400005: _Description += ::FormatText(L"GS Master Pan", (int) Iter[3]); break;                                                       // MASTER PAN (-63 .. +63) (Left to right)

        case 0x40007F: _Description += "GS Reset"; break;                                                                                          // MODE SET (0 = GS Reset)

        case 0x400100: _Description += "GS Patch Name"; break;                                                                                     // PATCh NAME (16 ASCII characters)

        case 0x400110: _Description += "GS Reserved"; break;

        case 0x400130: _Description += ::FormatText(L"GS Reverb Macro %s", IdentifyGSReverbMacro(Iter[3])); break;                                // REVERB MACRO (0 .. 7)
        case 0x400131: _Description += ::FormatText(L"GS Reberb Character %s", IdentifyGSReverbMacro(Iter[3])); break;                            // REVERB CHARACTER (0 .. 7)
        case 0x400132: _Description += ::FormatText(L"GS Reverb Pre-LPF %d", (int) Iter[3]); break;                                               // REVERB PRE-LPF (0 .. 7)
        case 0x400133: _Description += ::FormatText(L"GS Reverb Level %d", (int) Iter[3]); break;                                                 // REVERB LEVEL (0 .. 127)
        case 0x400134: _Description += ::FormatText(L"GS Reverb Time %d", (int) Iter[3]); break;                                                  // REVERB TIME (0 .. 127)
        case 0x400135: _Description += ::FormatText(L"GS Reverb Delay Feedback %d", (int) Iter[3]); break;                                        // REVERB DELAY FEEDBACK (0 .. 127)

        case 0x400137: _Description += ::FormatText(L"GS Reverb Predelay Time %d", (int) Iter[3]); break;                                         // REVERB PREdelay TIME (0 .. 127 ms)

        case 0x400138: _Description += ::FormatText(L"GS Chorus Macro %s", IdentifyGSChorusMacro(Iter[3])); break;
        case 0x400139: _Description += ::FormatText(L"GS Chorus Pre-LPF %d", (int) Iter[3]); break; // 0 .. 7
        case 0x40013A: _Description += ::FormatText(L"GS Chorus Level %d", (int) Iter[3]); break; // 0 .. 127
        case 0x40013B: _Description += ::FormatText(L"GS Chorus Feedback %d", (int) Iter[3]); break; // 0 .. 127
        case 0x40013C: _Description += ::FormatText(L"GS Chorus Delay %d", (int) Iter[3]); break; // 0 .. 127
        case 0x40013D: _Description += ::FormatText(L"GS Chorus Rate %d", (int) Iter[3]); break; // 0 .. 127
        case 0x40013E: _Description += ::FormatText(L"GS Chorus Depth %d", (int) Iter[3]); break; // 0 .. 127
        case 0x40013F: _Description += ::FormatText(L"GS Chorus Send Level to Reverb %d", (int) Iter[3]); break; // 0 .. 127
        case 0x400140: _Description += ::FormatText(L"GS Chorus Send Level to Delay %d", (int) Iter[3]); break; // 0 .. 127

        case 0x400150: _Description += ::FormatText(L"GS Delay Macro %s", IdentifyGSDelayMacro(Iter[3])); break;
        case 0x400151: _Description += ::FormatText(L"GS Delay Pre-LPF %d", (int) Iter[3]); break; // 0 .. 7
        case 0x400152: _Description += ::FormatText(L"GS Delay Time Center %d", (int) Iter[3]); break; // 0.1ms .. 1s
        case 0x400153: _Description += ::FormatText(L"GS Delay Time Ratio Left %d", (int) Iter[3]); break; // 4% .. 500%
        case 0x400154: _Description += ::FormatText(L"GS Delay Time Ration Right %d", (int) Iter[3]); break; // 4% .. 500%
        case 0x400155: _Description += ::FormatText(L"GS Delay Level Center %d", (int) Iter[3]); break; // 0 .. 127
        case 0x400156: _Description += ::FormatText(L"GS Delay Level Left %d", (int) Iter[3]); break; // 0 .. 127
        case 0x400157: _Description += ::FormatText(L"GS Delay Level Right %d", (int) Iter[3]); break; // 0 .. 127
        case 0x400158: _Description += ::FormatText(L"GS Delay Level %d", (int) Iter[3]); break; // 0 .. 127
        case 0x400159: _Description += ::FormatText(L"GS Delay Feedback %d", (int) Iter[3]); break; // -64 .. +63
        case 0x40015A: _Description += ::FormatText(L"GS Delay Send Level to Reverb: %d", (int) Iter[3]); break; // 0 .. 127

        case 0x400200: _Description += ::FormatText(L"GS Eq Low Frequency %d", (int) Iter[3]); break; // 200Hz, 400Hz
        case 0x400201: _Description += ::FormatText(L"GS Eq Low Gain %d", (int) Iter[3]); break; // -12dB .. +12dB
        case 0x400202: _Description += ::FormatText(L"GS Eq High Frequency %d", (int) Iter[3]); break; // 3kHz, 6kHz
        case 0x400203: _Description += ::FormatText(L"GS Eq High Gain %d", (int) Iter[3]); break; // -12dB .. +12dB

        case 0x400300: _Description += ::FormatText(L"GS EFX Type"); break; // SC-88Pro

        case 0x400303: case 0x400304: case 0x400305: case 0x400306: case 0x400307: case 0x400308: case 0x400309: case 0x40030A: case 0x40030B: case 0x40030C:
        case 0x40030D: case 0x40030E: case 0x40030F: case 0x400310: case 0x400311: case 0x400312: case 0x400313: case 0x400314: case 0x400315: case 0x400316:
                       _Description += ::FormatText(L"GS EFX Parameter %d %d", (int) _Data[7] - 2, (int) Iter[3]); break; // SC-88Pro

        case 0x400317: _Description += ::FormatText(L"GS EFX Send Level to Reverb: %d", (int) Iter[3]); break; // 0 .. 127
        case 0x400318: _Description += ::FormatText(L"GS EFX Send Level to Chorus: %d", (int) Iter[3]); break; // 0 .. 127
        case 0x400319: _Description += ::FormatText(L"GS EFX Send Level to Delay: %d", (int) Iter[3]); break; // 0 .. 127

        // Patch Part Parameters. The Sound Canvas VA has 16 Parts. Parameters that can be set individually for each Part are called Patch Part parameters.
        default:
        {
            int PartNumber = GSBlockToPart(_Data[6] & 0x0F);

            switch (Address & 0xFFF0FF)
            {
                case 0x401014: _Description += ::FormatText(L"GS Assign Mode %d to channel %d", (int) Iter[3], PartNumber); break;                            // ASSIGN MODE
                case 0x401015: _Description += ::FormatText(L"GS Use channel %d for Rhythm Part: %s", PartNumber, IdentifyGSRhythmPart(Iter[3])); break;      // USE FOR RHYTHM PART

                case 0x401030: _Description += ::FormatText(L"GS Tone Modify 1, channel %d: %d", PartNumber, (int) Iter[3]); break;                           // TONE MODIFY1 Vibrato Rate
                case 0x401031: _Description += ::FormatText(L"GS Tone Modify 2, channel %d: %d", PartNumber, (int) Iter[3]); break;                           // TONE MODIFY2 Vibrato Depth
                case 0x401032: _Description += ::FormatText(L"GS Tone Modify 3, channel %d: %d", PartNumber, (int) Iter[3]); break;                           // TONE MODIFY3 TVF Cutoff Frequency
                case 0x401033: _Description += ::FormatText(L"GS Tone Modify 4, channel %d: %d", PartNumber, (int) Iter[3]); break;                           // TONE MODIFY4 TVF Resonance
                case 0x401034: _Description += ::FormatText(L"GS Tone Modify 5, channel %d: %d", PartNumber, (int) Iter[3]); break;                           // TONE MODIFY5 TVF & TVA Envelope Attack
                case 0x401035: _Description += ::FormatText(L"GS Tone Modify 6, channel %d: %d", PartNumber, (int) Iter[3]); break;                           // TONE MODIFY6 TVF & TVA Envelope Decay
                case 0x401036: _Description += ::FormatText(L"GS Tone Modify 7, channel %d: %d", PartNumber, (int) Iter[3]); break;                           // TONE MODIFY7 TVF & TVA Envelope Release
                case 0x401037: _Description += ::FormatText(L"GS Tone Modify 8, channel %d: %d", PartNumber, (int) Iter[3]); break;                           // TONE MODIFY8 Vibrato Delay

                case 0x404000: _Description += ::FormatText(L"GS Tone Map Number: channel %d, %s", PartNumber, IdentifyGSToneMap(Iter[3])); break;            // TONE MAP NUMBER
                case 0x404001: _Description += ::FormatText(L"GS Tone Map-0 Number: channel %d, %s", PartNumber, IdentifyGSToneMap(Iter[3])); break;          // TONE MAP-0 NUMBER

                case 0x402010: _Description += ::FormatText(L"GS Bend Pitch Control: Channel %d, %d semitones", GSBlockToPart(_Data[6] & 0x0F), (int) Iter[3]); break;

                // Drum setup parameters
                default:
                {
                    int MapNumber = ((_Data[6] & 0xF0) >> 4) + 1;
                    int NoteNumber = (int) _Data[7];

                    switch (Address & 0xFF0F00)
                    {
                        case 0x410000: _Description += ::FormatText(L"GS Drum Map Name (Map %d)", MapNumber); break; // ASCII characters

                        case 0x410100: _Description += ::FormatText(L"GS Drum Play Note Number (Map %d, Note %d)", MapNumber, NoteNumber); break;
                        case 0x410200: _Description += ::FormatText(L"GS Drum Level (Map %d, Note %d)", MapNumber, NoteNumber); break;
                        case 0x410300: _Description += ::FormatText(L"GS Drum Assign Group Number (Map %d, Note %d)", MapNumber, NoteNumber); break;
                        case 0x410400: _Description += ::FormatText(L"GS Drum PanPot (Map %d, Note %d)", MapNumber, NoteNumber); break;
                        case 0x410500: _Description += ::FormatText(L"GS Drum Reverb Send Level (Map %d, Note %d)", MapNumber, NoteNumber); break;
                        case 0x410600: _Description += ::FormatText(L"GS Drum Chorus Send Level (Map %d, Note %d)", MapNumber, NoteNumber); break;

                        default: _Description = "GS Unknown address";
                    }
                }
            }
        }
    }
#endif
}

/// <summary>
/// 
/// </summary>
void sysex_t::Identify()
{
    if (_Data.size() < 5)
        return;

#ifdef later
    _Manufacturer = "Unknown";

    // Identify the manufacturer.
    if (_Data[1] != 0)
    {
        const uint8_t ManufacturerId = _Data[1];

        for (const auto & Manufacturer : Manufacturers)
        {
            if (Manufacturer.Id == ManufacturerId)
            {
                _Manufacturer = Manufacturer.Name;
                break;
            }
        }

        const uint8_t DeviceId = _Data[3];
        const uint8_t CommandId = _Data[4];

        if ((DeviceId == 0x42 /* GS */) && (CommandId == 0x12 /* DT1 */))
        {
            IdentifyGSMessage();
        }
        else
        {
            for (const auto & Message : SysExMessages)
            {
                if ((Message.KeySize <= _Data.size()) && (::memcmp(Message.KeyData, _Data.data(), 2u) == 0) && (::memcmp(Message.KeyData + 3, _Data.data() + 3, (size_t) Message.KeySize - 3) == 0))
                {
                    _Description = Message.Description;
                    break;
                }
            }
        }
    }
    else
    if (_Data.size() > 4)
    {
        const uint16_t Id = (uint16_t) ((_Data[2] << 8) | _Data[3]);

        switch (Id)
        {
        // 0x0001 - 0x1F7F American
            case 0x0009: _Manufacturer = "New England Digital"; break;
            case 0x0016: _Manufacturer = "Opcode"; break;
            case 0x001B: _Manufacturer = "Peavey"; break;
            case 0x001C: _Manufacturer = "360 Systems"; break;
            case 0x001F: _Manufacturer = "Zeta"; break;
            case 0x002F: _Manufacturer = "Encore Electronics"; break;
            case 0x003B: _Manufacturer = "MOTU"; break;
            case 0x0041: _Manufacturer = "Microsoft"; break;
            case 0x004D: _Manufacturer = "Studio Electronics"; break;
            case 0x007E: _Manufacturer = "MIDIbox"; break;

            case 0x0105: _Manufacturer = "M-Audio"; break;
            case 0x0121: _Manufacturer = "Cakewalk"; break;
            case 0x0137: _Manufacturer = "Roger Linn Design"; break;
            case 0x013F: _Manufacturer = "Numark / Alesis"; break;
            case 0x014D: _Manufacturer = "Open Labs"; break;
            case 0x0172: _Manufacturer = "Kilpatrick Audio"; break;
            case 0x0177: _Manufacturer = "Nektar"; break;

            case 0x0214: _Manufacturer = "Intellijel"; break;
            case 0x021F: _Manufacturer = "Madrona Labs"; break;
            case 0x0226: _Manufacturer = "Electro-Harmonix"; break;

        // 0x2000 - 0x3F7F European
            case 0x2013: _Manufacturer = "Kenton"; break;
            case 0x201A: _Manufacturer = "Fatar / Studiologic"; break;
            case 0x201F: _Manufacturer = "TC Electronic"; break;
            case 0x2029: _Manufacturer = "Novation"; break;
            case 0x2032: _Manufacturer = "Behringer"; break;
            case 0x2033: _Manufacturer = "Access Music"; break;
            case 0x203A: _Manufacturer = "Propellorhead"; break;
            case 0x203B: _Manufacturer = "Red Sound"; break;
            case 0x204D: _Manufacturer = "Vermona"; break;
            case 0x2050: _Manufacturer = "Hartmann"; break;
            case 0x2052: _Manufacturer = "Analogue Systems"; break;
            case 0x205F: _Manufacturer = "Sequentix"; break;
            case 0x2069: _Manufacturer = "Elby Designs"; break;
            case 0x206B: _Manufacturer = "Arturia"; break;
            case 0x2076: _Manufacturer = "Teenage Engineering"; break;
            case 0x2102: _Manufacturer = "Mutable Instruments"; break;
            case 0x2107: _Manufacturer = "Modal Electronics"; break;
            case 0x2109: _Manufacturer = "Native Instruments"; break;
            case 0x2110: _Manufacturer = "ROLI"; break;
            case 0x211A: _Manufacturer = "IK Multimedia"; break;
            case 0x2127: _Manufacturer = "Expert Sleepers"; break;
            case 0x2135: _Manufacturer = "Dreadbox"; break;
            case 0x2141: _Manufacturer = "Marienberg"; break;

        // 0x4000 - 0x5F7F Japanese

        // 0x6000 - 0x7F7F Other
            default:
                _Manufacturer = "Other";
        }
    }
#endif
}

void sysex_t::Identify2() noexcept
{
    assert(_Data[0] == 0xF0);

    Manufacturer = "Unknown";

    switch (_Data[1])
    {
        case 0x41: Identify2Roland(); break;
        case 0x43: Identify2Yamaha(); break;

        // Universal Non-Real Time
        case 0x7E:
        {
            Description = "Universal Non-Real Time ";

            switch (_Data[3])
            {
                case 0x06:
                {
                    switch (_Data[4])
                    {
                        case 0x01: Description += "Identity Request"; break;
                        case 0x02: Description += "Identity Reply"; break;
                        default:
                            ::DebugBreak();
                    }
                    break;
                }

                // Universal Non-Real Time (General MIDI, 1991)
                case 0x09:
                {
                    switch (_Data[4])
                    {
                        case 0x01: Description += "GM1 System On"; break;
                        case 0x02: Description += "GM1 System Off"; break;

                        // Universal Non-Real Time (General MIDI Level 2, 1999)
                        case 0x03: Description += "GM2 System On"; break;
                        default:
                            ::DebugBreak();
                    }
                    break;
                }

                // DLS Level 2.2, 1.16 DLS System Exclusive Messages
                case 0x0A:
                {
                    switch (_Data[4])
                    {
                        case 0x01: Description += "DLS On"; break;
                        case 0x02: Description += "DLS Off"; break;
                        case 0x03: Description += "DLS Static Voice Allocation Off"; break;
                        case 0x04: Description += "DLS Static Voice Allocation On"; break;
                        default:
                            ::DebugBreak();
                    }
                    break;
                }
            }
            break;
        }

        //  Universal Real Time
        case 0x7F:
        {
            Description = " Universal Real Time ";

            switch (_Data[3])
            {
                // Universal Real Time
                case 0x03:
                {
                    switch (_Data[4])
                    {
                        case 0x01: Description += "Bar Number"; break;
                        case 0x02: Description += "Time Signature (Immediate)"; break;
                        case 0x03: Description += "Time Signature (Delayed)"; break;
                        default:
                            ::DebugBreak();
                    }
                    break;
                }

                // Universal Real Time (General MIDI Level 2, 1999)
                case 0x04:
                {
                    switch (_Data[4])
                    {
                        case 0x01: Description += "Master Volume"; break;
                        case 0x02: Description += "Master Balance"; break;
                        case 0x03: Description += "Master Fine Tune"; break;
                        case 0x04: Description += "Master Coarse Tune"; break;
                        case 0x05:
                        {
                            if (::memcmp (&_Data[5], "\x01\x01\x01\x01\x01", 5) == 0)
                                Description += "Global Reverb Parameters";
                            else
                            if (::memcmp (&_Data[5], "\x01\x01\x01\x01\x02", 5) == 0)
                                Description += "Global Chorus Parameters";
                            break;
                        }
                        default:
                            ::DebugBreak();
                    }
                    break;
                }

                case 0x05:
                {
                    Description += "Real-Time MTC Cueing Command";
                    break;
                }

                case 0x06:
                {
                    Description += "MIDI Machine Control Command";
                    break;
                }

                case 0x07:
                {
                    Description += "MIDI Machine Control Response";
                    break;
                }

                case 0x08:
                {
                    switch (_Data[4])
                    {
                        case 0x02: Description += "Single Note Tuning Change"; break;

                        case 0x07: Description += "Single Note Tuning Change with Bank Select"; break;
                        case 0x08: Description += "Scale/Octave Tuning Adjust"; break;
                        case 0x09: Description += "Scale/Octave Tuning Adjust (2-byte format)"; break;
                        default:
                            ::DebugBreak();
                    }
                    break;
                }

                case 0x09:
                {
                    switch (_Data[4])
                    {
                        case 0x01: Description += "Channel Pressure (Aftertouch)"; break;
                        case 0x03: Description += "Controller (Control Change)"; break;
                        default:
                            ::DebugBreak();
                    }
                    break;
                }

                case 0x0A:
                {
                    switch (_Data[4])
                    {
                        case 0x01: Description += "Key-based Instrument Controller"; break;
                        default:
                            ::DebugBreak();
                    }
                    break;
                }

                case 0x0B:
                {
                    switch (_Data[4])
                    {
                        case 0x01: Description += "Scalable Polyphony MIDI MIP Message"; break;
                        default:
                            ::DebugBreak();
                    }
                    break;
                }

                case 0x0C:
                {
                    switch (_Data[4])
                    {
                        case 0x00: Description += "Mobile Phone Control Message"; break;
                        default:
                            ::DebugBreak();
                    }
                    break;
                }

                default:
                    ::DebugBreak();
            }
            break;
        }

        default:
        {
            Description  = "Unknown";
//          ::DebugBreak();
            break;
        }
    }
}

void sysex_t::Identify2Roland() noexcept
{
    Manufacturer = "Roland";

    auto Iter = _Data.begin() + 3;

    // Model ID
    Model = "Unknown";

    switch (*Iter++)
    {
        case 0x14: Model = "MT-32"; break;    // Multi-timbral sound module
        case 0x15: Model = "D-110"; break;    // Multi-timbral sound module
        case 0x16: Model = "D-50"; break;     // Also D-550
        case 0x2A: Model = "TR-808"; break;   // Rhythm machine
        case 0x2B: Model = "TR-909"; break;   // Rhythm machine
        case 0x38: Model = "RSP-550"; break;  // Multi-effects unit
        case 0x42: Model = "GS"; break;       // GS-compatible device (e.g. SC-88)

        case 0x00:
        {
            switch (*Iter++)
            {
                case 0x6B: Model = "Fantom-X6/X7/X8"; break; // Performance synthesizers, used for patch changes and parameter control.

                case 0x00:
                {
                    switch (*Iter++)
                    {
                        case 0x3A: Model = "Juno-DS"; break;
                        case 0x64: Model = "Integra-7"; break;
                        case 0x20: Model = "V-1HD"; break; // Video switcher, uses a 4-byte Model ID.
                        case 0x2D: Model = "TR-8S"; break; // Drum machine, used for pattern and kit data.
                        case 0x4A: Model = "Jupiter-X/Xm"; break; // Modern synthesizers, used for patch and system settings.
                        case 0x45: Model = "Fantom (2019)"; break; // Modern Fantom series workstations.
                        case 0x0B: Model = "MC-707"; break; // Groovebox, used for track and pattern data.
                        case 0x0C: Model = "MC-101"; break; // Compact groovebox, similar to MC-707.
                        case 0x1E: Model = "SH-4d"; break; // Desktop synthesizer, used for patch and sequence data.
                        case 0x29: Model = "TR-06"; break; // Used for Roland Boutique synths. Also JU-08A.
                        default: Model = "Unknown"; break;
                    }
                    break;
                }
            }
            break;
        }
    }

    // Command ID
    switch (*Iter++)
    {
        case 0x11: Command = "RQ1"; break; // Data Request 1
        case 0x12: Command = "DT1"; break; // Data Set 1
        case 0x13: Command = "ACK"; break;
        case 0x14: Command = "NAK"; break;
        default:   Command = "???"; break;
    }

    const uint32_t Address = (uint32_t) (Iter[0] << 16) | (Iter[1] << 8) | Iter[2];

    Description = "Unknown";

    switch (Iter[0])
    {
        // System parameters
        case 0x00:
        {
            switch (Address)
            {
                case 0x00007F: Description = ::FormatText("System Mode Set %02X %s", Iter[3], "Mode 1"); break;
            }
            break;
        }

        // Patch Common Parameters. Parameters common to all Parts in each module (Block A)
        case 0x40:
        {
            switch (Address)
            {
                case 0x400000: Description = ::FormatText("Master Tune %02Xh (%d cents)",           Iter[3], (int) Iter[3]); break;

                case 0x400004: Description = ::FormatText("Master Volume %02Xh (%d)",               Iter[3], (int) Iter[3]); break;
                case 0x400006: Description = ::FormatText("Master Key Shift %02Xh (%d semitones)",  Iter[3], (int) Iter[3]); break;
                case 0x400005: Description = ::FormatText("Master Pan %02Xh (%d)",                  Iter[3], (int) Iter[3]); break;

                case 0x40007F: Description = ::FormatText("Mode Set %02Xh (%s)",                    Iter[3], "GS Reset"); break;

                case 0x400100: Description = "Patch Name <16 ASCII characters>"; break;

                case 0x400110: Description = "Reserved"; break;

                case 0x400130: Description = ::FormatText("Reverb Macro %02Xh (%s)",                Iter[3], IdentifyGSReverbMacro(Iter[3])); break;
                case 0x400131: Description = ::FormatText("Reberb Character %02Xh (%s)",            Iter[3], IdentifyGSReverbMacro(Iter[3])); break;
                case 0x400132: Description = ::FormatText("Reverb Pre-LPF %02Xh (%d)",              Iter[3], (int) Iter[3]); break;
                case 0x400133: Description = ::FormatText("Reverb Level %02Xh (%d)",                Iter[3], (int) Iter[3]); break;
                case 0x400134: Description = ::FormatText("Reverb Time %02Xh (%d)",                 Iter[3], (int) Iter[3]); break;
                case 0x400135: Description = ::FormatText("Reverb Delay Feedback %02Xh (%d)",       Iter[3], (int) Iter[3]); break;
                case 0x400137: Description = ::FormatText("Reverb Predelay Time %02Xh (%d ms)",     Iter[3], (int) Iter[3]); break;

                case 0x400138: Description = ::FormatText("Chorus Macro %02Xh (%s)",                Iter[3], IdentifyGSChorusMacro(Iter[3])); break;
                case 0x400139: Description = ::FormatText("Chorus Pre-LPF %02Xh (%d)",              Iter[3], (int) Iter[3]); break;
                case 0x40013A: Description = ::FormatText("Chorus Level %02Xh (%d)",                Iter[3], (int) Iter[3]); break;
                case 0x40013B: Description = ::FormatText("Chorus Feedback %02Xh (%d)",             Iter[3], (int) Iter[3]); break;
                case 0x40013C: Description = ::FormatText("Chorus Delay %02Xh (%d)",                Iter[3], (int) Iter[3]); break;
                case 0x40013D: Description = ::FormatText("Chorus Rate %02Xh (%d))",                Iter[3], (int) Iter[3]); break;
                case 0x40013E: Description = ::FormatText("Chorus Depth %02Xh (%d)",                Iter[3], (int) Iter[3]); break;
                case 0x40013F: Description = ::FormatText("Chorus Send Level to Reverb %02Xh (%d)", Iter[3], (int) Iter[3]); break;
                case 0x400140: Description = ::FormatText("Chorus Send Level to Delay %02Xh (%d)",  Iter[3], (int) Iter[3]); break;

                case 0x400150: Description += ::FormatText("Delay Macro %02Xh (%s)",                    Iter[3], IdentifyGSDelayMacro(Iter[3])); break;
                case 0x400151: Description += ::FormatText("Delay Pre-LPF %02Xh (%d)",                  Iter[3], (int) Iter[3]); break;
                case 0x400152: Description += ::FormatText("Delay Time Center %02Xh (%.1fms)",          Iter[3], (float) Iter[3] / 0x73); break;
                case 0x400153: Description += ::FormatText("Delay Time Ratio Left %02Xh (%d%%)",        Iter[3], (int) Iter[3] * 4); break;
                case 0x400154: Description += ::FormatText("Delay Time Ration Right %02Xh (%d%%)",      Iter[3], (int) Iter[3] * 4); break;
                case 0x400155: Description += ::FormatText("Delay Level Center %02Xh (%d)",             Iter[3], (int) Iter[3]); break;
                case 0x400156: Description += ::FormatText("Delay Level Left %02Xh (%d)",               Iter[3], (int) Iter[3]); break;
                case 0x400157: Description += ::FormatText("Delay Level Right %02Xh (%d)",              Iter[3], (int) Iter[3]); break;
                case 0x400158: Description += ::FormatText("Delay Level %02Xh (%d)",                    Iter[3], (int) Iter[3]); break;
                case 0x400159: Description += ::FormatText("Delay Feedback %02Xh (%d)",                 Iter[3], (int) Iter[3]); break;
                case 0x40015A: Description += ::FormatText("Delay Send Level to Reverb %02Xh (%d)",     Iter[3], (int) Iter[3]); break;

                default:
                {
                    // Patch Part Parameters. (Block A 00-0F)
                    switch (Address & 0xFFF0FF)
                    {
                        case 0x401000: Description = ::FormatText("Tone Number %02Xh (%d)",            Iter[3], (int) Iter[3]); break;

                        case 0x401002: Description = ::FormatText("Rx. Channel %02Xh (%d)",            Iter[3], (int) Iter[3]); break;
                        case 0x401003: Description = ::FormatText("Rx. Pitch Bend %02Xh (%s)",         Iter[3], (Iter[3] == 0x01 ? "On" : "Off")); break;
                        case 0x401004: Description = ::FormatText("Rx. Channel Pressure %02Xh (%s)",   Iter[3], (Iter[3] == 0x01 ? "On" : "Off")); break;
                        case 0x401005: Description = ::FormatText("Rx. Program Change %02Xh (%s)",     Iter[3], (Iter[3] == 0x01 ? "On" : "Off")); break;
                        case 0x401006: Description = ::FormatText("Rx. Control Change %02Xh (%s)",     Iter[3], (Iter[3] == 0x01 ? "On" : "Off")); break;
                        case 0x401007: Description = ::FormatText("Rx. Poly Pressure %02Xh (%s)",      Iter[3], (Iter[3] == 0x01 ? "On" : "Off")); break;
                        case 0x401008: Description = ::FormatText("Rx. Note Message %02Xh (%s)",       Iter[3], (Iter[3] == 0x01 ? "On" : "Off")); break;
                        case 0x401009: Description = ::FormatText("Rx. RPN %02Xh (%s)",                Iter[3], (Iter[3] == 0x01 ? "On" : "Off")); break;
                        case 0x40100A: Description = ::FormatText("Rx. NRPN %02Xh (%s)",               Iter[3], (Iter[3] == 0x01 ? "On" : "Off")); break;
                        case 0x40100B: Description = ::FormatText("Rx. Modulation %02Xh (%s)",         Iter[3], (Iter[3] == 0x01 ? "On" : "Off")); break;
                        case 0x40100C: Description = ::FormatText("Rx. Volume %02Xh (%s)",             Iter[3], (Iter[3] == 0x01 ? "On" : "Off")); break;
                        case 0x40100D: Description = ::FormatText("Rx. PanPot %02Xh (%s)",             Iter[3], (Iter[3] == 0x01 ? "On" : "Off")); break;
                        case 0x40100E: Description = ::FormatText("Rx. Expression %02Xh (%s)",         Iter[3], (Iter[3] == 0x01 ? "On" : "Off")); break;
                        case 0x40100F: Description = ::FormatText("Rx. Hold1 %02Xh (%s)",              Iter[3], (Iter[3] == 0x01 ? "On" : "Off")); break;
                        case 0x401010: Description = ::FormatText("Rx. Portamento %02Xh (%s)",         Iter[3], (Iter[3] == 0x01 ? "On" : "Off")); break;
                        case 0x401011: Description = ::FormatText("Rx. Sostenuto %02Xh (%s)",          Iter[3], (Iter[3] == 0x01 ? "On" : "Off")); break;
                        case 0x401012: Description = ::FormatText("Rx. Soft %02Xh (%s)",               Iter[3], (Iter[3] == 0x01 ? "On" : "Off")); break;
                        case 0x401013: Description = ::FormatText("Rx. Mono/Poly Mode %02Xh (%s)",     Iter[3], (Iter[3] == 0x01 ? "Poly" : "Mono")); break;
                        case 0x401014: Description = ::FormatText("Rx. Assign Mode %02Xh (%s)",        Iter[3], (Iter[3] == 0x00 ? "Single" : (Iter[3] == 0x01 ? "Limited-Multi" : "Full-Multi"))); break;

                        case 0x401015: Description = ::FormatText("Use for Rhythm Part %02Xh (%s)",         Iter[3], (Iter[3] == 0x00 ? "Off" : (Iter[3] == 0x01 ? "MAMP1" : "MAP2"))); break;
                        case 0x401016: Description = ::FormatText("Pitch Key Shift %02Xh (%d semitones)",   Iter[3], (int) Iter[3]); break;
                        case 0x401017: Description = ::FormatText("Pitch Offset Fine %02Xh %02Xh (%.1fHz)", Iter[3], Iter[4], ((Iter[4] << 8) | Iter[3])); break;

                        case 0x401019: Description = ::FormatText("Part Level %02Xh (%d)",                  Iter[3], (int) Iter[3]); break;
                        case 0x40101A: Description = ::FormatText("Velocity Sense Depth %02Xh (%d)",        Iter[3], (int) Iter[3]); break;
                        case 0x40101B: Description = ::FormatText("Velocity Sense Offset %02Xh (%d)",       Iter[3], (int) Iter[3]); break;
                        case 0x40101C: Description = ::FormatText("Part PanPot %02Xh (%d)",                 Iter[3], (int) Iter[3]); break;
                        case 0x40101D: Description = ::FormatText("Keyboard Range Low %02Xh (%d)",          Iter[3], (int) Iter[3]); break;
                        case 0x40101E: Description = ::FormatText("Keyboard Range High %02Xh (%d)",         Iter[3], (int) Iter[3]); break;
                        case 0x40101F: Description = ::FormatText("CC1 Controller NUmber %02Xh (%d)",       Iter[3], (int) Iter[3]); break;
                        case 0x401020: Description = ::FormatText("CC2 Controller Number %02Xh (%d)",       Iter[3], (int) Iter[3]); break;
                        case 0x401021: Description = ::FormatText("Chorus Send Level %02Xh (%d)",           Iter[3], (int) Iter[3]); break;
                        case 0x401022: Description = ::FormatText("Reverb Send Level %02Xh (%d)",           Iter[3], (int) Iter[3]); break;

                        case 0x401023: Description = ::FormatText("Rx. Bank Select %02Xh (%s)",             Iter[3], (Iter[3] == 0x01 ? "On" : "Off")); break;
                        case 0x401024: Description = ::FormatText("Rx. Bank Select LSB %02Xh (%s)",         Iter[3], (Iter[3] == 0x01 ? "On" : "Off")); break;

                        case 0x40102A: Description = ::FormatText("Pitch Fine Tune %02Xh %02Xh (%d cents)", Iter[3], Iter[4], (int) ((((Iter[3] & 0x7F) << 7) | (Iter[4] & 0x7F)) * 100) / 0x7F7F); break;

                        case 0x40102C: Description = ::FormatText("Delay Send Level %02Xh (%d)",                        Iter[3], (int) Iter[3]); break;

                        case 0x401030: Description = ::FormatText("Tone Modify 1 Vibrato Rate %02Xh (%d)",              Iter[3], (int) Iter[3]); break;
                        case 0x401031: Description = ::FormatText("Tone Modify 2 Vibrato Depth %02Xh (%d)",             Iter[3], (int) Iter[3]); break;

                        case 0x401032: Description = ::FormatText("Tone Modify 3 TVF Cutoff Freq. %02Xh (%d)",          Iter[3], (int) Iter[3]); break;
                        case 0x401033: Description = ::FormatText("Tone Modify 4 TVF Resonance %02Xh (%d)",             Iter[3], (int) Iter[3]); break;
                        case 0x401034: Description = ::FormatText("Tone Modify 5 TVF & TVA Env. attack %02Xh (%d)",     Iter[3], (int) Iter[3]); break;
                        case 0x401035: Description = ::FormatText("Tone Modify 5 TVF & TVA Env. decay %02Xh (%d)",      Iter[3], (int) Iter[3]); break;
                        case 0x401036: Description = ::FormatText("Tone Modify 5 TVF & TVA Env. release %02Xh (%d)",    Iter[3], (int) Iter[3]); break;

                        case 0x401037: Description = ::FormatText("Tone Modify 8 Vibrato Delay %02Xh (%d)",             Iter[3], (int) Iter[3]); break;
                    }
                }
            }
            break;
        }

        default:
            Description = ::FormatText("%06X", Address);
    }
/*
        // Patch parameters (Patch Common)
        case 0x400200: _Description += ::FormatText(L"GS Eq Low Frequency %d", (int) Iter[3]); break; // 200Hz, 400Hz
        case 0x400201: _Description += ::FormatText(L"GS Eq Low Gain %d", (int) Iter[3]); break; // -12dB .. +12dB
        case 0x400202: _Description += ::FormatText(L"GS Eq High Frequency %d", (int) Iter[3]); break; // 3kHz, 6kHz
        case 0x400203: _Description += ::FormatText(L"GS Eq High Gain %d", (int) Iter[3]); break; // -12dB .. +12dB

        case 0x400300: _Description += ::FormatText(L"GS EFX Type"); break; // SC-88Pro

        case 0x400303: case 0x400304: case 0x400305: case 0x400306: case 0x400307: case 0x400308: case 0x400309: case 0x40030A: case 0x40030B: case 0x40030C:
        case 0x40030D: case 0x40030E: case 0x40030F: case 0x400310: case 0x400311: case 0x400312: case 0x400313: case 0x400314: case 0x400315: case 0x400316:
                       _Description += ::FormatText(L"GS EFX Parameter %d %d", (int) _Data[7] - 2, (int) Iter[3]); break; // SC-88Pro

        case 0x400317: _Description += ::FormatText(L"GS EFX Send Level to Reverb: %d", (int) Iter[3]); break; // 0 .. 127
        case 0x400318: _Description += ::FormatText(L"GS EFX Send Level to Chorus: %d", (int) Iter[3]); break; // 0 .. 127
        case 0x400319: _Description += ::FormatText(L"GS EFX Send Level to Delay: %d", (int) Iter[3]); break; // 0 .. 127

        // Patch Part Parameters. The Sound Canvas VA has 16 Parts. Parameters that can be set individually for each Part are called Patch Part parameters.
        default:
        {
            int PartNumber = GSBlockToPart(_Data[6] & 0x0F);

            switch (Address & 0xFFF0FF)
            {
                case 0x404000: _Description += ::FormatText(L"GS Tone Map Number: channel %d, %s", PartNumber, IdentifyGSToneMap(Iter[3])); break;            // TONE MAP NUMBER
                case 0x404001: _Description += ::FormatText(L"GS Tone Map-0 Number: channel %d, %s", PartNumber, IdentifyGSToneMap(Iter[3])); break;          // TONE MAP-0 NUMBER

                case 0x402010: _Description += ::FormatText(L"GS Bend Pitch Control: Channel %d, %d semitones", GSBlockToPart(_Data[6] & 0x0F), (int) Iter[3]); break;

                // Drum setup parameters
                default:
                {
                    int MapNumber = ((_Data[6] & 0xF0) >> 4) + 1;
                    int NoteNumber = (int) _Data[7];

                    switch (Address & 0xFF0F00)
                    {
                        case 0x410000: _Description += ::FormatText(L"GS Drum Map Name (Map %d)", MapNumber); break; // ASCII characters

                        case 0x410100: _Description += ::FormatText(L"GS Drum Play Note Number (Map %d, Note %d)", MapNumber, NoteNumber); break;
                        case 0x410200: _Description += ::FormatText(L"GS Drum Level (Map %d, Note %d)", MapNumber, NoteNumber); break;
                        case 0x410300: _Description += ::FormatText(L"GS Drum Assign Group Number (Map %d, Note %d)", MapNumber, NoteNumber); break;
                        case 0x410400: _Description += ::FormatText(L"GS Drum PanPot (Map %d, Note %d)", MapNumber, NoteNumber); break;
                        case 0x410500: _Description += ::FormatText(L"GS Drum Reverb Send Level (Map %d, Note %d)", MapNumber, NoteNumber); break;
                        case 0x410600: _Description += ::FormatText(L"GS Drum Chorus Send Level (Map %d, Note %d)", MapNumber, NoteNumber); break;

                        default: _Description += " UNKNOWN";
                    }
                }
            }
        }
    }
*/
//  VerifyChecksum(Iter[3]);
}

void sysex_t::Identify2Yamaha() noexcept
{
    Manufacturer = "Yamaha";
}
