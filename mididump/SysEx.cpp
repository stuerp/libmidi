
/** $VER: SysEx.cpp (2025.07.15) P. Stuer **/

#include <CppCoreCheck/Warnings.h>

#pragma warning(disable: 4100 4625 4626 4710 4711 4738 5045 ALL_CPPCORECHECK_WARNINGS)

#include "Encoding.h"
#include "Tables.h"
#include "SysEx.h"

/// <summary>
/// Maps a value from one range (srcMin, srcMax) to another (dstMin, dstMax).
/// </summary>
template<class T, class U>
inline static U Map(T value, T srcMin, T srcMax, U dstMin, U dstMax)
{
    return dstMin + (U) (((double) (value - srcMin) * (double) (dstMax - dstMin)) / (double) (srcMax - srcMin));
}

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

static const char * IdentifyGSRhythmPart(uint8_t value) noexcept
{
    switch (value)
    {
        case 0x00: return "Off"; break;
        case 0x01: return "Map 1"; break;
        case 0x02: return "Map 2"; break;

        default: return "<Unknown>";
    }
}

static const char * IdentifyGSToneMap(uint8_t value) noexcept
{
    switch (value)
    {
        case 0x00: return "Selected"; break;
        case 0x01: return "SC-55 Map"; break;
        case 0x02: return "SC-88 Map"; break;
        case 0x03: return "SC-88Pro Map"; break;
        case 0x04: return "SC-8850 Map"; break;

        default: return "<Unknown>";
    }
}

void sysex_t::Identify() noexcept
{
    assert(_Data[0] == 0xF0);

    _Iter = _Data.begin() + 1;

    const uint32_t ManufacturerId = IdentifyManufacturer();

    const uint8_t DeviceId = *_Iter++;

    switch (ManufacturerId)
    {
        case 0x41: IdentifyRoland(); break;
        case 0x43: IdentifyYamaha(); break;

        // Universal Non-Real Time
        case 0x7E:
        {
            switch (_Data[3])
            {
                case 0x06:
                {
                    switch (_Data[4])
                    {
                        case 0x01: Description = "Identity Request"; break;
                        case 0x02: Description = "Identity Reply"; break;
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
                        case 0x01: Description = "GM1 System On"; break;
                        case 0x02: Description = "GM1 System Off"; break;

                        // Universal Non-Real Time (General MIDI Level 2, 1999)
                        case 0x03: Description = "GM2 System On"; break;
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
                        case 0x01: Description = "DLS On"; break;
                        case 0x02: Description = "DLS Off"; break;
                        case 0x03: Description = "DLS Static Voice Allocation Off"; break;
                        case 0x04: Description = "DLS Static Voice Allocation On"; break;
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
            switch (_Data[3])
            {
                // Universal Real Time
                case 0x03:
                {
                    switch (_Data[4])
                    {
                        case 0x01: Description = "Bar Number"; break;
                        case 0x02: Description = "Time Signature (Immediate)"; break;
                        case 0x03: Description = "Time Signature (Delayed)"; break;
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
                        case 0x01: Description = "Master Volume"; break;
                        case 0x02: Description = "Master Balance"; break;
                        case 0x03: Description = "Master Fine Tune"; break;
                        case 0x04: Description = "Master Coarse Tune"; break;
                        case 0x05:
                        {
                            if (::memcmp (&_Data[5], "\x01\x01\x01\x01\x01", 5) == 0)
                                Description = "Global Reverb Parameters";
                            else
                            if (::memcmp (&_Data[5], "\x01\x01\x01\x01\x02", 5) == 0)
                                Description = "Global Chorus Parameters";
                            break;
                        }
                        default:
                            ::DebugBreak();
                    }
                    break;
                }

                case 0x05:
                {
                    Description = "Real-Time MTC Cueing Command";
                    break;
                }

                case 0x06:
                {
                    Description = "MIDI Machine Control Command";
                    break;
                }

                case 0x07:
                {
                    Description = "MIDI Machine Control Response";
                    break;
                }

                case 0x08:
                {
                    switch (_Data[4])
                    {
                        case 0x02: Description = "Single Note Tuning Change"; break;

                        case 0x07: Description = "Single Note Tuning Change with Bank Select"; break;
                        case 0x08: Description = "Scale/Octave Tuning Adjust"; break;
                        case 0x09: Description = "Scale/Octave Tuning Adjust (2-byte format)"; break;
                        default:
                            ::DebugBreak();
                    }
                    break;
                }

                case 0x09:
                {
                    switch (_Data[4])
                    {
                        case 0x01: Description = "Channel Pressure (Aftertouch)"; break;
                        case 0x03: Description = "Controller (Control Change)"; break;
                        default:
                            ::DebugBreak();
                    }
                    break;
                }

                case 0x0A:
                {
                    switch (_Data[4])
                    {
                        case 0x01: Description = "Key-based Instrument Controller"; break;
                        default:
                            ::DebugBreak();
                    }
                    break;
                }

                case 0x0B:
                {
                    switch (_Data[4])
                    {
                        case 0x01: Description = "Scalable Polyphony MIDI MIP Message"; break;
                        default:
                            ::DebugBreak();
                    }
                    break;
                }

                case 0x0C:
                {
                    switch (_Data[4])
                    {
                        case 0x00: Description = "Mobile Phone Control Message"; break;
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

uint32_t sysex_t::IdentifyManufacturer() noexcept
{
    uint32_t ManufacturerId = *_Iter++;

    for (const auto & m : Manufacturers)
    {
        if (ManufacturerId == m.Id)
        {
            Manufacturer = m.Name;
            break;
        }
    }

    return ManufacturerId;
}

void sysex_t::IdentifyRoland() noexcept
{
    Manufacturer = "Roland";

    // Model ID
    Model = "Unknown";

    switch (*_Iter++)
    {
        case 0x14: Model = "MT-32"; break;      // Multi-timbral sound module
        case 0x15: Model = "D-110"; break;      // Multi-timbral sound module
        case 0x16: Model = "D-50"; break;       // Also D-550
        case 0x2A: Model = "TR-808"; break;     // Rhythm machine
        case 0x2B: Model = "TR-909"; break;     // Rhythm machine
        case 0x38: Model = "RSP-550"; break;    // Multi-effects unit
        case 0x42: Model = "GS"; break;         // GS-compatible device (e.g. SC-88)
        case 0x45: Model = "SC-55"; break;      // SC-55, SC-88, SC-88 Pro

        case 0x00:
        {
            switch (*_Iter++)
            {
                case 0x48: Model = "SD-20"; break;          // SD-20, SD-90

                case 0x6B: Model = "Fantom-Xx"; break;      // Fantom-X6, Fantom-X7, Fantom-X8

                case 0x00:
                {
                    switch (*_Iter++)
                    {
                        case 0x0B: Model = "MC-707"; break; // Groovebox, used for track and pattern data.
                        case 0x0C: Model = "MC-101"; break; // Compact groovebox, similar to MC-707.

                        case 0x1E: Model = "SH-4d"; break;  // Desktop synthesizer, used for patch and sequence data.
                        case 0x20: Model = "V-1HD"; break;  // Video switcher, uses a 4-byte Model ID.

                        case 0x29: Model = "TR-06"; break;  // Used for Roland Boutique synths. Also JU-08A.
                        case 0x2D: Model = "TR-8S"; break;  // Drum machine, used for pattern and kit data.

                        case 0x3A: Model = "Juno-DS"; break;
                        case 0x45: Model = "Fantom (2019)"; break;
                        case 0x4A: Model = "Jupiter-X/Xm"; break;
                        case 0x64: Model = "Integra-7"; break;

                        default: Model = "Unknown"; break;
                    }
                    break;
                }
            }
            break;
        }
    }

    // Command ID
    switch (*_Iter++)
    {
        case 0x11: Command = "RQ1"; break; // Data Request 1
        case 0x12: Command = "DT1"; break; // Data Set 1
        case 0x13: Command = "ACK"; break;
        case 0x14: Command = "NAK"; break;
        default:   Command = "???"; break;
    }

    const uint32_t Address = (uint32_t) (_Iter[0] << 16) | (_Iter[1] << 8) | _Iter[2];

    Description = "Unknown";

    switch (_Iter[0])
    {
        // System parameters. Parameters affecting the entire unit, such as how the two MIDI IN connectors will function.
        case 0x00:
        {
            switch (Address)
            {
                case 0x00007F: Description = ::FormatText("System Mode Set %02X %s", _Iter[3], "Mode 1"); break;
            }
            break;
        }

        // Patch Common Parameters A. Parameters common to all Parts in each module (Block A 00-0F)
        case 0x40:
        {
            switch (Address)
            {
                case 0x400000: Description = ::FormatText("Master Tune %02Xh (%d cents)",               _Iter[3], (int) _Iter[3]); break;

                case 0x400004: Description = ::FormatText("Master Volume %02Xh (%d)",                   _Iter[3], (int) _Iter[3]); break;
                case 0x400006: Description = ::FormatText("Master Key Shift %02Xh (%d semitones)",      _Iter[3], (int) _Iter[3]); break;
                case 0x400005: Description = ::FormatText("Master Pan %02Xh (%d)",                      _Iter[3], (int) _Iter[3]); break;

                case 0x40007F: Description = ::FormatText("Mode Set %02Xh (%s)",                        _Iter[3], "GS Reset"); break;

                case 0x400100:
                {
                    Description = "Patch Name ";

                    _Iter += 3;

                    while (*_Iter != 0xF7)
                    {
                        Description.append(1, (char) *_Iter++);
                    }
                    break;
                }

                case 0x400110: Description = "Reserved"; break;

                case 0x400130: Description = ::FormatText("Reverb Macro %02Xh (%s)",                    _Iter[3], IdentifyGSReverbMacro(_Iter[3])); break;
                case 0x400131: Description = ::FormatText("Reverb Character %02Xh (%s)",                _Iter[3], IdentifyGSReverbMacro(_Iter[3])); break;
                case 0x400132: Description = ::FormatText("Reverb Pre-LPF %02Xh (%d)",                  _Iter[3], (int) _Iter[3]); break;
                case 0x400133: Description = ::FormatText("Reverb Level %02Xh (%d)",                    _Iter[3], (int) _Iter[3]); break;
                case 0x400134: Description = ::FormatText("Reverb Time %02Xh (%d)",                     _Iter[3], (int) _Iter[3]); break;
                case 0x400135: Description = ::FormatText("Reverb Delay Feedback %02Xh (%d)",           _Iter[3], (int) _Iter[3]); break;
                case 0x400136: Description = ::FormatText("Reverb Send Level to Chorus %02Xh (%d)",     _Iter[3], (int) _Iter[3]); break;
                case 0x400137: Description = ::FormatText("Reverb Predelay Time %02Xh (%d ms)",         _Iter[3], (int) _Iter[3]); break;

                case 0x400138: Description = ::FormatText("Chorus Macro %02Xh (%s)",                    _Iter[3], IdentifyGSChorusMacro(_Iter[3])); break;
                case 0x400139: Description = ::FormatText("Chorus Pre-LPF %02Xh (%d)",                  _Iter[3], (int) _Iter[3]); break;
                case 0x40013A: Description = ::FormatText("Chorus Level %02Xh (%d)",                    _Iter[3], (int) _Iter[3]); break;
                case 0x40013B: Description = ::FormatText("Chorus Feedback %02Xh (%d)",                 _Iter[3], (int) _Iter[3]); break;
                case 0x40013C: Description = ::FormatText("Chorus Delay %02Xh (%d)",                    _Iter[3], (int) _Iter[3]); break;
                case 0x40013D: Description = ::FormatText("Chorus Rate %02Xh (%d))",                    _Iter[3], (int) _Iter[3]); break;
                case 0x40013E: Description = ::FormatText("Chorus Depth %02Xh (%d)",                    _Iter[3], (int) _Iter[3]); break;
                case 0x40013F: Description = ::FormatText("Chorus Send Level to Reverb %02Xh (%d)",     _Iter[3], (int) _Iter[3]); break;
                case 0x400140: Description = ::FormatText("Chorus Send Level to Delay %02Xh (%d)",      _Iter[3], (int) _Iter[3]); break;

                case 0x400150: Description = ::FormatText("Delay Macro %02Xh (%s)",                     _Iter[3], IdentifyGSDelayMacro(_Iter[3])); break;
                case 0x400151: Description = ::FormatText("Delay Pre-LPF %02Xh (%d)",                   _Iter[3], (int) _Iter[3]); break;
                case 0x400152: Description = ::FormatText("Delay Time Center %02Xh (%.1fms)",           _Iter[3], (float) _Iter[3] / 0x73); break;
                case 0x400153: Description = ::FormatText("Delay Time Ratio Left %02Xh (%d%%)",         _Iter[3], (int) _Iter[3] * 4); break;
                case 0x400154: Description = ::FormatText("Delay Time Ration Right %02Xh (%d%%)",       _Iter[3], (int) _Iter[3] * 4); break;
                case 0x400155: Description = ::FormatText("Delay Level Center %02Xh (%d)",              _Iter[3], (int) _Iter[3]); break;
                case 0x400156: Description = ::FormatText("Delay Level Left %02Xh (%d)",                _Iter[3], (int) _Iter[3]); break;
                case 0x400157: Description = ::FormatText("Delay Level Right %02Xh (%d)",               _Iter[3], (int) _Iter[3]); break;
                case 0x400158: Description = ::FormatText("Delay Level %02Xh (%d)",                     _Iter[3], (int) _Iter[3]); break;
                case 0x400159: Description = ::FormatText("Delay Feedback %02Xh (%d)",                  _Iter[3], (int) _Iter[3]); break;
                case 0x40015A: Description = ::FormatText("Delay Send Level to Reverb %02Xh (%d)",      _Iter[3], (int) _Iter[3]); break;

                case 0x400200: Description = ::FormatText("Eq Low Frequency %02Xh (%dHz)",              _Iter[3], (_Iter[3] == 0 ? 200 : 400)); break;
                case 0x400201: Description = ::FormatText("Eq Low Gain %02Xh (%ddB)",                   _Iter[3], Map((int) _Iter[3], 0x34, 0x4C, -12, 12)); break; // -12dB .. +12dB
                case 0x400202: Description = ::FormatText("Eq High Frequency %02Xh (%dkHz)",            _Iter[3], (_Iter[3] == 0 ? 3 : 6)); break;
                case 0x400203: Description = ::FormatText("Eq High Gain %02Xh (%ddB)",                  _Iter[3], Map((int) _Iter[3], 0x34, 0x4C, -12, 12)); break; // -12dB .. +12dB

                case 0x400300: Description = ::FormatText("EFX Type %02Xh (%ddB)",                      _Iter[3], (int) _Iter[3]); break;

                case 0x400303: case 0x400304: case 0x400305: case 0x400306: case 0x400307: case 0x400308: case 0x400309: case 0x40030A: case 0x40030B: case 0x40030C:
                case 0x40030D: case 0x40030E: case 0x40030F: case 0x400310: case 0x400311: case 0x400312: case 0x400313: case 0x400314: case 0x400315: case 0x400316:
                               Description = ::FormatText("EFX Parameter %d %02Xh (%d)", (int) _Iter[2] - 2, _Iter[3], (int) _Iter[3]); break;

                case 0x400317: Description = ::FormatText("EFX Send Level to Reverb %02Xh (%d)",        _Iter[3], (int) _Iter[3]); break;
                case 0x400318: Description = ::FormatText("EFX Send Level to Chorus %02Xh (%d)",        _Iter[3], (int) _Iter[3]); break;
                case 0x400319: Description = ::FormatText("EFX Send Level to Delay %02Xh (%d)",         _Iter[3], (int) _Iter[3]); break;

                case 0x40031B: Description = ::FormatText("EFX Control Source 1 %02Xh (%d)",            _Iter[3], (int) _Iter[3]); break;
                case 0x40031C: Description = ::FormatText("EFX Control Depth 1 %02Xh (%d%%)",           _Iter[3], Map((int) _Iter[3], 0x00, 0x7F, -100, 100)); break;
                case 0x40031D: Description = ::FormatText("EFX Control Source 2 %02Xh (%d)",            _Iter[3], (int) _Iter[3]); break;
                case 0x40031E: Description = ::FormatText("EFX Control Depth 2 %02Xh (%d%%)",           _Iter[3], Map((int) _Iter[3], 0x00, 0x7F, -100, 100)); break;

                case 0x40031F: Description = ::FormatText("EFX Send Level to Delay %02Xh (%s)",         _Iter[3], (_Iter[3] == 0x01 ? "On" : "Off")); break;

                default:
                {
                    switch (Address & 0xFFF0FF)
                    {
                        // Patch Part Parameters. (Block A 00-0F)
                        case 0x401000: Description = ::FormatText("Tone Number %02Xh (%d)",                 _Iter[3], (int) _Iter[3]); break;

                        case 0x401002: Description = ::FormatText("Rx. Channel %02Xh (%d)",                 _Iter[3], (int) _Iter[3]); break;
                        case 0x401003: Description = ::FormatText("Rx. Pitch Bend %02Xh (%s)",              _Iter[3], (_Iter[3] == 0x01 ? "On" : "Off")); break;
                        case 0x401004: Description = ::FormatText("Rx. Channel Pressure %02Xh (%s)",        _Iter[3], (_Iter[3] == 0x01 ? "On" : "Off")); break;
                        case 0x401005: Description = ::FormatText("Rx. Program Change %02Xh (%s)",          _Iter[3], (_Iter[3] == 0x01 ? "On" : "Off")); break;
                        case 0x401006: Description = ::FormatText("Rx. Control Change %02Xh (%s)",          _Iter[3], (_Iter[3] == 0x01 ? "On" : "Off")); break;
                        case 0x401007: Description = ::FormatText("Rx. Poly Pressure %02Xh (%s)",           _Iter[3], (_Iter[3] == 0x01 ? "On" : "Off")); break;
                        case 0x401008: Description = ::FormatText("Rx. Note Message %02Xh (%s)",            _Iter[3], (_Iter[3] == 0x01 ? "On" : "Off")); break;
                        case 0x401009: Description = ::FormatText("Rx. RPN %02Xh (%s)",                     _Iter[3], (_Iter[3] == 0x01 ? "On" : "Off")); break;
                        case 0x40100A: Description = ::FormatText("Rx. NRPN %02Xh (%s)",                    _Iter[3], (_Iter[3] == 0x01 ? "On" : "Off")); break;
                        case 0x40100B: Description = ::FormatText("Rx. Modulation %02Xh (%s)",              _Iter[3], (_Iter[3] == 0x01 ? "On" : "Off")); break;
                        case 0x40100C: Description = ::FormatText("Rx. Volume %02Xh (%s)",                  _Iter[3], (_Iter[3] == 0x01 ? "On" : "Off")); break;
                        case 0x40100D: Description = ::FormatText("Rx. PanPot %02Xh (%s)",                  _Iter[3], (_Iter[3] == 0x01 ? "On" : "Off")); break;
                        case 0x40100E: Description = ::FormatText("Rx. Expression %02Xh (%s)",              _Iter[3], (_Iter[3] == 0x01 ? "On" : "Off")); break;
                        case 0x40100F: Description = ::FormatText("Rx. Hold1 %02Xh (%s)",                   _Iter[3], (_Iter[3] == 0x01 ? "On" : "Off")); break;
                        case 0x401010: Description = ::FormatText("Rx. Portamento %02Xh (%s)",              _Iter[3], (_Iter[3] == 0x01 ? "On" : "Off")); break;
                        case 0x401011: Description = ::FormatText("Rx. Sostenuto %02Xh (%s)",               _Iter[3], (_Iter[3] == 0x01 ? "On" : "Off")); break;
                        case 0x401012: Description = ::FormatText("Rx. Soft %02Xh (%s)",                    _Iter[3], (_Iter[3] == 0x01 ? "On" : "Off")); break;
                        case 0x401013: Description = ::FormatText("Rx. Mono/Poly Mode %02Xh (%s)",          _Iter[3], (_Iter[3] == 0x01 ? "Poly" : "Mono")); break;
                        case 0x401014: Description = ::FormatText("Rx. Assign Mode %02Xh (%s)",             _Iter[3], (_Iter[3] == 0x00 ? "Single" : (_Iter[3] == 0x01 ? "Limited-Multi" : "Full-Multi"))); break;

                        case 0x401015: Description = ::FormatText("Use for Rhythm Part %02Xh (%s)",                     _Iter[3],           IdentifyGSRhythmPart(_Iter[3])); break;
                        case 0x401016: Description = ::FormatText("Pitch Key Shift %02Xh (%d semitones)",               _Iter[3],           Map((int) _Iter[3], 0x28, 0x58, -24, 24)); break;
                        case 0x401017: Description = ::FormatText("Pitch Offset Fine %02Xh %02Xh (%.1fHz)",             _Iter[3], _Iter[4], Map((_Iter[4] << 7) | _Iter[3], 0x08, 0xF8, -12.f, 12.f)); break;

                        case 0x401019: Description = ::FormatText("Part Level %02Xh (%d)",                              _Iter[3], (int) _Iter[3]); break;
                        case 0x40101A: Description = ::FormatText("Velocity Sense Depth %02Xh (%d)",                    _Iter[3], (int) _Iter[3]); break;
                        case 0x40101B: Description = ::FormatText("Velocity Sense Offset %02Xh (%d)",                   _Iter[3], (int) _Iter[3]); break;
                        case 0x40101C: Description = ::FormatText("Part PanPot %02Xh (%d)",                             _Iter[3], (int) _Iter[3]); break;
                        case 0x40101D: Description = ::FormatText("Keyboard Range Low %02Xh (%d)",                      _Iter[3], (int) _Iter[3]); break;
                        case 0x40101E: Description = ::FormatText("Keyboard Range High %02Xh (%d)",                     _Iter[3], (int) _Iter[3]); break;
                        case 0x40101F: Description = ::FormatText("CC1 Controller NUmber %02Xh (%d)",                   _Iter[3], (int) _Iter[3]); break;
                        case 0x401020: Description = ::FormatText("CC2 Controller Number %02Xh (%d)",                   _Iter[3], (int) _Iter[3]); break;
                        case 0x401021: Description = ::FormatText("Chorus Send Level %02Xh (%d)",                       _Iter[3], (int) _Iter[3]); break;
                        case 0x401022: Description = ::FormatText("Reverb Send Level %02Xh (%d)",                       _Iter[3], (int) _Iter[3]); break;

                        case 0x401023: Description = ::FormatText("Rx. Bank Select %02Xh (%s)",                         _Iter[3], (_Iter[3] == 0x01 ? "On" : "Off")); break;
                        case 0x401024: Description = ::FormatText("Rx. Bank Select LSB %02Xh (%s)",                     _Iter[3], (_Iter[3] == 0x01 ? "On" : "Off")); break;

                        case 0x401025: Description = ::FormatText("%06X <Undocumented>", Address); break;

                        case 0x40102A: Description = ::FormatText("Pitch Fine Tune %02Xh %02Xh (%d cents)",             _Iter[3], _Iter[4], Map((_Iter[3] << 7) | _Iter[4], 0x0000, 0x7F7F, -100, 100)); break;

                        case 0x40102C: Description = ::FormatText("Delay Send Level %02Xh (%d)",                        _Iter[3], (int) _Iter[3]); break;

                        case 0x401030: Description = ::FormatText("Tone Modify 1 Vibrato Rate %02Xh (%d)",              _Iter[3], (int) _Iter[3]); break;
                        case 0x401031: Description = ::FormatText("Tone Modify 2 Vibrato Depth %02Xh (%d)",             _Iter[3], (int) _Iter[3]); break;

                        case 0x401032: Description = ::FormatText("Tone Modify 3 TVF Cutoff Freq. %02Xh (%d)",          _Iter[3], (int) _Iter[3]); break;
                        case 0x401033: Description = ::FormatText("Tone Modify 4 TVF Resonance %02Xh (%d)",             _Iter[3], (int) _Iter[3]); break;
                        case 0x401034: Description = ::FormatText("Tone Modify 5 TVF & TVA Env. attack %02Xh (%d)",     _Iter[3], (int) _Iter[3]); break;
                        case 0x401035: Description = ::FormatText("Tone Modify 5 TVF & TVA Env. decay %02Xh (%d)",      _Iter[3], (int) _Iter[3]); break;
                        case 0x401036: Description = ::FormatText("Tone Modify 5 TVF & TVA Env. release %02Xh (%d)",    _Iter[3], (int) _Iter[3]); break;

                        case 0x401037: Description = ::FormatText("Tone Modify 8 Vibrato Delay %02Xh (%d)",             _Iter[3], (int) _Iter[3]); break;

                        case 0x401040: Description = ::FormatText("Scale Tuning C %02Xh (%d cents)",                    _Iter[3], Map((int) _Iter[3], 0x00, 0x7F, -64, 63)); break; // 12 bytes
                        case 0x401041: Description = ::FormatText("Scale Tuning C# %02Xh (%d cents)",                   _Iter[3], Map((int) _Iter[3], 0x00, 0x7F, -64, 63)); break; // 12 bytes
                        case 0x401042: Description = ::FormatText("Scale Tuning D %02Xh (%d cents)",                    _Iter[3], Map((int) _Iter[3], 0x00, 0x7F, -64, 63)); break; // 12 bytes
                        case 0x401043: Description = ::FormatText("Scale Tuning D# %02Xh (%d cents)",                   _Iter[3], Map((int) _Iter[3], 0x00, 0x7F, -64, 63)); break; // 12 bytes
                        case 0x401044: Description = ::FormatText("Scale Tuning E %02Xh (%d cents)",                    _Iter[3], Map((int) _Iter[3], 0x00, 0x7F, -64, 63)); break; // 12 bytes
                        case 0x401045: Description = ::FormatText("Scale Tuning F %02Xh (%d cents)",                    _Iter[3], Map((int) _Iter[3], 0x00, 0x7F, -64, 63)); break; // 12 bytes
                        case 0x401046: Description = ::FormatText("Scale Tuning F# %02Xh (%d cents)",                   _Iter[3], Map((int) _Iter[3], 0x00, 0x7F, -64, 63)); break; // 12 bytes
                        case 0x401047: Description = ::FormatText("Scale Tuning G %02Xh (%d cents)",                    _Iter[3], Map((int) _Iter[3], 0x00, 0x7F, -64, 63)); break; // 12 bytes
                        case 0x401048: Description = ::FormatText("Scale Tuning G# %02Xh (%d cents)",                   _Iter[3], Map((int) _Iter[3], 0x00, 0x7F, -64, 63)); break; // 12 bytes
                        case 0x401049: Description = ::FormatText("Scale Tuning A %02Xh (%d cents)",                    _Iter[3], Map((int) _Iter[3], 0x00, 0x7F, -64, 63)); break; // 12 bytes
                        case 0x40104A: Description = ::FormatText("Scale Tuning A# %02Xh (%d cents)",                   _Iter[3], Map((int) _Iter[3], 0x00, 0x7F, -64, 63)); break; // 12 bytes
                        case 0x40104B: Description = ::FormatText("Scale Tuning B %02Xh (%d cents)",                    _Iter[3], Map((int) _Iter[3], 0x00, 0x7F, -64, 63)); break; // 12 bytes

                        case 0x402000: Description = ::FormatText("Mod Pitch Control %02Xh (%d semitones)",             _Iter[3], Map((int) _Iter[3], 0x28, 0x58, -24, 24)); break;
                        case 0x402001: Description = ::FormatText("Mod TVF Cutoff Control %02Xh (%d cents)",            _Iter[3], Map((int) _Iter[3], 0x00, 0x7F, -9600, 9600)); break;
                        case 0x402002: Description = ::FormatText("Mod Amplitude Control %02Xh (%.1f%%)",               _Iter[3], Map((int) _Iter[3], 0x00, 0x7F, -100.f, 100.f)); break;

                        case 0x402003: Description = ::FormatText("Mod LFO1 Rate Control %02Xh (%.1fHz)",               _Iter[3], Map((int) _Iter[3], 0x00, 0x7F, -10.f, 10.f)); break;
                        case 0x402004: Description = ::FormatText("Mod LFO1 Pitch Depth %02Xh (%d cents)",              _Iter[3], Map((int) _Iter[3], 0x00, 0x7F, 0, 600)); break;
                        case 0x402005: Description = ::FormatText("Mod LFO1 TVF Depth %02Xh (%d cents)",                _Iter[3], Map((int) _Iter[3], 0x28, 0x58, 0, 2400)); break;
                        case 0x402006: Description = ::FormatText("Mod LFO1 TVA Depth %02Xh (%.1f%%)",                  _Iter[3], Map((int) _Iter[3], 0x28, 0x58, 0.f, 100.f)); break;

                        case 0x402007: Description = ::FormatText("Mod LFO2 Rate Control %02Xh (%.1fHz)",               _Iter[3], Map((int) _Iter[3], 0x00, 0x7F, -10.f, 10.f)); break;
                        case 0x402008: Description = ::FormatText("Mod LFO2 Pitch Depth %02Xh (%d cents)",              _Iter[3], Map((int) _Iter[3], 0x00, 0x7F, 0, 600)); break;
                        case 0x402009: Description = ::FormatText("Mod LFO2 TVF Depth %02Xh (%d cents)",                _Iter[3], Map((int) _Iter[3], 0x28, 0x58, 0, 2400)); break;
                        case 0x40200A: Description = ::FormatText("Mod LFO2 TVA Depth %02Xh (%.1f%%)",                  _Iter[3], Map((int) _Iter[3], 0x28, 0x58, 0.f, 100.f)); break;

                        case 0x402010: Description = ::FormatText("Bend Pitch Control %02Xh (%d semitones)",            _Iter[3], Map((int) _Iter[3], 0x28, 0x58, -24, 24)); break;
                        case 0x402011: Description = ::FormatText("Bend TVF Cutoff Control %02Xh (%d cents)",           _Iter[3], Map((int) _Iter[3], 0x00, 0x7F, -9600, 9600)); break;
                        case 0x402012: Description = ::FormatText("Bend Amplitude Control %02Xh (%.1f%%)",              _Iter[3], Map((int) _Iter[3], 0x00, 0x7F, -100.f, 100.f)); break;

                        case 0x402013: Description = ::FormatText("Bend LFO1 Rate Control %02Xh (%.1fHz)",              _Iter[3], Map((int) _Iter[3], 0x00, 0x7F, -10.f, 10.f)); break;
                        case 0x402014: Description = ::FormatText("Bend LFO1 Pitch Depth %02Xh (%d cents)",             _Iter[3], Map((int) _Iter[3], 0x00, 0x7F, 0, 600)); break;
                        case 0x402015: Description = ::FormatText("Bend LFO1 TVF Depth %02Xh (%d cents)",               _Iter[3], Map((int) _Iter[3], 0x28, 0x58, 0, 2400)); break;
                        case 0x402016: Description = ::FormatText("Bend LFO1 TVA Depth %02Xh (%.1f%%)",                 _Iter[3], Map((int) _Iter[3], 0x28, 0x58, 0.f, 100.f)); break;

                        case 0x402017: Description = ::FormatText("Bend LFO2 Rate Control %02Xh (%.1fHz)",              _Iter[3], Map((int) _Iter[3], 0x00, 0x7F, -10.f, 10.f)); break;
                        case 0x402018: Description = ::FormatText("Bend LFO2 Pitch Depth %02Xh (%d cents)",             _Iter[3], Map((int) _Iter[3], 0x00, 0x7F, 0, 600)); break;
                        case 0x402019: Description = ::FormatText("Bend LFO2 TVF Depth %02Xh (%d cents)",               _Iter[3], Map((int) _Iter[3], 0x28, 0x58, 0, 2400)); break;
                        case 0x40201A: Description = ::FormatText("Bend LFO2 TVA Depth %02Xh (%.1f%%)",                 _Iter[3], Map((int) _Iter[3], 0x28, 0x58, 0.f, 100.f)); break;

                        case 0x402020: Description = ::FormatText("CAf Pitch Control %02Xh (%d semitones)",             _Iter[3], Map((int) _Iter[3], 0x28, 0x58, -24, 24)); break;
                        case 0x402021: Description = ::FormatText("CAf TVF Cutoff Control %02Xh (%d cents)",            _Iter[3], Map((int) _Iter[3], 0x00, 0x7F, -9600, 9600)); break;
                        case 0x402022: Description = ::FormatText("CAf Amplitude Control %02Xh (%.1f%%)",               _Iter[3], Map((int) _Iter[3], 0x00, 0x7F, -100.f, 100.f)); break;

                        case 0x402023: Description = ::FormatText("CAf LFO1 Rate Control %02Xh (%.1fHz)",               _Iter[3], Map((int) _Iter[3], 0x00, 0x7F, -10.f, 10.f)); break;
                        case 0x402024: Description = ::FormatText("CAf LFO1 Pitch Depth %02Xh (%d cents)",              _Iter[3], Map((int) _Iter[3], 0x00, 0x7F, 0, 600)); break;
                        case 0x402025: Description = ::FormatText("CAf LFO1 TVF Depth %02Xh (%d cents)",                _Iter[3], Map((int) _Iter[3], 0x28, 0x58, 0, 2400)); break;
                        case 0x402026: Description = ::FormatText("CAf LFO1 TVA Depth %02Xh (%.1f%%)",                  _Iter[3], Map((int) _Iter[3], 0x28, 0x58, 0.f, 100.f)); break;

                        case 0x402027: Description = ::FormatText("CAf LFO2 Rate Control %02Xh (%.1fHz)",               _Iter[3], Map((int) _Iter[3], 0x00, 0x7F, -10.f, 10.f)); break;
                        case 0x402028: Description = ::FormatText("CAf LFO2 Pitch Depth %02Xh (%d cents)",              _Iter[3], Map((int) _Iter[3], 0x00, 0x7F, 0, 600)); break;
                        case 0x402029: Description = ::FormatText("CAf LFO2 TVF Depth %02Xh (%d cents)",                _Iter[3], Map((int) _Iter[3], 0x28, 0x58, 0, 2400)); break;
                        case 0x40202A: Description = ::FormatText("CAf LFO2 TVA Depth %02Xh (%.1f%%)",                  _Iter[3], Map((int) _Iter[3], 0x28, 0x58, 0.f, 100.f)); break;

                        case 0x402030: Description = ::FormatText("PAf Pitch Control %02Xh (%d semitones)",             _Iter[3], Map((int) _Iter[3], 0x28, 0x58, -24, 24)); break;
                        case 0x402031: Description = ::FormatText("PAf TVF Cutoff Control %02Xh (%d cents)",            _Iter[3], Map((int) _Iter[3], 0x00, 0x7F, -9600, 9600)); break;
                        case 0x402032: Description = ::FormatText("PAf Amplitude Control %02Xh (%.1f%%)",               _Iter[3], Map((int) _Iter[3], 0x00, 0x7F, -100.f, 100.f)); break;

                        case 0x402033: Description = ::FormatText("PAf LFO1 Rate Control %02Xh (%.1fHz)",               _Iter[3], Map((int) _Iter[3], 0x00, 0x7F, -10.f, 10.f)); break;
                        case 0x402034: Description = ::FormatText("PAf LFO1 Pitch Depth %02Xh (%d cents)",              _Iter[3], Map((int) _Iter[3], 0x00, 0x7F, 0, 600)); break;
                        case 0x402035: Description = ::FormatText("PAf LFO1 TVF Depth %02Xh (%d cents)",                _Iter[3], Map((int) _Iter[3], 0x28, 0x58, 0, 2400)); break;
                        case 0x402036: Description = ::FormatText("PAf LFO1 TVA Depth %02Xh (%.1f%%)",                  _Iter[3], Map((int) _Iter[3], 0x28, 0x58, 0.f, 100.f)); break;

                        case 0x402037: Description = ::FormatText("PAf LFO2 Rate Control %02Xh (%.1fHz)",               _Iter[3], Map((int) _Iter[3], 0x00, 0x7F, -10.f, 10.f)); break;
                        case 0x402038: Description = ::FormatText("PAf LFO2 Pitch Depth %02Xh (%d cents)",              _Iter[3], Map((int) _Iter[3], 0x00, 0x7F, 0, 600)); break;
                        case 0x402039: Description = ::FormatText("PAf LFO2 TVF Depth %02Xh (%d cents)",                _Iter[3], Map((int) _Iter[3], 0x28, 0x58, 0, 2400)); break;
                        case 0x40203A: Description = ::FormatText("PAf LFO2 TVA Depth %02Xh (%.1f%%)",                  _Iter[3], Map((int) _Iter[3], 0x28, 0x58, 0.f, 100.f)); break;

                        case 0x402040: Description = ::FormatText("CC1 Pitch Control %02Xh (%d semitones)",             _Iter[3], Map((int) _Iter[3], 0x28, 0x58, -24, 24)); break;
                        case 0x402041: Description = ::FormatText("CC1 TVF Cutoff Control %02Xh (%d cents)",            _Iter[3], Map((int) _Iter[3], 0x00, 0x7F, -9600, 9600)); break;
                        case 0x402042: Description = ::FormatText("CC1 Amplitude Control %02Xh (%.1f%%)",               _Iter[3], Map((int) _Iter[3], 0x00, 0x7F, -100.f, 100.f)); break;

                        case 0x402043: Description = ::FormatText("CC1 LFO1 Rate Control %02Xh (%.1fHz)",               _Iter[3], Map((int) _Iter[3], 0x00, 0x7F, -10.f, 10.f)); break;
                        case 0x402044: Description = ::FormatText("CC1 LFO1 Pitch Depth %02Xh (%d cents)",              _Iter[3], Map((int) _Iter[3], 0x00, 0x7F, 0, 600)); break;
                        case 0x402045: Description = ::FormatText("CC1 LFO1 TVF Depth %02Xh (%d cents)",                _Iter[3], Map((int) _Iter[3], 0x28, 0x58, 0, 2400)); break;
                        case 0x402046: Description = ::FormatText("CC1 LFO1 TVA Depth %02Xh (%.1f%%)",                  _Iter[3], Map((int) _Iter[3], 0x28, 0x58, 0.f, 100.f)); break;

                        case 0x402047: Description = ::FormatText("CC1 LFO2 Rate Control %02Xh (%.1fHz)",               _Iter[3], Map((int) _Iter[3], 0x00, 0x7F, -10.f, 10.f)); break;
                        case 0x402048: Description = ::FormatText("CC1 LFO2 Pitch Depth %02Xh (%d cents)",              _Iter[3], Map((int) _Iter[3], 0x00, 0x7F, 0, 600)); break;
                        case 0x402049: Description = ::FormatText("CC1 LFO2 TVF Depth %02Xh (%d cents)",                _Iter[3], Map((int) _Iter[3], 0x28, 0x58, 0, 2400)); break;
                        case 0x40204A: Description = ::FormatText("CC1 LFO2 TVA Depth %02Xh (%.1f%%)",                  _Iter[3], Map((int) _Iter[3], 0x28, 0x58, 0.f, 100.f)); break;

                        case 0x402050: Description = ::FormatText("CC2 Pitch Control %02Xh (%d semitones)",             _Iter[3], Map((int) _Iter[3], 0x28, 0x58, -24, 24)); break;
                        case 0x402051: Description = ::FormatText("CC2 TVF Cutoff Control %02Xh (%d cents)",            _Iter[3], Map((int) _Iter[3], 0x00, 0x7F, -9600, 9600)); break;
                        case 0x402052: Description = ::FormatText("CC2 Amplitude Control %02Xh (%.1f%%)",               _Iter[3], Map((int) _Iter[3], 0x00, 0x7F, -100.f, 100.f)); break;

                        case 0x402053: Description = ::FormatText("CC2 LFO1 Rate Control %02Xh (%.1fHz)",               _Iter[3], Map((int) _Iter[3], 0x00, 0x7F, -10.f, 10.f)); break;
                        case 0x402054: Description = ::FormatText("CC2 LFO1 Pitch Depth %02Xh (%d cents)",              _Iter[3], Map((int) _Iter[3], 0x00, 0x7F, 0, 600)); break;
                        case 0x402055: Description = ::FormatText("CC2 LFO1 TVF Depth %02Xh (%d cents)",                _Iter[3], Map((int) _Iter[3], 0x28, 0x58, 0, 2400)); break;
                        case 0x402056: Description = ::FormatText("CC2 LFO1 TVA Depth %02Xh (%.1f%%)",                  _Iter[3], Map((int) _Iter[3], 0x28, 0x58, 0.f, 100.f)); break;

                        case 0x402057: Description = ::FormatText("CC2 LFO2 Rate Control %02Xh (%.1fHz)",               _Iter[3], Map((int) _Iter[3], 0x00, 0x7F, -10.f, 10.f)); break;
                        case 0x402058: Description = ::FormatText("CC2 LFO2 Pitch Depth %02Xh (%d cents)",              _Iter[3], Map((int) _Iter[3], 0x00, 0x7F, 0, 600)); break;
                        case 0x402059: Description = ::FormatText("CC2 LFO2 TVF Depth %02Xh (%d cents)",                _Iter[3], Map((int) _Iter[3], 0x28, 0x58, 0, 2400)); break;
                        case 0x40205A: Description = ::FormatText("CC2 LFO2 TVA Depth %02Xh (%.1f%%)",                  _Iter[3], Map((int) _Iter[3], 0x28, 0x58, 0.f, 100.f)); break;

                        case 0x404000: Description = ::FormatText("Tone Map Number %02Xh (%s)",                         _Iter[3], IdentifyGSToneMap(_Iter[3])); break;
                        case 0x404001: Description = ::FormatText("Tone Map-0 Number %02Xh (%s)",                       _Iter[3], IdentifyGSToneMap(_Iter[3])); break;

                        case 0x404020: Description = ::FormatText("Eq %02Xh (%s)",                                      _Iter[3], (_Iter[3] == 0x01 ? "On" : "Off")); break;
                        case 0x404021: Description = ::FormatText("Output Assign %02Xh (%d)",                           _Iter[3], (int) _Iter[3]); break;
                        case 0x404022: Description = ::FormatText("Part EFX Assign %02Xh (%s)",                         _Iter[3], (_Iter[3] == 0x00 ? "Bypass" : "EFX")); break;

                        case 0x404023: Description = ::FormatText("%06X <Undocumented>", Address); break;
                        case 0x404024: Description = ::FormatText("%06X <Undocumented>", Address); break;
                        case 0x404025: Description = ::FormatText("%06X <Undocumented>", Address); break;
                        case 0x404026: Description = ::FormatText("%06X <Undocumented>", Address); break;
                        case 0x404027: Description = ::FormatText("%06X <Undocumented>", Address); break;
                        case 0x404028: Description = ::FormatText("%06X <Undocumented>", Address); break;
                        case 0x404029: Description = ::FormatText("%06X <Undocumented>", Address); break;
                        case 0x40402A: Description = ::FormatText("%06X <Undocumented>", Address); break;
                        case 0x40402B: Description = ::FormatText("%06X <Undocumented>", Address); break;
                        case 0x40402C: Description = ::FormatText("%06X <Undocumented>", Address); break;
                        case 0x40402D: Description = ::FormatText("%06X <Undocumented>", Address); break;
                        case 0x40402E: Description = ::FormatText("%06X <Undocumented>", Address); break;
                        case 0x40402F: Description = ::FormatText("%06X <Undocumented>", Address); break;
                    }
                }
            }
            break;
        }

        // Drum Setup Parameters
        case 0x41:
        {
            if (Address < 0x420000)
            {
                if ((_Iter[1] == 0x00 || _Iter[1] == 0x10) && (_Iter[2] < 0x0C))
                    Description = ::FormatText("Drum Map Name");
                else
                if ((_Iter[1] == 0x01 || _Iter[1] == 0x11))
                    Description = ::FormatText("Drum Play Note Number");
                else
                if ((_Iter[1] == 0x02 || _Iter[1] == 0x12))
                    Description = ::FormatText("Drum Level");
                else
                if ((_Iter[1] == 0x03 || _Iter[1] == 0x13))
                    Description = ::FormatText("Drum Assign Group Number");
                else
                if ((_Iter[1] == 0x04 || _Iter[1] == 0x14))
                    Description = ::FormatText("Drum Pan Pot");
                else
                if ((_Iter[1] == 0x05 || _Iter[1] == 0x15))
                    Description = ::FormatText("Drum Reverb Send Level");
                else
                if ((_Iter[1] == 0x06 || _Iter[1] == 0x16))
                    Description = ::FormatText("Drum Chorus Send Level");
                else
                if ((_Iter[1] == 0x07 || _Iter[1] == 0x17))
                    Description = ::FormatText("Drum Rx. Note Off");
                else
                if ((_Iter[1] == 0x08 || _Iter[1] == 0x18))
                    Description = ::FormatText("Drum Rx. Note On");
                else
                if ((_Iter[1] == 0x09 || _Iter[1] == 0x19))
                    Description = ::FormatText("Drum Delay Send Level");
            }
            break;
        }

        // Patch Common Parameters B. Parameters common to all Parts in each module (Block B 10-1F)
        case 0x50:
        {
            break;
        }

        default:
            Description = ::FormatText("Unknown address %06Xh", Address);
    }

//  VerifyChecksum(_Iter[3]);
}

void sysex_t::IdentifyYamaha() noexcept
{
    Manufacturer = "Yamaha";
}
