
/** $VER: SysEx.h (2025.03.30) P. Stuer **/

#pragma once

#include <CppCoreCheck/Warnings.h>

#pragma warning(disable: 4625 4626 ALL_CPPCORECHECK_WARNINGS)

#include <stdint.h>

#pragma warning(disable: 4820) // x bytes padding

struct sysex_description_t
{
    const char * KeyData;
    uint32_t KeySize;
    const wchar_t * Description;
};

const sysex_description_t SysExMessages[]
{
    // Format: SysEx Status Code, SysEx Manufacturer Id, SysEx Channel, Sub-Id 1, Sub-Id 2, data, SysEx End Status Code

    // Universal Non-Real Time
    { "\xF0\x7E" "\x00" "\x06\x01" "\xF7",                               6, L"Identity Request"},
    { "\xF0\x7E" "\x00" "\x06\x02" "\xF7",                               6, L"Identity Reply"},

    // Universal Non-Real Time (General MIDI, 1991)
    { "\xF0\x7E" "\x00" "\x09\x01" "\xF7",                               6, L"GM1 System On"},
    { "\xF0\x7E" "\x00" "\x09\x02" "\xF7",                               6, L"GM1 System Off"},
    { "\xF0\x7E" "\x00" "\x09\x03" "\xF7",                               6, L"GM2 System On"},

    { "\xF0\x7E" "\x00" "\x0A\x01" "\xF7",                               6, L"DLS On"},                             // DLS Level 2.2, 1.16 DLS System Exclusive Messages
    { "\xF0\x7E" "\x00" "\x0A\x02" "\xF7",                               6, L"DLS Off"},                            // DLS Level 2.2, 1.16 DLS System Exclusive Messages
    { "\xF0\x7E" "\x00" "\x0A\x03" "\xF7",                               6, L"DLS Static Voice Allocation Off"},    // DLS Level 2.2, 1.16 DLS System Exclusive Messages
    { "\xF0\x7E" "\x00" "\x0A\x04" "\xF7",                               6, L"DLS Static Voice Allocation On"},     // DLS Level 2.2, 1.16 DLS System Exclusive Messages

    // Universal Real Time
    { "\xF0\x7F" "\x00" "\x03\x01",                                      5, L"Bar Number" },
    { "\xF0\x7F" "\x00" "\x03\x02",                                      5, L"Time Signature (Immediate)" },
    { "\xF0\x7F" "\x00" "\x03\x03",                                      5, L"Time Signature (Delayed)" },

    { "\xF0\x7F" "\x00" "\x04\x01",                                      5, L"Master Volume" },
    { "\xF0\x7F" "\x00" "\x04\x02",                                      5, L"Master Balance" },
    { "\xF0\x7F" "\x00" "\x04\x03",                                      5, L"Master Fine Tune" },
    { "\xF0\x7F" "\x00" "\x04\x04",                                      5, L"Master Coarse Tune" },

    // Universal Real Time (General MIDI Level 2, 1999)
    { "\xF0\x7F" "\x00" "\x04\x05" "\x01\x01\x01\x01\x01",              10, L"Global Reverb Parameters" },
    { "\xF0\x7F" "\x00" "\x04\x05" "\x01\x01\x01\x01\x02",              10, L"Global Chorus Parameters" },

    { "\xF0\x7F" "\x00" "\x05",                                          4, L"Real-Time MTC Cueing Command" },
    { "\xF0\x7F" "\x00" "\x06",                                          4, L"MIDI Machine Control Command" },
    { "\xF0\x7F" "\x00" "\x07",                                          4, L"MIDI Machine Control Response" },

    { "\xF0\x7F" "\x00" "\x08\x02",                                      5, L"Single Note Tuning Change" },
    { "\xF0\x7F" "\x00" "\x08\x07",                                      5, L"Single Note Tuning Change with Bank Select" },
    { "\xF0\x7F" "\x00" "\x08\x08",                                      5, L"Scale/Octave Tuning Adjust" },
    { "\xF0\x7F" "\x00" "\x08\x09",                                      5, L"Scale/Octave Tuning Adjust (2-byte format)" },
    { "\xF0\x7F" "\x00" "\x09\x01",                                      5, L"Channel Pressure (Aftertouch)" },
    { "\xF0\x7F" "\x00" "\x09\x03",                                      5, L"Controller (Control Change)" },
    { "\xF0\x7F" "\x00" "\x0A\x01",                                      5, L"Key-based Instrument Controller" },
    { "\xF0\x7F" "\x00" "\x0B\x01",                                      5, L"Scalable Polyphony MIDI MIP Message" },
    { "\xF0\x7F" "\x00" "\x0C\x00",                                      5, L"Mobile Phone Control Message" },

    // Roland MC505 / JX305 / D2
    { "\xF0\x41" "\x00" "\x0B",                                          4, L"Roland MC505/JX305/D2-specific" },

    // Roland S-10
    { "\xF0\x41" "\x00" "\x10",                                          4, L"Roland S-10-specific" },

    // Roland MT-32
    { "\xF0\x41" "\x00" "\x16",                                          4, L"Roland MT-32-specific" },

    // Roland GS, SC-88, SC-88Pro, Command ID = 0x12 (DT1, Data Set 1), Address (3-bytes), Data Value, Checksum, 0xF7 (SC-88 Manual, Section 3. Individual Parameter Transmission)
//  { "\xF0\x41" "\x00" "\x42",                                          4, L"Roland SC-88-specific" },

    // Roland SC-55, SC-155
    { "\xF0\x41" "\x00" "\x45",                                          4, L"Roland SC-55/SC-155-specific" },

    // Roland SC-7
    { "\xF0\x41" "\x00" "\x56",                                          4, L"Roland SC-7-specific" },

    // Roland JD-990
    { "\xF0\x41" "\x00" "\x57",                                          4, L"Roland JD-990-specific" },

    // Roland JV-1010
    { "\xF0\x41" "\x00" "\x6A",                                          4, L"Roland JV-1010-specific" },

    // Roland Fantom XR
    { "\xF0\x41" "\x00" "\x6B",                                          4, L"Roland Fantom XR-specific" },

    // Roland VS-880
    { "\xF0\x41" "\x00" "\x7C",                                          4, L"Roland VS-880-specific" },

    // Roland GR-30
    { "\xF0\x41" "\x00" "\x00\x07",                                      5, L"Roland GR-30-specific" },

    // Roland TD-6
    { "\xF0\x41" "\x00" "\x00\x3F",                                      5, L"Roland TD-6-specific" },

    // Roland VG-99 / JP-08
    { "\xF0\x41" "\x00" "\x00\x00\x1C",                                  6, L"Roland VG-99-specific" },

    // Roland Integra-7
    { "\xF0\x41" "\x00" "\x00\x00\x3F",                                  6, L"Roland Integra-7-specific" },

    // Roland TR-8S
    { "\xF0\x41" "\x00" "\x00\x00\x00\x45",                              7, L"Roland TR-8S-specific" },

    // Roland Jupiter X
    { "\xF0\x41" "\x00" "\x00\x00\x00\x65",                              7, L"Roland Jupiter X-specific" },

    // Roland MC-707
    { "\xF0\x41" "\x00" "\x00\x00\x00\xC0",                              7, L"Roland MC-707-specific" },

    // Roland Verselab MV-1
    { "\xF0\x41" "\x00" "\x00\x00\x00\x00\xCF",                          8, L"Roland Verselab MV-1-specific" },

/* Roland checksum
    1. Add the values together, but if the answer to any sum exceeds 127 then subtract 128.
    2. Subtract the final answer from 128.

    F0 41 10 42 12 (40 11 00 41 63) 0B F7

        64 + 17 =  81
        81 +  0 =  81
        81 + 65 = 146
            (146 - 128 = 18)
        18 + 99 = 117

        128 - 117 = 11 <- Checksum
*/
    // Yamaha, Device 0x79
    { "\xF0\x43" "\x00" "\x05\x7E\x09",                                     6, L"SMW-5 Volume" },           // MMF/SMAF MMFTool
    { "\xF0\x43" "\x00" "\x06\x7F\x04",                                     6, L"SMW-5 Detach Wave" },      // MMF/SMAF MMFTool
    { "\xF0\x43" "\x00" "\x06\x7F\x7F\xF7",                                 7, L"SMW-5 Reset" },            // MMF/SMAF MMFTool

    // Yamaha
    { "\xF0\x43" "\x00" "\x1A" "\x10\x01\x01\x01\x01\x01\x01\x01\x01\xF7", 14, L"XG Works On" },

    { "\xF0\x43" "\x00" "\x49",                                             4, L"Yamaha MU80-specific" },

    // Yamaha, \x4C = XG
    { "\xF0\x43" "\x00" "\x4C" "\x00\x00\x00",                              7, L"XG Master Tune" },
    { "\xF0\x43" "\x00" "\x4C" "\x00\x00\x04",                              7, L"XG Master Volume" },
    { "\xF0\x43" "\x00" "\x4C" "\x00\x00\x05",                              7, L"XG Master Attenuator" },
    { "\xF0\x43" "\x00" "\x4C" "\x00\x00\x06",                              7, L"XG Master Transpose" },

    { "\xF0\x43" "\x00" "\x4C" "\x00\x00\x7D" "\x01\xF7",                   9, L"XG Drum Setup 1 Reset" },
    { "\xF0\x43" "\x00" "\x4C" "\x00\x00\x7D" "\x02\xF7",                   9, L"XG Drum Setup 2 Reset" },
    { "\xF0\x43" "\x00" "\x4C" "\x00\x00\x7E" "\x00\xF7",                   9, L"XG System On" },
    { "\xF0\x43" "\x00" "\x4C" "\x00\x00\x7F" "\x00\xF7",                   9, L"XG Reset" },

    { "\xF0\x43" "\x00" "\x4C" "\x02\x01\x00",                              7, L"XG Reverb Type" },
    { "\xF0\x43" "\x00" "\x4C" "\x02\x01\x02",                              7, L"XG Reverb Parameter 1" },
    { "\xF0\x43" "\x00" "\x4C" "\x02\x01\x03",                              7, L"XG Reverb Parameter 2" },
    { "\xF0\x43" "\x00" "\x4C" "\x02\x01\x04",                              7, L"XG Reverb Parameter 3" },
    { "\xF0\x43" "\x00" "\x4C" "\x02\x01\x05",                              7, L"XG Reverb Parameter 4" },
    { "\xF0\x43" "\x00" "\x4C" "\x02\x01\x06",                              7, L"XG Reverb Parameter 5" },
    { "\xF0\x43" "\x00" "\x4C" "\x02\x01\x07",                              7, L"XG Reverb Parameter 6" },
    { "\xF0\x43" "\x00" "\x4C" "\x02\x01\x08",                              7, L"XG Reverb Parameter 7" },
    { "\xF0\x43" "\x00" "\x4C" "\x02\x01\x09",                              7, L"XG Reverb Parameter 8" },
    { "\xF0\x43" "\x00" "\x4C" "\x02\x01\x0A",                              7, L"XG Reverb Parameter 9" },
    { "\xF0\x43" "\x00" "\x4C" "\x02\x01\x0B",                              7, L"XG Reverb Parameter 10" },
    { "\xF0\x43" "\x00" "\x4C" "\x02\x01\x0C",                              7, L"XG Reverb Return" },
    { "\xF0\x43" "\x00" "\x4C" "\x02\x01\x0D",                              7, L"XG Reverb Pan" },

    { "\xF0\x43" "\x00" "\x4C" "\x02\x01\x20",                              7, L"XG Chorus Type" },
    { "\xF0\x43" "\x00" "\x4C" "\x02\x01\x22",                              7, L"XG Chorus Parameter 1" },
    { "\xF0\x43" "\x00" "\x4C" "\x02\x01\x23",                              7, L"XG Chorus Parameter 2" },
    { "\xF0\x43" "\x00" "\x4C" "\x02\x01\x24",                              7, L"XG Chorus Parameter 3" },
    { "\xF0\x43" "\x00" "\x4C" "\x02\x01\x25",                              7, L"XG Chorus Parameter 4" },
    { "\xF0\x43" "\x00" "\x4C" "\x02\x01\x26",                              7, L"XG Chorus Parameter 5" },
    { "\xF0\x43" "\x00" "\x4C" "\x02\x01\x27",                              7, L"XG Chorus Parameter 6" },
    { "\xF0\x43" "\x00" "\x4C" "\x02\x01\x28",                              7, L"XG Chorus Parameter 7" },
    { "\xF0\x43" "\x00" "\x4C" "\x02\x01\x29",                              7, L"XG Chorus Parameter 8" },
    { "\xF0\x43" "\x00" "\x4C" "\x02\x01\x2A",                              7, L"XG Chorus Parameter 9" },
    { "\xF0\x43" "\x00" "\x4C" "\x02\x01\x2B",                              7, L"XG Chorus Parameter 10" },
    { "\xF0\x43" "\x00" "\x4C" "\x02\x01\x2C",                              7, L"XG Chorus Return" },
    { "\xF0\x43" "\x00" "\x4C" "\x02\x01\x2D",                              7, L"XG Chorus Pan" },
    { "\xF0\x43" "\x00" "\x4C" "\x02\x01\x2E",                              7, L"XG Send Chorus to Reverb" },

    { "\xF0\x43" "\x00" "\x4C" "\x02\x01\x30",                              7, L"XG Chorus Parameter 11" },
    { "\xF0\x43" "\x00" "\x4C" "\x02\x01\x31",                              7, L"XG Chorus Parameter 12" },
    { "\xF0\x43" "\x00" "\x4C" "\x02\x01\x32",                              7, L"XG Chorus Parameter 13" },
    { "\xF0\x43" "\x00" "\x4C" "\x02\x01\x33",                              7, L"XG Chorus Parameter 14" },
    { "\xF0\x43" "\x00" "\x4C" "\x02\x01\x34",                              7, L"XG Chorus Parameter 15" },
    { "\xF0\x43" "\x00" "\x4C" "\x02\x01\x35",                              7, L"XG Chorus Parameter 16" },

    { "\xF0\x43" "\x00" "\x4C" "\x02\x01\x40",                              7, L"XG Variation Type" },
    { "\xF0\x43" "\x00" "\x4C" "\x02\x01\x42",                              7, L"XG Variation Parameter 1" },
    { "\xF0\x43" "\x00" "\x4C" "\x02\x01\x44",                              7, L"XG Variation Parameter 2" },
    { "\xF0\x43" "\x00" "\x4C" "\x02\x01\x46",                              7, L"XG Variation Parameter 3" },
    { "\xF0\x43" "\x00" "\x4C" "\x02\x01\x48",                              7, L"XG Variation Parameter 4" },
    { "\xF0\x43" "\x00" "\x4C" "\x02\x01\x4A",                              7, L"XG Variation Parameter 5" },
    { "\xF0\x43" "\x00" "\x4C" "\x02\x01\x4C",                              7, L"XG Variation Parameter 6" },
    { "\xF0\x43" "\x00" "\x4C" "\x02\x01\x4E",                              7, L"XG Variation Parameter 7" },
    { "\xF0\x43" "\x00" "\x4C" "\x02\x01\x50",                              7, L"XG Variation Parameter 8" },
    { "\xF0\x43" "\x00" "\x4C" "\x02\x01\x52",                              7, L"XG Variation Parameter 9" },
    { "\xF0\x43" "\x00" "\x4C" "\x02\x01\x54",                              7, L"XG Variation Parameter 10" },
    { "\xF0\x43" "\x00" "\x4C" "\x02\x01\x56",                              7, L"XG Variation Return" },
    { "\xF0\x43" "\x00" "\x4C" "\x02\x01\x57",                              7, L"XG Variation Pan" },
    { "\xF0\x43" "\x00" "\x4C" "\x02\x01\x58",                              7, L"XG Send Variation to Reverb" },
    { "\xF0\x43" "\x00" "\x4C" "\x02\x01\x59",                              7, L"XG Send Variation to Chorus" },
    { "\xF0\x43" "\x00" "\x4C" "\x02\x01\x5A",                              7, L"XG Variation Connection" },
    { "\xF0\x43" "\x00" "\x4C" "\x02\x01\x5B",                              7, L"XG Variation Part Number" },
    { "\xF0\x43" "\x00" "\x4C" "\x02\x01\x5C",                              7, L"XG MW Variation Control Depth" },
    { "\xF0\x43" "\x00" "\x4C" "\x02\x01\x5D",                              7, L"XG Bend Variation Control Depth" },
    { "\xF0\x43" "\x00" "\x4C" "\x02\x01\x5E",                              7, L"XG CAT Variation Control Depth" },
    { "\xF0\x43" "\x00" "\x4C" "\x02\x01\x5F",                              7, L"XG AC1 Variation Control Depth" },

    { "\xF0\x43" "\x00" "\x4C" "\x02\x01\x60",                              7, L"XG AC2 Variation Control Depth" },

    { "\xF0\x43" "\x00" "\x4C" "\x02\x01\x70",                              7, L"XG Variation Parameter 11" },
    { "\xF0\x43" "\x00" "\x4C" "\x02\x01\x71",                              7, L"XG Variation Parameter 12" },
    { "\xF0\x43" "\x00" "\x4C" "\x02\x01\x72",                              7, L"XG Variation Parameter 13" },
    { "\xF0\x43" "\x00" "\x4C" "\x02\x01\x73",                              7, L"XG Variation Parameter 14" },
    { "\xF0\x43" "\x00" "\x4C" "\x02\x01\x74",                              7, L"XG Variation Parameter 15" },
    { "\xF0\x43" "\x00" "\x4C" "\x02\x01\x75",                              7, L"XG Variation Parameter 16" },

    { "\xF0\x43" "\x00" "\x4C" "\x02\x40\x00",                              7, L"XG Multi EQ" },

    { "\xF0\x43" "\x00" "\x4C" "\x03\x00\x00",                              7, L"XG Insertion Effect 1 Type" },
    { "\xF0\x43" "\x00" "\x4C" "\x03\x01\x00",                              7, L"XG Insertion Effect 2 Type" },
    { "\xF0\x43" "\x00" "\x4C" "\x03\x02\x00",                              7, L"XG Insertion Effect 3 Type" },
    { "\xF0\x43" "\x00" "\x4C" "\x03\x03\x00",                              7, L"XG Insertion Effect 4 Type" },

    { "\xF0\x43" "\x00" "\x4C" "\x06\x00\x00",                              7, L"XG Display Letter" },
    { "\xF0\x43" "\x00" "\x4C" "\x07\x00\x00",                              7, L"XG Display Bit Map" },

    { "\xF0\x43" "\x00" "\x4C" "\x08\x00",                                  6, L"XG Multi Part 1" },
    { "\xF0\x43" "\x00" "\x4C" "\x08\x01",                                  6, L"XG Multi Part 2" },
    { "\xF0\x43" "\x00" "\x4C" "\x08\x04",                                  6, L"XG Multi Part 5" },

    { "\xF0\x43" "\x00" "\x4C" "\x30\x0D",                                  6, L"XG Drum Setup 1" },
    { "\xF0\x43" "\x00" "\x4C" "\x31\x0D",                                  6, L"XG Drum Setup 2" },
    { "\xF0\x43" "\x00" "\x4C" "\x32\x0D",                                  6, L"XG Drum Setup 3" },
    { "\xF0\x43" "\x00" "\x4C" "\x33\x0D",                                  6, L"XG Drum Setup 4" },

//  { "\xF0\x43" "\x00" "\x4C" "\x08\x09" "\x11\x64\xF7", , L"" ),

    { "\xF0\x43" "\x00" "\x59",                                             4, L"Yamaha MU90-specific" },

    { "\xF0\x43" "\x00" "\x7A",                                             4, L"Yamaha SW60XG-specific" },

    { "\xF0\x43" "\x00" "\x7F\x17",                                         5, L"Yamaha MX88-specific" },
    { "\xF0\x43" "\x00" "\x7F\x1C\x02",                                     6, L"Yamaha Montage-specific" },
};

class sysex_t
{
public:
    sysex_t(const std::vector<uint8_t> & data)
    {
        _Data = data;
    }

    sysex_t(const uint8_t * data, size_t size)
    {
        _Data.resize(size);
        ::memcpy(_Data.data(), data, size);
    }

    void Identify();

    std::wstring GetManufacturer() { return _Manufacturer; }
    std::wstring GetDescription() { return _Description; }

private:
    void IdentifyGSMessage();

private:
    std::vector<uint8_t> _Data;

    std::wstring _Manufacturer;
    std::wstring _Description;
};
