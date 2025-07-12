
/** $VER: SysEx.cpp (2025.06.21) **/

#include "pch.h"

#include "SysEx.h"

namespace midi
{

const uint8_t sysex_t::GMSystemOn[6] =
{
    0xF0, // Start of SysEx
    0x7E, 0x7F, 0x09, 0x01,
    0xF7  // End of SysEx
};

const uint8_t sysex_t::GMDisable[6] =
{
    0xF0, // Start of SysEx
    0x7E, 0x7F, 0x09, 0x02,
    0xF7  // End of SysEx
};

const uint8_t sysex_t::GM2Reset[6] =
{
    0xF0, // Start of SysEx
    0x7E, 0x7F, 0x09, 0x03,
    0xF7  // End of SysEx
};

const uint8_t sysex_t::D50Reset[10] =
{
    0xF0, // Start of SysEx
    0x41, // Manufacturer ID (Roland)
    0x10, // Device ID
    0x14, // Model ID (D50)
    0x12, // Command ID (Data Set 1, DT1)
    0x7F, // Address MSB
    0x00,
    0x00,
    0xF7  // End of SysEx
};

const uint8_t sysex_t::MT32Reset[10] =
{
    0xF0, // Start of SysEx
    0x41, // Manufacturer ID (Roland)
    0x10, // Device ID
    0x16, // Model ID (MT32)
    0x12, // Command ID (Data Set 1, DT1)
    0x7F, // Address MSB
    0x01, // Address LSB
    0xF7  // End of SysEx
};

const uint8_t sysex_t::GSReset[11] =
{
    0xF0, // Start of SysEx
    0x41, // Manufacturer ID (Roland)
    0x10, // Device ID
    0x42, // Model ID (GS)
    0x12, // Command ID (Data Set 1, DT1)
    0x40, // Address MSB
    0x00, // Address
    0x7F, // Address LSB
    0x00, // Data (GS Reset)
    0x41, // Checksum
    0xF7  // End of SysEx
};

const uint8_t sysex_t::GSToneMapNumber[11]  =
{
    0xF0, // Start of SysEx
    0x41, // Manufacturer ID (Roland)
    0x10, // Device ID
    0x42, // Model ID (GS)
    0x12, // Command ID (Data Set 1, DT1)
    0x40, // Tone Map Number?
    0x41, // Tone Map?
    0x00,
    0x03, // Channel?
    0x00, // Checksum
    0xF7  // End of SysEx
};

const uint8_t sysex_t::XGSystemOn[9] =
{
    0xF0, // Start of SysEx
    0x43, // Manufacturer ID (Yamaha)
    0x10, // Device ID
    0x4C, // Model ID
    0x00,
    0x00,
    0x7E,
    0x00,
    0xF7  // End of SysEx
};

const uint8_t sysex_t::XGReset[9] =
{
    0xF0, // Start of SysEx
    0x43, // Manufacturer ID (Yamaha)
    0x10, // Device ID
    0x4C, // Model ID
    0x00,
    0x00,
    0x7F,
    0x00,
    0xF7  // End of SysEx
};
/*
    XG Works On     F0 43 76 1A 10 01 01 01 01 01 01 01 01 F7
    Yamaha          F0 43 73 39 F1 00 46 00 F7
*/
}
