
/** $VER: MIDIProcessor.h (2024.05.16) **/

#pragma once

#include "framework.h"

#include "MIDIContainer.h"
#include "Exception.h"
#include "IFF.h"

class midi_processor_t
{
public:
    static bool Process(std::vector<uint8_t> const & data, const wchar_t * filePath, midi_container_t & container);

private:
    static bool IsSMF(std::vector<uint8_t> const & data);
    static bool IsRIFF(std::vector<uint8_t> const & data);
    static bool IsHMP(std::vector<uint8_t> const & data);
    static bool IsHMI(std::vector<uint8_t> const & data);
    static bool IsXMI(std::vector<uint8_t> const & data);
    static bool IsMUS(std::vector<uint8_t> const & data);
    static bool IsMDS(std::vector<uint8_t> const & data);
    static bool IsLDS(std::vector<uint8_t> const & data, const wchar_t * fileExtension);
    static bool IsGMF(std::vector<uint8_t> const & data);
    static bool IsRCP(std::vector<uint8_t> const & data, const wchar_t * fileExtension);
    static bool IsSysEx(std::vector<uint8_t> const & data);

    static bool ProcessSMF(std::vector<uint8_t> const & data, midi_container_t & container);
    static bool ProcessRIFF(std::vector<uint8_t> const & data, midi_container_t & container);
    static bool ProcessHMP(std::vector<uint8_t> const & data, midi_container_t & container);
    static bool ProcessHMI(std::vector<uint8_t> const & data, midi_container_t & container);
    static bool ProcessXMI(std::vector<uint8_t> const & data, midi_container_t & container);
    static bool ProcessMUS(std::vector<uint8_t> const & data, midi_container_t & container);
    static bool ProcessMDS(std::vector<uint8_t> const & data, midi_container_t & container);
    static bool ProcessLDS(std::vector<uint8_t> const & data, midi_container_t & container);
    static bool ProcessGMF(std::vector<uint8_t> const & data, midi_container_t & container);
    static bool ProcessRCP(std::vector<uint8_t> const & data, const wchar_t * filePath, midi_container_t & container);
    static bool ProcessSysEx(std::vector<uint8_t> const & data, midi_container_t & container);

    static bool ProcessSMFTrack(std::vector<uint8_t>::const_iterator & it, std::vector<uint8_t>::const_iterator end, midi_container_t & container, bool needs_end_marker);
    static int DecodeVariableLengthQuantity(std::vector<uint8_t>::const_iterator & it, std::vector<uint8_t>::const_iterator end) noexcept;

    static uint32_t DecodeVariableLengthQuantityHMP(std::vector<uint8_t>::const_iterator & it, std::vector<uint8_t>::const_iterator end) noexcept;

    static bool ReadStream(std::vector<uint8_t> const & data, iff_stream_t & stream);
    static bool ReadChunk(std::vector<uint8_t>::const_iterator & it, std::vector<uint8_t>::const_iterator end, iff_chunk_t & chunk, bool isFirstChunk);
    static uint32_t DecodeVariableLengthQuantityXMI(std::vector<uint8_t>::const_iterator & it, std::vector<uint8_t>::const_iterator end) noexcept;

private:
    static const uint8_t MIDIEventEndOfTrack[2];
    static const uint8_t LoopBeginMarker[11];
    static const uint8_t LoopEndMarker[9];

    static const uint8_t DefaultTempoXMI[5];

    static const uint8_t DefaultTempoHMP[5];

    static const uint8_t DefaultTempoMUS[5];
    static const uint8_t MusControllers[15];

    static const uint8_t DefaultTempoLDS[5];
};
