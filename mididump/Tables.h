
/** $VER: Tables.h (2025.03.02) P. Stuer **/

#pragma once

#include <CppCoreCheck/Warnings.h>

#include <stdint.h>

#pragma warning(disable: 4625 4626 ALL_CPPCORECHECK_WARNINGS)

#pragma warning(disable: 4820) // x bytes padding
struct instrument_description_t
{
    uint8_t Id;
    const wchar_t * Name;
};
#pragma warning(default: 4820) // x bytes padding

// General MIDI Level 1 Instrument Families
const instrument_description_t Instruments[]
{
    //   1 -  8 Piano
    {   1, L"Acoustic Grand Piano" },
    {   2, L"Bright Acoustic Piano" },
    {   3, L"Electric Grand Piano" },
    {   4, L"Honky-tonk Piano" },
    {   5, L"Electric Piano 1" },
    {   6, L"Electric Piano 2" },
    {   7, L"Harpsichord" },
    {   8, L"Clavi" },

    //   9 - 16 Chromatic Percussion
    {   9, L"Celesta" },
    {  10, L"Glockenspiel" },
    {  11, L"Music Box" },
    {  12, L"Vibraphone" },
    {  13, L"Marimba" },
    {  14, L"Xylophone" },
    {  15, L"Tubular Bells" },
    {  16, L"Dulcimer" },

    //  17 - 24 Organ
    {  17, L"Drawbar Organ" },
    {  18, L"Percussive Organ" },
    {  19, L"Rock Organ" },
    {  20, L"Church Organ" },
    {  21, L"Reed Organ" },
    {  22, L"Accordion" },
    {  23, L"Harmonica" },
    {  24, L"Tango Accordion" },

    //  25 - 32 Guitar
    {  25, L"Acoustic Guitar (nylon)" },
    {  26, L"Acoustic Guitar (steel)" },
    {  27, L"Electric Guitar (jazz)" },
    {  28, L"Electric Guitar (clean)" },
    {  29, L"Electric Guitar (muted)" },
    {  30, L"Overdriven Guitar" },
    {  31, L"Distortion Guitar" },
    {  32, L"Guitar harmonics" },

    //  33 - 40 Bass
    {  33, L"Acoustic Bass" },
    {  34, L"Electric Bass (finger)" },
    {  35, L"Electric Bass (pick)" },
    {  36, L"Fretless Bass" },
    {  37, L"Slap Bass 1" },
    {  38, L"Slap Bass 2" },
    {  39, L"Synth Bass 1" },
    {  40, L"Synth Bass 2" },

    //  41 - 48 Strings
    {  41, L"Violin" },
    {  42, L"Viola" },
    {  43, L"Cello" },
    {  44, L"Contrabass" },
    {  45, L"Tremolo Strings" },
    {  46, L"Pizzicato Strings" },
    {  47, L"Orchestral Harp" },
    {  48, L"Timpani" },

    //  49 - 56 Ensemble
    {  49, L"String Ensemble 1" },
    {  50, L"String Ensemble 2" },
    {  51, L"SynthStrings 1" },
    {  52, L"SynthStrings 2" },
    {  53, L"Choir Aahs" },
    {  54, L"Voice Oohs" },
    {  55, L"Synth Voice" },
    {  56, L"Orchestra Hit" },

    //  57 - 64 Brass
    {  57, L"Trumpet" },
    {  58, L"Trombone" },
    {  59, L"Tuba" },
    {  60, L"Muted Trumpet" },
    {  61, L"French Horn" },
    {  62, L"Brass Section" },
    {  63, L"SynthBrass 1" },
    {  64, L"SynthBrass 2" },

    //  65 - 72 Reed
    {  65, L"Soprano Sax" },
    {  66, L"Alto Sax" },
    {  67, L"Tenor Sax" },
    {  68, L"Baritone Sax" },
    {  69, L"Oboe" },
    {  70, L"English Horn" },
    {  71, L"Bassoon" },
    {  72, L"Clarinet" },

    //  73 - 80 Pipe
    {  73, L"Piccolo" },
    {  74, L"Flute" },
    {  75, L"Recorder" },
    {  76, L"Pan Flute" },
    {  77, L"Blown Bottle" },
    {  78, L"Shakuhachi" },
    {  79, L"Whistle" },
    {  80, L"Ocarina" },

    //  81 - 88 Synth Lead
    {  81, L"Lead 1 (square)" },
    {  82, L"Lead 2 (sawtooth)" },
    {  83, L"Lead 3 (calliope)" },
    {  84, L"Lead 4 (chiff)" },
    {  85, L"Lead 5 (charang)" },
    {  86, L"Lead 6 (voice)" },
    {  87, L"Lead 7 (fifths)" },
    {  88, L"Lead 8 (bass + lead)" },

    //  89 - 96 Synth Pad
    {  89, L"Pad 1 (new age)" },
    {  90, L"Pad 2 (warm)" },
    {  91, L"Pad 3 (polysynth)" },
    {  92, L"Pad 4 (choir)" },
    {  93, L"Pad 5 (bowed)" },
    {  94, L"Pad 6 (metallic)" },
    {  95, L"Pad 7 (halo)" },
    {  96, L"Pad 8 (sweep)" },

    //  97 - 104 Synth Effects
    {  97, L"FX 1 (rain)" },
    {  98, L"FX 2 (soundtrack)" },
    {  99, L"FX 3 (crystal)" },
    { 100, L"FX 4 (atmosphere)" },
    { 101, L"FX 5 (brightness)" },
    { 102, L"FX 6 (goblins)" },
    { 103, L"FX 7 (echoes)" },
    { 104, L"FX 8 (sci-fi)" },

    // 105 - 112 Ethnic
    { 105, L"Sitar" },
    { 106, L"Banjo" },
    { 107, L"Shamisen" },
    { 108, L"Koto" },
    { 109, L"Kalimba" },
    { 110, L"Bag pipe" },
    { 111, L"Fiddle" },
    { 112, L"Shanai" },

    // 113 - 120 Percussive
    { 113, L"Tinkle Bell" },
    { 114, L"Agogo" },
    { 115, L"Steel Drums" },
    { 116, L"Woodblock" },
    { 117, L"Taiko Drum" },
    { 118, L"Melodic Tom" },
    { 119, L"Synth Drum" },
    { 120, L"Reverse Cymbal" },

    // 121 - 128 Sound Effects
    { 121, L"Guitar Fret Noise" },
    { 122, L"Breath Noise" },
    { 123, L"Seashore" },
    { 124, L"Bird Tweet" },
    { 125, L"Telephone Ring" },
    { 126, L"Helicopter" },
    { 127, L"Applause" },
    { 128, L"Gunshot" },
};

#pragma warning(disable: 4820) // x bytes padding
struct percussion_description_t
{
    uint8_t Key;
    const wchar_t * Name;
};
#pragma warning(default: 4820) // x bytes padding

// General MIDI Level 1 Percussion Key Map
const percussion_description_t Percussions[]
{
    { 35, L"Acoustic Bass Drum" },
    { 36, L"Bass Drum 1" },
    { 37, L"Side Stick" },
    { 38, L"Acoustic Snare" },
    { 39, L"Hand Clap" },
    { 40, L"Electric Snare" },
    { 41, L"Low Floor Tom" },
    { 42, L"Closed Hi Hat" },
    { 43, L"High Floor Tom" },
    { 44, L"Pedal Hi-Hat" },
    { 45, L"Low Tom" },
    { 46, L"Open Hi-Hat" },
    { 47, L"Low-Mid Tom" },
    { 48, L"Hi-Mid Tom" },
    { 49, L"Crash Cymbal 1" },
    { 50, L"High Tom" },
    { 51, L"Ride Cymbal 1" },
    { 52, L"Chinese Cymbal" },
    { 53, L"Ride Bell" },
    { 54, L"Tambourine" },
    { 55, L"Splash Cymbal" },
    { 56, L"Cowbell" },
    { 57, L"Crash Cymbal 2" },
    { 58, L"Vibraslap" },
    { 59, L"Ride Cymbal 2" },
    { 60, L"Hi Bongo" },
    { 61, L"Low Bongo" },
    { 62, L"Mute Hi Conga" },
    { 63, L"Open Hi Conga" },
    { 64, L"Low Conga" },
    { 65, L"High Timbale" },
    { 66, L"Low Timbale" },
    { 67, L"High Agogo" },
    { 68, L"Low Agogo" },
    { 69, L"Cabasa" },
    { 70, L"Maracas" },
    { 71, L"Short Whistle" },
    { 72, L"Long Whistle" },
    { 73, L"Short Guiro" },
    { 74, L"Long Guiro" },
    { 75, L"Claves" },
    { 76, L"Hi Wood Block" },
    { 77, L"Low Wood Block" },
    { 78, L"Mute Cuica" },
    { 79, L"Open Cuica" },
    { 80, L"Mute Triangle" },
    { 81, L"Open Triangle" },
};

#pragma warning(disable: 4820) // x bytes padding
struct control_change_description_t
{
    uint8_t Id;
    const wchar_t * Name;
};
#pragma warning(default: 4820) // x bytes padding

const control_change_description_t ControlChangeMessages[] =
{
    {   0, L"Bank Select, MSB" },                       // Switches instrument banks for program selection.
    {   1, L"Modulation Wheel or Lever, MSB" },         // controls a vibrato effect (pitch, loudness, brighness). What is modulated depends on the program.
    {   2, L"Breath Controller, MSB" },                 // Often used with aftertouch messages. It was originally intended for use with a breath MIDI controller in which blowing harder produced higher MIDI control values. It can be used for modulation as well.
    {   3, L"Undefined" },
    {   4, L"Foot Controller, MSB" },                   // Often used with aftertouch messages. It can send a continuous stream of values based on how the pedal is used.
    {   5, L"Portamento Time, MSB" },                   // Controls portamento rate between 2 notes played after each other.
    {   6, L"Data Entry, MSB" },                        // Enters a value for NRPN or RPN parameters.
    {   7, L"Channel Volume, MSB" },                    // Sets the volume of the channel.
    {   8, L"Balance, MSB" },                           // Sets the left/right balance, usually for stereo patches. (0 = hard left, 64 = center, 127 = hard right)
    {   9, L"Undefined" },
    {  10, L"Pan, MSB" },                               // Sets the left/right balance, usually for mono patches. (0 = hard left, 64 = center, 127 = hard right)
    {  11, L"Expression Controller, MSB" },             // Expression is a percentage of the channel volume.
    {  12, L"Effect Control 1, MSB" },
    {  13, L"Effect Control 2, MSB" },
    {  14, L"Undefined" },
    {  15, L"Undefined" },
    {  16, L"General Purpose Controller 1, MSB" },
    {  17, L"General Purpose Controller 2, MSB" },
    {  18, L"General Purpose Controller 3, MSB" },
    {  19, L"General Purpose Controller 4, MSB" },

    {  20, L"Undefined" },
    {  21, L"Undefined" },
    {  22, L"Undefined" },
    {  23, L"Undefined" },
    {  24, L"Undefined" },
    {  25, L"Undefined" },
    {  26, L"Undefined" },
    {  27, L"Undefined" },
    {  28, L"Undefined" },
    {  29, L"Undefined" },
    {  30, L"Undefined" },
    {  31, L"Undefined" },

    {  32, L"Bank Select, LSB" },
    {  33, L"Modulation Wheel or Lever, LSB" },
    {  34, L"Breath Controller, LSB" },
    {  35, L"Undefined" },
    {  36, L"Foot Controller, LSB" },
    {  37, L"Portamento Time, LSB" },
    {  38, L"Data Entry, LSB" },
    {  39, L"Channel Volume, LSB" },
    {  40, L"Balance, LSB" },
    {  41, L"Undefined" },
    {  42, L"Pan, LSB" },
    {  43, L"Expression Controller, LSB" },
    {  44, L"Effect Control 1, LSB" },
    {  45, L"Effect Control 2, LSB" },
    {  46, L"Undefined" },
    {  47, L"Undefined" },
    {  48, L"General Purpose Controller 1, LSB" },
    {  49, L"General Purpose Controller 2, LSB" },
    {  50, L"General Purpose Controller 3, LSB" },
    {  51, L"General Purpose Controller 4, LSB" },

    {  52, L"Undefined" },
    {  53, L"Undefined" },
    {  54, L"Undefined" },
    {  55, L"Undefined" },
    {  56, L"Undefined" },
    {  57, L"Undefined" },
    {  58, L"Undefined" },
    {  59, L"Undefined" },
    {  60, L"Undefined" },
    {  61, L"Undefined" },
    {  62, L"Undefined" },
    {  63, L"Undefined" },

    {  64, L"Damper Pedal On/Off (Sustain)" },                              // On/Off switch to control sustain. See also Sostenuto CC 66. (0 to 63 = Off, 64 to 127 = On)
    {  65, L"Portamento On/Off" },                                          // On/Off switch (0 to 63 = Off, 64 to 127 = On)
    {  66, L"Sostenuto On/Off" },                                           // On/Off switch to hold only notes that were “On” when the pedal was pressed. (0 to 63 = Off, 64 to 127 = On)
    {  67, L"Soft Pedal On/Off" },                                          // On/Off switch to lower the volume of playing note. (0 to 63 = Off, 64 to 127 = On)
    {  68, L"Legato Footswitch" },                                          // On/Off switch to turn legato effect between 2 notes On or Off. (0 to 63 = Off, 64 to 127 = On)
    {  69, L"Hold 2" },                                                     // Another way to “hold notes” (see MIDI CC 64 and MIDI CC 66). However notes fade out according to their release parameter rather than when the pedal is released.

    {  70, L"Sound Controller 1 (default: Sound Variation), LSB" },
    {  71, L"Sound Controller 2 (default: Timbre/Harmonic Intens.), LSB" },
    {  72, L"Sound Controller 3 (default: Release Time), LSB" },
    {  73, L"Sound Controller 4 (default: Attack Time), LSB" },
    {  74, L"Sound Controller 5 (default: Brightness), LSB" },
    {  75, L"Sound Controller 6 (default: Decay Time), LSB" },
    {  76, L"Sound Controller 7 (default: Vibrato Rate), LSB" },
    {  77, L"Sound Controller 8 (default: Vibrato Depth), LSB" },
    {  78, L"Sound Controller 9 (default: Vibrato Delay), LSB" },
    {  79, L"Sound Controller 10 (default undefined), LSB" },

    {  80, L"General Purpose Controller 5, LSB" },
    {  81, L"General Purpose Controller 6, LSB" },
    {  82, L"General Purpose Controller 7, LSB" },
    {  83, L"General Purpose Controller 8, LSB" },

    {  84, L"Portamento Control, LSB" },                                    // Controls the amount of portamento.

    {  85, L"Undefined" },
    {  86, L"Undefined" },
    {  87, L"Undefined" },

    {  88, L"High Resolution Velocity Prefix, LSB" },                       // Extends the range of values to increase precision.

    {  89, L"Undefined" },
    {  90, L"Undefined" },

    {  91, L"Effects 1 Depth" },                                            // Usually sets the reverb send value.
    {  92, L"Effects 2 Depth" },                                            // Usually sets the tremolo value.
    {  93, L"Effects 3 Depth" },                                            // Usually sets the chorus value.
    {  94, L"Effects 4 Depth" },                                            // Usually sets the detune value.
    {  95, L"Effects 5 Depth" },                                            // Usually sets the phaser value.

    {  96, L"Data Increment (Data Entry +1)" },
    {  97, L"Data Decrement (Data Entry -1)" },

    {  98, L"Non-Registered Parameter Number (NRPN), LSB" },                // Selects the NRPN parameter for controller 6, 38, 96 and 97.
    {  99, L"Non-Registered Parameter Number (NRPN), MSB" },                // Selects the NRPN parameter for controller 6, 38, 96 and 97.

    { 100, L"Select Registered Parameter Number (RPN), LSB" },              // Selects the RPN parameter for controller 6, 38, 96 and 97.
    { 101, L"Select Registered Parameter Number (RPN), MSB" },              // Selects the RPN parameter for controller 6, 38, 96 and 97.

    { 102, L"Undefined" },
    { 103, L"Undefined" },
    { 104, L"Undefined" },
    { 105, L"Undefined" },
    { 106, L"Undefined" },
    { 107, L"Undefined" },
    { 108, L"Undefined" },
    { 109, L"Undefined" },
    { 110, L"Undefined" },
    { 111, L"Undefined" },
    { 112, L"Undefined" },
    { 113, L"Undefined" },
    { 114, L"Undefined" },
    { 115, L"Undefined" },
    { 116, L"Undefined" },
    { 117, L"Undefined" },
    { 118, L"Undefined" },
    { 119, L"Undefined" },

    { 120, L"[Channel Mode Message] All Sound Off" },
    { 121, L"[Channel Mode Message] Reset All Controllers" },
    { 122, L"[Channel Mode Message] Local Control On/Off, 0 off, 127 on" },
    { 123, L"[Channel Mode Message] All Notes Off" },
    { 124, L"[Channel Mode Message] Omni Mode Off (+ all notes off)" },
    { 125, L"[Channel Mode Message] Omni Mode On (+ all notes off)" },
    { 126, L"[Channel Mode Message] Mono Mode On (+ poly off, + all notes off)" },
    { 127, L"[Channel Mode Message] Poly Mode On (+ mono off, +all notes off)" },
};

#pragma warning(disable: 4820) // x bytes padding
struct manufacturer_description_t
{
    uint8_t Id;
    const wchar_t * Name;
};
#pragma warning(default: 4820) // x bytes padding

// https://www.amei.or.jp/report/System_ID_e.html
const manufacturer_description_t Manufacturers[]
{
    { 0x01, L"Sequential Circuits / Dave Smith Instruments" },
    { 0x02, L"IDP" },
    { 0x03, L"Octave-Plateau Electronics" },
    { 0x04, L"Moog" },
    { 0x05, L"Passport Designs" },
    { 0x06, L"Lexicon" },
    { 0x07, L"Kurzweil" },
    { 0x08, L"Fender" },
    { 0x09, L"Gulbransen" },
    { 0x0A, L"Delta Labs" },
    { 0x0B, L"Sound Comp." },
    { 0x0C, L"General Electro" },
    { 0x0D, L"Techmar" },
    { 0x0E, L"Matthews Research" },
    { 0x0F, L"Ensoniq" },

    // 0x010-0x1F American
    { 0x10, L"Oberheim" },
    { 0x11, L"PAIA / Apple Computer" },
    { 0x12, L"Simmons" },
    { 0x13, L"DigiDesign / Gentle Electric" },
    { 0x14, L"Fairlight" },
    { 0x15, L"JL Cooper" },
    { 0x16, L"Lowery" },
    { 0x17, L"Lin" },
    { 0x18, L"Emu Systems" },
    { 0x19, L"Harmony Systems" },
    { 0x1A, L"ART" },
    { 0x1B, L"Peavey / Baldwin" },
    { 0x1C, L"Eventide" },
    { 0x1D, L"Inventronics" },

    { 0x1F, L"Clarity" },
/*
    00H 00H 01H 	Time/Warner Interactive
    00H 00H 02H 	Advanced Gravis Comp.
    00H 00H 03H 	Media Vision
    00H 00H 04H 	Dornes Research Group
    00H 00H 05H 	K-Muse
    00H 00H 06H 	Stypher
    00H 00H 07H 	Digital Music Corp.
    00H 00H 08H 	IOTA Systems
    00H 00H 09H 	New England Digital
    00H 00H 0AH 	Artisyn
    00H 00H 0BH 	IVL Technologies
    00H 00H 0CH 	Southern Music Systems
    00H 00H 0DH 	Lake Butler Sound Co.
    00H 00H 0EH 	Alesis Studio Electronics
    00H 00H 0FH 	Sound Creation
    00H 00H 10H 	DOD Electronics Corp.
    00H 00H 11H 	Studer-Editech
    00H 00H 12H 	Sonus
    00H 00H 13H 	Temporal Acuity Products
    00H 00H 14H 	Perfect Fretworks
    00H 00H 15H 	KAT Inc.
    00H 00H 16H 	Opcode Systems
    00H 00H 17H 	Rane Corporation
    00H 00H 18H 	Anadi Electronique
    00H 00H 19H 	KMX
    00H 00H 1AH 	Allen & Heath Brenell
    00H 00H 1BH 	Peavey Electronics
    00H 00H 1CH 	360 System
    00H 00H 1DH 	Spectrum Design
    00H 00H 1EH 	Marquis Music
    00H 00H 1FH 	Zeta Systems
    00H 00H 20H 	Axxes
    00H 00H 21H 	Orban
    00H 00H 22H 	Indian Valley Mfg.
    00H 00H 23H 	Triton
    00H 00H 24H 	KTI
    00H 00H 25H 	Breakaway Technologies
    00H 00H 26H 	CAE Inc.
    00H 00H 27H 	Harrison Systems Inc.
    00H 00H 28H 	Future Lab/Mark Kuo
    00H 00H 29H 	Rocktron Corporation
    00H 00H 2AH 	PianoDisc
    00H 00H 2BH 	Cannon Research Group
    00H 00H 2DH 	Rodgers Instrument Corp.
    00H 00H 2EH 	Blue Sky Logic
    00H 00H 2FH 	Encore Electronics
    00H 00H 30H 	Uptown
    00H 00H 31H 	Voce
    00H 00H 32H 	CTI Audio
    00H 00H 33H 	S & S Research
    00H 00H 34H 	Broderbund Software
    00H 00H 35H 	Allen Organ Co.
    00H 00H 37H 	Music Quest
    00H 00H 38H 	Aphex
    00H 00H 39H 	Gallien Krueger
    00H 00H 3AH 	IBM
    00H 00H 3BH 	Mark of the Unicorn
    00H 00H 3CH 	Hotz Instruments
    00H 00H 3DH 	ETA Lighting
    00H 00H 3EH 	NSI Corporation
    00H 00H 3FH 	Ad Lib
    00H 00H 40H 	Richmond Sound Design
    00H 00H 41H 	Microsoft Corp
    00H 00H 42H 	Software Toolworks
    00H 00H 43H 	Russ Jones / Niche
    00H 00H 44H 	Intone
    00H 00H 45H 	Advanced Remote Tech.
    00H 00H 47H 	GT Electronics
    00H 00H 49H 	Timeline Vista
    00H 00H 4AH 	Mesa Boogie Ltd.
    00H 00H 4CH 	Sequoia Development
    00H 00H 4DH 	Studio Electronics
    00H 00H 4EH 	Euphonix
    00H 00H 4FH 	InterMIDI
    00H 00H 50H 	MIDI Solutions Inc.
    00H 00H 51H 	3DO Company
    00H 00H 52H 	Lightwave Research
    00H 00H 53H 	Micro-W Corporation
    00H 00H 54H 	Spectral Synthesis
    00H 00H 55H 	Lone Wolf
    00H 00H 56H 	Studio Technologies Inc.
    00H 00H 57H 	Peterson Electro-Musical
    00H 00H 58H 	Atari Corporation
    00H 00H 59H 	Marion Systems
    00H 00H 5AH 	Design Event
    00H 00H 5BH 	Winjammer Software
    00H 00H 5CH 	AT&T Bell Laboratories
    00H 00H 5EH 	Symetrix
    00H 00H 5FH 	MIDI the World
    00H 00H 60H 	Desper Products
    00H 00H 61H 	Micros 'N MIDI
    00H 00H 62H 	Accordians International
    00H 00H 63H 	EuPhonics
    00H 00H 64H 	Musonix
    00H 00H 65H 	Turtle Beach Systems
    00H 00H 66H 	Mackie Designs
    00H 00H 67H 	Compuserve
    00H 00H 68H 	BEC Technologies
    00H 00H 69H 	QRS Music Rolls Inc
    00H 00H 6AH 	P.G. Music
    00H 00H 6BH 	Sierra Semiconductor
    00H 00H 6CH 	EpiGraf Audio Visual
    00H 00H 6DH 	Electronics Diversified Inc
    00H 00H 6EH 	Tune 1000
    00H 00H 6FH 	Advanced Micro Devices
    00H 00H 70H 	Mediamation
    00H 00H 71H 	Sabine Musical Mfg. Co.
    00H 00H 72H 	Woog Labs
    00H 00H 73H 	Micropolis Corp
    00H 00H 74H 	Ta Horng Musical Instr.
    00H 00H 75H 	Forte Technologies
    00H 00H 76H 	Electro-Voice
    00H 00H 77H 	Midisoft Corporation
    00H 00H 78H 	Q-Sound Labs
    00H 00H 79H 	Westrex
    00H 00H 7AH 	NVidia
    00H 00H 7BH 	ESS Technology
    00H 00H 7CH 	MediaTrix Peripherals
    00H 00H 7DH 	Brooktree Corp
    00H 00H 7EH 	Otari Corp
    00H 00H 7FH 	Key Electronics, Inc.
    00H 01H 00H 	Shure Brothers Inc
    00H 01H 01H 	Crystalake Multimedia
    00H 01H 02H 	Crystal Semiconductor
    00H 01H 03H 	Rockwell Semiconductor
    00H 01H 04H 	Silicon Graphics
    00H 01H 05H 	Midiman
    00H 01H 06H 	PreSonus
    00H 01H 08H 	Topaz Enterprises
    00H 01H 09H 	Cast Lighting
    00H 01H 0AH 	Microsoft Consumer Division
    00H 01H 0CH 	Fast Forward Designs
    00H 01H 0DH 	lgor's Software Laboratories
    00H 01H 0EH 	Van Koevering Company
    00H 01H 0FH 	Altech Systems
    00H 01H 10H 	S & S Research
    00H 01H 11H 	VLSI Technology
    00H 01H 12H 	Chromatic Research
    00H 01H 13H 	Sapphire
    00H 01H 14H 	IDRC
    00H 01H 15H 	Justonic Tuning
    00H 01H 16H 	TorComp
    00H 01H 17H 	Newtek Inc
    00H 01H 18H 	Sound Sculpture
    00H 01H 19H 	Walker Technical
    00H 01H 1AH 	PAVO
    00H 01H 1BH 	InVision Interactive
    00H 01H 1CH 	T-Square Design
    00H 01H 1DH 	Nemesys Music Technology
    00H 01H 1EH 	DBX Professional (Harman Intl)
    00H 01H 1FH 	Syndyne Corporation
    00H 01H 20H 	Bitheadz
    00H 01H 21H 	Cakewalk Music Software
    00H 01H 22H 	Analog Devices (Staccato Systems)
    00H 01H 23H 	National Semiconductor
    00H 01H 24H 	Boom Theory / Adinolfi Alt. Perc.
    00H 01H 25H 	Virtual DSP Corporation
    00H 01H 26H 	Antares Systems
    00H 01H 27H 	Angel Software
    00H 01H 28H 	St Louis Music
    00H 01H 29H 	Lyrrus dba G-VOX
    00H 01H 2AH 	Ashley Audio Inc
    00H 01H 2BH 	Vari-Lite Inc
    00H 01H 2CH 	Summit Audio Inc
    00H 01H 2DH 	Aureal Semiconductor Inc
    00H 01H 2EH 	SeaSound LLC
    00H 01H 2FH 	U. S. Robotics
    00H 01H 30H 	Aurisis Research
    00H 01H 31H 	Nearfield Multimedia
    00H 01H 32H 	FM7 Inc
    00H 01H 33H 	Swivel Systems
    00H 01H 34H 	Hyperactive Audio Systems
    00H 01H 35H 	MidiLite (Castle Studios Prods)
    00H 01H 36H 	Radikal Technologies
    00H 01H 37H 	Roger Linn Design
    00H 01H 38H 	TC-Helicon Vocal Technologies
    00H 01H 39H 	Event Electronics
    00H 01H 3AH 	Sonic Network (Sonic Implants)
    00H 01H 3BH 	Realtime Music Solutions
    00H 01H 3CH 	Apogee Digital
    00H 01H 3DH 	Classical Organs, Inc.
    00H 01H 3EH 	Microtools Inc
    00H 01H 3FH 	Numark Industries
    00H 01H 40H 	Frontier Design Group LLC
    00H 01H 41H 	Recordare LLC
    00H 01H 42H 	Star Labs
    00H 01H 43H 	Voyager Sound Inc
    00H 01H 44H 	Manifold Labs
    00H 01H 45H 	Aviom Inc
    00H 01H 46H 	Mixmeister Technology
    00H 01H 47H 	Notation Software
    00H 01H 48H 	Mercurial Communications
    00H 01H 49H 	Wave Arts, Inc
    00H 01H 4AH 	Logic Sequencing Devices Inc
    00H 01H 4BH 	Axess Electronics
    00H 01H 4CH 	Muse Reasearch
    00H 01H 4DH 	Open Labs
    00H 01H 4EH 	Guillemot R&D Inc
    00H 01H 4FH 	Samson Technologies
    00H 01H 50H 	Electoronic Theatre Controls
    00H 01H 51H 	Research In Motion
    00H 01H 52H 	Mobileer
    00H 01H 53H 	Synthogy
    00H 01H 54H 	Lynx Studio Technology Inc.
    00H 01H 55H 	Damage Control Engineering LLC
    00H 01H 56H 	Yost Engineering, Inc.
    00H 01H 57H 	Brooks & Forsman Designs LLC
    00H 01H 58H 	Magnekey
    00H 01H 59H 	Garritan Corp
    00H 01H 5AH 	Plogue Art et Technology, Inc.
    00H 01H 5BH 	RJM Music Technology
    00H 01H 5CH 	Custom Solutions Software
    00H 01H 5DH 	Sonarcana LLC
    00H 01H 5EH 	Centrance 
*/
    // 0x20-0x3F European
    { 0x20, L"Passac / Bon Tempi" },
    { 0x21, L"S.I.E.L." },
    { 0x22, L"Synthaxe (UK)" },
    { 0x23, L"Stepp" },
    { 0x24, L"Hohner" },
    { 0x25, L"Crumar / Twister" },
    { 0x26, L"Solton" },
    { 0x27, L"Jellinghaus MS" },
    { 0x28, L"CTS / Southworth Music Systems" },
    { 0x29, L"PPG" },
    { 0x2A, L"JEN" },
    { 0x2B, L"Solid State Organ Systems" },
    { 0x2C, L"Audio Vertrieb P. Struven" },
    { 0x2D, L"Hinton Instruments / Neve" },
    { 0x2E, L"Soundtracs" },
    { 0x2F, L"Elka" },
    { 0x30, L"Dynacord" },
    { 0x31, L"Viscount" },
    { 0x32, L"Drawmer" },
    { 0x33, L"Clavia Digital Instruments" },
    { 0x34, L"Audio Architecture" },
    { 0x35, L"GeneralMusic" },
    { 0x36, L"Cheetah Marketing" },
    { 0x37, L"C.T.M." },
    { 0x38, L"Simmons UK" },
    { 0x39, L"Soundcraft Electronics" },
    { 0x3A, L"Steinberg" },
    { 0x3B, L"Wersi" },
    { 0x3C, L"AVAB Niethammer" },
    { 0x3D, L"Digigram" },
    { 0x3E, L"Waldorf Electronics" },
    { 0x3F, L"Quasimidi" },
/*
    00H 20H 00H 	Dream
    00H 20H 01H 	Strand Lighting
    00H 20H 02H 	Amek Systems
    00H 20H 03H 	Casa Di Risparmio Di Loreto
    00H 20H 04H 	Bohm electronic GmbH
    00H 20H 05H 	Syntec Digital Audio
    00H 20H 06H 	Trident Audio Developments
    00H 20H 07H 	Real World Studio
    00H 20H 08H 	Evolution Synthesis
    00H 20H 09H 	Yes Technology
    00H 20H 0AH 	Audiomatica
    00H 20H 0BH 	Bontempi / Farfisa (COMUS)
    00H 20H 0CH 	F.B.T. Elettronica SpA
    00H 20H 0DH 	MidiTemp GmbH
    00H 20H 0EH 	LA Audio (Larking Audio)
    00H 20H 0FH 	Zero 88 Lighting Limited
    00H 20H 10H 	Micon Audio Electronics GmbH
    00H 20H 11H 	Forefront Technology
    00H 20H 12H 	Studio Audio and Video Ltd.
    00H 20H 13H 	Kenton Electronics
    00H 20H 14H 	Celco Division of Electrosonic
    00H 20H 15H 	ADB
    00H 20H 16H 	Marshall Products Limited
    00H 20H 17H 	DDA
    00H 20H 18H 	BSS Audio Ltd.
    00H 20H 19H 	MA Lighting Technology
    00H 20H 1AH 	Fatar SRL c/o Music Industries
    00H 20H 1CH 	Artisan Clasic Organ Inc.
    00H 20H 1DH 	Orla Spa
    00H 20H 1EH 	Pinnacle Audio (Klark Teknik)
    00H 20H 1FH 	TC Electronics
    00H 20H 20H 	Doepfer Musikelektronik GmbH
    00H 20H 21H 	Creative Technology Pte. Ltd. c/o
    00H 20H 22H 	Seiyddo/Minami
    00H 20H 23H 	Goldstar Co. Ltd.
    00H 20H 24H 	Midisoft s.a.s. di M.Cima & C
    00H 20H 25H 	Samick Musical Inst. Co. Ltd.
    00H 20H 26H 	Penny and Giles
    00H 20H 27H 	Acorn Computer
    00H 20H 28H 	LSC Electronics Pty. Ltd.
    00H 20H 29H 	Novation EMS
    00H 20H 2AH 	Samkyung Mechatronics
    00H 20H 2BH 	Medeli Electronics Co
    00H 20H 2CH 	Charlie Lab SRL
    00H 20H 2DH 	Blue Chip Music Technology
    00H 20H 2EH 	BEE OH Corp
    00H 20H 2FH 	LG Semiconductor
    00H 20H 30H 	TESI
    00H 20H 31H 	EMAGIC
    00H 20H 32H 	Behringer GmbH
    00H 20H 33H 	Access
    00H 20H 34H 	Synoptic
    00H 20H 35H 	Hanmesoft Corp
    00H 20H 36H 	Terratec Electronic GmbH
    00H 20H 37H 	Proel SpA
    00H 20H 38H 	IBK MIDI
    00H 20H 39H 	IRCAM
    00H 20H 3AH 	Propellerhead Software
    00H 20H 3BH 	Red Sound Systems Ltd
    00H 20H 3CH 	Elektron ESI AB
    00H 20H 3DH 	Sintefex Audio
    00H 20H 3EH 	MAM (Music and More)
    00H 20H 3FH 	Amsaro GmbH
    00H 20H 40H 	CDS Advanced Technology BV
    00H 20H 41H 	Touched By Sound GmbH
    00H 20H 42H 	DSP Arts
    00H 20H 43H 	Phil Rees Music Tech
    00H 20H 44H 	Stamer Musikanlagen GmbH
    00H 20H 45H 	Soundart (Musical Muntaner)
    00H 20H 46H 	C-Mexx Software
    00H 20H 47H 	Klavis Technologies
    00H 20H 48H 	Noteheads AB
    00H 20H 49H 	Algorithmix
    00H 20H 4AH 	Skrydstrup R&D
    00H 20H 4BH 	Professional Audio Company
    00H 20H 4CH 	DBTECH
    00H 20H 4DH 	Vermona
    00H 20H 4EH 	Nokia
    00H 20H 4FH 	Wave Idea
    00H 20H 50H 	Hartmann GmbH
    00H 20H 51H 	Lion's Tracs
    00H 20H 52H 	Analogue Systems
    00H 20H 53H 	Focal-JMlab
    00H 20H 54H 	Ringway Electronics (Chang-Zhou)
    00H 20H 55H 	Faith Technologies (Digiplug)
    00H 20H 56H 	Showwork
    00H 20H 57H 	Manikin Electoronic
    00H 20H 58H 	1 Come Tech
    00H 20H 59H 	Phonic Corp
    00H 20H 5AH 	Lake Technology
    00H 20H 5BH 	Silansys Technologies
    00H 20H 5CH 	Winbond Electronics
    00H 20H 5DH 	Cinetix Medien und Interface GmbH
    00H 20H 5EH 	A&G Soluzioni Digitali
    00H 20H 5FH 	Sequentix Music Systems
    00H 20H 60H 	Oram Pro Audio
    00H 20H 61H 	Be4 Ltd.
    00H 20H 62H 	Infection Music
    00H 20H 63H 	Central Music Co.(CME)
    00H 20H 64H 	GenoQs Machines
    00H 20H 65H 	Medialon
    00H 20H 66H 	Waves Audio Ltd
    00H 20H 67H 	Jerash Labs
    00H 20H 68H 	Da Fact
    00H 20H 69H 	Elby Designs
    00H 20H 6AH 	Spectral Audio
    00H 20H 6BH 	Arturia
    00H 20H 6CH 	Vixid
    00H 20H 6DH 	C-Thru Music
*/
    // 0x40-0x5F Japanese
    { 0x40, L"Kawai Musical Instruments" },
    { 0x41, L"Roland" },
    { 0x42, L"Korg" },
    { 0x43, L"Yamaha" },
    { 0x44, L"Casio Computer" },

    { 0x46, L"Kamiya Studio" },
    { 0x47, L"Akai Electric" },
    { 0x48, L"Victor Company of Japan" },

    { 0x4B, L"Fujitsu" },
    { 0x4C, L"Sony" },

    { 0x4E, L"Teac" },

    { 0x50, L"Matsushita Electric Industrial" },
    { 0x51, L"Fostex" },
    { 0x52, L"Zoom" },

    { 0x54, L"Matsushita Communication Industrial" },
    { 0x55, L"Suzuki Musical Instruments" },
    { 0x56, L"Fuji Sound" },
    { 0x57, L"Acoustic Technical Laboratory" },

    { 0x59, L"Faith" },
    { 0x5A, L"Internet Corporation" },
    { 0x5C, L"Seekers Co." },
    { 0x5F, L"SD Card Association" },

    // 0x60-0x7C Other

    // 0x7D-0x7F Special
    { 0x7D, L"Private Use" },
    { 0x7E, L"Universal (Non-Real Time)" },
    { 0x7F, L"Universal (Real Time)" },

/*
    00H 40H 00H 	CRIMSON TECHNOLOGY INC
    00H 40H 01H 	SOFTBANK MOBILE CORP
    00H 40H 03H 	D&M HOLDINGS INC.
    00H 40H 04H 	XING INC.
    00H 40H 05H 	Pioneer DJ Corporation
*/
};
