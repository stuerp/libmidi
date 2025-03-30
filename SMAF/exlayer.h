#pragma once

#define VOICE_FM    0
#define VOICE_PCM   1

#pragma pack(1)

struct CHPARAM
{
    BYTE bm;
    BYTE bl;
    BYTE pc;

    // Drum parameters
    BYTE na;    // NoteAssign
    BYTE dk;    // DrumKey

    BYTE lfo;
    BYTE pan;
    BOOL pe;
    BYTE alg;   // Only FM

    // Extended member
    UINT type;
//  BYTE version;

    // PCM
    DWORD fs;
    BOOL rm;
    BYTE wavno;
    DWORD lp;
    DWORD ep;
};

struct OPPARAM
{
    BYTE multi; // 15    FM only
    BYTE dt;    //  7    FM only
    BYTE ar;    // 15
    BYTE dr;    // 15
    BYTE sr;    // 15
    BYTE rr;    // 15
    BYTE sl;    // 15
    BYTE tl;    // 63
    BYTE ksl;   //  3   FM only
    BYTE dam;   //  3
    BYTE dvb;   //  3
    BYTE fb;    //  7   FM only
    BYTE ws;    // 31   FM only

    BOOL xof;
    BOOL sus;
    BOOL ksr;   //      FM only
    BOOL eam;
    BOOL evb;
};

#pragma pack()

bool GetHPSExclusiveFM(const uint8_t * data, CHPARAM * chp, OPPARAM * opp);
size_t setMA3Exclusive(uint8_t * data, CHPARAM * chp, OPPARAM * opp);

