
/** $VER: SysEx.cpp (2025.04.06) P. Stuer **/

#include <CppCoreCheck/Warnings.h>

#pragma warning(disable: 4100 4625 4626 4710 4711 4738 5045 ALL_CPPCORECHECK_WARNINGS)

#include "Encoding.h"
#include "Tables.h"
#include "SysEx.h"

static const wchar_t * IdentifyGSReverbMacro(uint8_t value) noexcept
{
    switch (value)
    {
        case 0x00: return L"Room 1"; break;
        case 0x01: return L"Room 2"; break;
        case 0x02: return L"Room 3"; break;
        case 0x03: return L"Hall 1"; break;
        case 0x04: return L"Hall 2"; break;
        case 0x05: return L"Plate"; break;
        case 0x06: return L"Delay"; break;
        case 0x07: return L"Panning Delay"; break;

        default: return L"<Unknown>";
    }
}

static const wchar_t * IdentifyGSChorusMacro(uint8_t value) noexcept
{
    switch (value)
    {
        case 0x00: return L"Chorus 1"; break;
        case 0x01: return L"Chorus 2"; break;
        case 0x02: return L"Chorus 3"; break;
        case 0x03: return L"Chorus 4"; break;
        case 0x04: return L"Feedback Chorus"; break;
        case 0x05: return L"Flanger"; break;
        case 0x06: return L"Short Delay"; break;
        case 0x07: return L"Short Delay (FB)"; break;

        default: return L"<Unknown>";
    }
}

static const wchar_t * IdentifyGSDelayMacro(uint8_t value) noexcept
{
    switch (value)
    {
        case 0x00: return L"Delay 1"; break;
        case 0x01: return L"Delay 2"; break;
        case 0x02: return L"Delay 3"; break;
        case 0x03: return L"Delay 4"; break;
        case 0x04: return L"Pan Delay 1"; break;
        case 0x05: return L"Pan Delay 2"; break;
        case 0x06: return L"Pan Delay 3"; break;
        case 0x07: return L"Pan Delay 4"; break;
        case 0x08: return L"Delay to Reverb"; break;
        case 0x09: return L"Pan Repeat"; break;

        default: return L"<Unknown>";
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

    switch (Address)
    {
        // System parameters
        case 0x00007F: _Description = L"GS System Mode Set"; break;

        // Patch parameters (Patch Common)
        case 0x400000: _Description = FormatText(L"GS Master Tune %d", (int) _Data[8]); break;                                                  // MASTER TUNE (-100.0 .. +100.0 cents)
        case 0x400004: _Description = FormatText(L"GS Master Volume %d", (int) _Data[8]); break;                                                // MASTER VOLUME (0 .. 127)
        case 0x400006: _Description = FormatText(L"GS Master Key Shift %d semitones", (int) _Data[8]); break;                                   // MASTER KEY-SHIFT (-24 .. +24 semitones)
        case 0x400005: _Description = FormatText(L"GS Master Pan", (int) _Data[8]); break;                                                      // MASTER PAN (-63 .. +63) (Left to right)

        case 0x40007F: _Description = L"GS Reset"; break;                                                                                       // MODE SET (0 = GS Reset)

        case 0x400100: _Description = L"GS Patch Name"; break;                                                                                  // PATCH NAME (16 ASCII characters)

        case 0x400110: _Description = L"GS Reserved"; break;

        case 0x400130: _Description = FormatText(L"GS Reverb Macro %s", IdentifyGSReverbMacro(_Data[8])); break;                                // REVERB MACRO (0 .. 7)
        case 0x400131: _Description = FormatText(L"GS Reberb Character %s", IdentifyGSReverbMacro(_Data[8])); break;                            // REVERB CHARACTER (0 .. 7)
        case 0x400132: _Description = FormatText(L"GS Reverb Pre-LPF %d", (int) _Data[8]); break;                                               // REVERB PRE-LPF (0 .. 7)
        case 0x400133: _Description = FormatText(L"GS Reverb Level %d", (int) _Data[8]); break;                                                 // REVERB LEVEL (0 .. 127)
        case 0x400134: _Description = FormatText(L"GS Reverb Time %d", (int) _Data[8]); break;                                                  // REVERB TIME (0 .. 127)
        case 0x400135: _Description = FormatText(L"GS Reverb Delay Feedback %d", (int) _Data[8]); break;                                        // REVERB DELAY FEEDBACK (0 .. 127)

        case 0x400137: _Description = FormatText(L"GS Reverb Predelay Time %d", (int) _Data[8]); break;                                         // REVERB PREdelay TIME (0 .. 127 ms)

        case 0x400138: _Description = FormatText(L"GS Chorus Macro %s", IdentifyGSChorusMacro(_Data[8])); break;
        case 0x400139: _Description = FormatText(L"GS Chorus Pre-LPF %d", (int) _Data[8]); break; // 0 .. 7
        case 0x40013A: _Description = FormatText(L"GS Chorus Level %d", (int) _Data[8]); break; // 0 .. 127
        case 0x40013B: _Description = FormatText(L"GS Chorus Feedback %d", (int) _Data[8]); break; // 0 .. 127
        case 0x40013C: _Description = FormatText(L"GS Chorus Delay %d", (int) _Data[8]); break; // 0 .. 127
        case 0x40013D: _Description = FormatText(L"GS Chorus Rate %d", (int) _Data[8]); break; // 0 .. 127
        case 0x40013E: _Description = FormatText(L"GS Chorus Depth %d", (int) _Data[8]); break; // 0 .. 127
        case 0x40013F: _Description = FormatText(L"GS Chorus Send Level to Reverb %d", (int) _Data[8]); break; // 0 .. 127
        case 0x400140: _Description = FormatText(L"GS Chorus Send Level to Delay %d", (int) _Data[8]); break; // 0 .. 127

        case 0x400150: _Description = FormatText(L"GS Delay Macro %s", IdentifyGSDelayMacro(_Data[8])); break;
        case 0x400151: _Description = FormatText(L"GS Delay Pre-LPF %d", (int) _Data[8]); break; // 0 .. 7
        case 0x400152: _Description = FormatText(L"GS Delay Time Center %d", (int) _Data[8]); break; // 0.1ms .. 1s
        case 0x400153: _Description = FormatText(L"GS Delay Time Ratio Left %d", (int) _Data[8]); break; // 4% .. 500%
        case 0x400154: _Description = FormatText(L"GS Delay Time Ration Right %d", (int) _Data[8]); break; // 4% .. 500%
        case 0x400155: _Description = FormatText(L"GS Delay Level Center %d", (int) _Data[8]); break; // 0 .. 127
        case 0x400156: _Description = FormatText(L"GS Delay Level Left %d", (int) _Data[8]); break; // 0 .. 127
        case 0x400157: _Description = FormatText(L"GS Delay Level Right %d", (int) _Data[8]); break; // 0 .. 127
        case 0x400158: _Description = FormatText(L"GS Delay Level %d", (int) _Data[8]); break; // 0 .. 127
        case 0x400159: _Description = FormatText(L"GS Delay Feedback %d", (int) _Data[8]); break; // -64 .. +63
        case 0x40015A: _Description = FormatText(L"GS Delay Send Level to Reverb: %d", (int) _Data[8]); break; // 0 .. 127

        case 0x400200: _Description = FormatText(L"GS Eq Low Frequency %d", (int) _Data[8]); break; // 200Hz, 400Hz
        case 0x400201: _Description = FormatText(L"GS Eq Low Gain %d", (int) _Data[8]); break; // -12dB .. +12dB
        case 0x400202: _Description = FormatText(L"GS Eq High Frequency %d", (int) _Data[8]); break; // 3kHz, 6kHz
        case 0x400203: _Description = FormatText(L"GS Eq High Gain %d", (int) _Data[8]); break; // -12dB .. +12dB

        case 0x400300: _Description = FormatText(L"GS EFX Type"); break; // SC-88Pro

        case 0x400303: case 0x400304: case 0x400305: case 0x400306: case 0x400307: case 0x400308: case 0x400309: case 0x40030A: case 0x40030B: case 0x40030C:
        case 0x40030D: case 0x40030E: case 0x40030F: case 0x400310: case 0x400311: case 0x400312: case 0x400313: case 0x400314: case 0x400315: case 0x400316:
                       _Description = FormatText(L"GS EFX Parameter %d %d", (int) _Data[7] - 2, (int) _Data[8]); break; // SC-88Pro

        case 0x400317: _Description = FormatText(L"GS EFX Send Level to Reverb: %d", (int) _Data[8]); break; // 0 .. 127
        case 0x400318: _Description = FormatText(L"GS EFX Send Level to Chorus: %d", (int) _Data[8]); break; // 0 .. 127
        case 0x400319: _Description = FormatText(L"GS EFX Send Level to Delay: %d", (int) _Data[8]); break; // 0 .. 127

        // Patch Part Parameters. The Sound Canvas VA has 16 Parts. Parameters that can be set individually for each Part are called Patch Part parameters.
        default:
        {
            int PartNumber = GSBlockToPart(_Data[6] & 0x0F);

            switch (Address & 0xFFF0FF)
            {
                case 0x401014: _Description = FormatText(L"GS Assign Mode %d to channel %d", (int) _Data[8], PartNumber); break;                            // ASSIGN MODE
                case 0x401015: _Description = FormatText(L"GS Use channel %d for Rhythm Part: %s", PartNumber, IdentifyGSRhythmPart(_Data[8])); break;      // USE FOR RHYTHM PART

                case 0x401030: _Description = FormatText(L"GS Tone Modify 1, channel %d: %d", PartNumber, (int) _Data[8]); break;                           // TONE MODIFY1 Vibrato Rate
                case 0x401031: _Description = FormatText(L"GS Tone Modify 2, channel %d: %d", PartNumber, (int) _Data[8]); break;                           // TONE MODIFY2 Vibrato Depth
                case 0x401032: _Description = FormatText(L"GS Tone Modify 3, channel %d: %d", PartNumber, (int) _Data[8]); break;                           // TONE MODIFY3 TVF Cutoff Frequency
                case 0x401033: _Description = FormatText(L"GS Tone Modify 4, channel %d: %d", PartNumber, (int) _Data[8]); break;                           // TONE MODIFY4 TVF Resonance
                case 0x401034: _Description = FormatText(L"GS Tone Modify 5, channel %d: %d", PartNumber, (int) _Data[8]); break;                           // TONE MODIFY5 TVF & TVA Envelope Attack
                case 0x401035: _Description = FormatText(L"GS Tone Modify 6, channel %d: %d", PartNumber, (int) _Data[8]); break;                           // TONE MODIFY6 TVF & TVA Envelope Decay
                case 0x401036: _Description = FormatText(L"GS Tone Modify 7, channel %d: %d", PartNumber, (int) _Data[8]); break;                           // TONE MODIFY7 TVF & TVA Envelope Release
                case 0x401037: _Description = FormatText(L"GS Tone Modify 8, channel %d: %d", PartNumber, (int) _Data[8]); break;                           // TONE MODIFY8 Vibrato Delay

                case 0x404000: _Description = FormatText(L"GS Tone Map Number: channel %d, %s", PartNumber, IdentifyGSToneMap(_Data[8])); break;            // TONE MAP NUMBER
                case 0x404001: _Description = FormatText(L"GS Tone Map-0 Number: channel %d, %s", PartNumber, IdentifyGSToneMap(_Data[8])); break;          // TONE MAP-0 NUMBER

                case 0x402010: _Description = FormatText(L"GS Bend Pitch Control: Channel %d, %d semitones", GSBlockToPart(_Data[6] & 0x0F), (int) _Data[8]); break;

                // Drum setup parameters
                default:
                {
                    int MapNumber = ((_Data[6] & 0xF0) >> 4) + 1;
                    int NoteNumber = (int) _Data[7];

                    switch (Address & 0xFF0F00)
                    {
                        case 0x410000: _Description = FormatText(L"GS Drum Map Name (Map %d)", MapNumber); break; // ASCII characters

                        case 0x410100: _Description = FormatText(L"GS Drum Play Note Number (Map %d, Note %d)", MapNumber, NoteNumber); break;
                        case 0x410200: _Description = FormatText(L"GS Drum Level (Map %d, Note %d)", MapNumber, NoteNumber); break;
                        case 0x410300: _Description = FormatText(L"GS Drum Assign Group Number (Map %d, Note %d)", MapNumber, NoteNumber); break;
                        case 0x410400: _Description = FormatText(L"GS Drum PanPot (Map %d, Note %d)", MapNumber, NoteNumber); break;
                        case 0x410500: _Description = FormatText(L"GS Drum Reverb Send Level (Map %d, Note %d)", MapNumber, NoteNumber); break;
                        case 0x410600: _Description = FormatText(L"GS Drum Chorus Send Level (Map %d, Note %d)", MapNumber, NoteNumber); break;

                        default: _Description = L"GS Unknown address";
                    }
                }
            }
        }
    }
}

/// <summary>
/// 
/// </summary>
void sysex_t::Identify()
{
    if (_Data.size() < 5)
        return;

    _Manufacturer = L"Unknown";

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
            case 0x0009: _Manufacturer = L"New England Digital"; break;
            case 0x0016: _Manufacturer = L"Opcode"; break;
            case 0x001B: _Manufacturer = L"Peavey"; break;
            case 0x001C: _Manufacturer = L"360 Systems"; break;
            case 0x001F: _Manufacturer = L"Zeta"; break;
            case 0x002F: _Manufacturer = L"Encore Electronics"; break;
            case 0x003B: _Manufacturer = L"MOTU"; break;
            case 0x0041: _Manufacturer = L"Microsoft"; break;
            case 0x004D: _Manufacturer = L"Studio Electronics"; break;
            case 0x007E: _Manufacturer = L"MIDIbox"; break;

            case 0x0105: _Manufacturer = L"M-Audio"; break;
            case 0x0121: _Manufacturer = L"Cakewalk"; break;
            case 0x0137: _Manufacturer = L"Roger Linn Design"; break;
            case 0x013F: _Manufacturer = L"Numark / Alesis"; break;
            case 0x014D: _Manufacturer = L"Open Labs"; break;
            case 0x0172: _Manufacturer = L"Kilpatrick Audio"; break;
            case 0x0177: _Manufacturer = L"Nektar"; break;

            case 0x0214: _Manufacturer = L"Intellijel"; break;
            case 0x021F: _Manufacturer = L"Madrona Labs"; break;
            case 0x0226: _Manufacturer = L"Electro-Harmonix"; break;

        // 0x2000 - 0x3F7F European
            case 0x2013: _Manufacturer = L"Kenton"; break;
            case 0x201A: _Manufacturer = L"Fatar / Studiologic"; break;
            case 0x201F: _Manufacturer = L"TC Electronic"; break;
            case 0x2029: _Manufacturer = L"Novation"; break;
            case 0x2032: _Manufacturer = L"Behringer"; break;
            case 0x2033: _Manufacturer = L"Access Music"; break;
            case 0x203A: _Manufacturer = L"Propellorhead"; break;
            case 0x203B: _Manufacturer = L"Red Sound"; break;
            case 0x204D: _Manufacturer = L"Vermona"; break;
            case 0x2050: _Manufacturer = L"Hartmann"; break;
            case 0x2052: _Manufacturer = L"Analogue Systems"; break;
            case 0x205F: _Manufacturer = L"Sequentix"; break;
            case 0x2069: _Manufacturer = L"Elby Designs"; break;
            case 0x206B: _Manufacturer = L"Arturia"; break;
            case 0x2076: _Manufacturer = L"Teenage Engineering"; break;
            case 0x2102: _Manufacturer = L"Mutable Instruments"; break;
            case 0x2107: _Manufacturer = L"Modal Electronics"; break;
            case 0x2109: _Manufacturer = L"Native Instruments"; break;
            case 0x2110: _Manufacturer = L"ROLI"; break;
            case 0x211A: _Manufacturer = L"IK Multimedia"; break;
            case 0x2127: _Manufacturer = L"Expert Sleepers"; break;
            case 0x2135: _Manufacturer = L"Dreadbox"; break;
            case 0x2141: _Manufacturer = L"Marienberg"; break;

        // 0x4000 - 0x5F7F Japanese

        // 0x6000 - 0x7F7F Other
            default:
                _Manufacturer = L"Other";
        }
    }
}
