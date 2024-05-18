
/** $VER: MIDIProcessorSMF.cpp (2024.05.16) Standard MIDI File **/

#include "framework.h"

#include "MIDIProcessor.h"

const uint8_t SysExUseForRhythmPartCh16[] = { 0xF0, 0x41, 0x10, 0x42, 0x12, 0x40, 0x1F, 0x15, 0x02, 0x0A, 0xF7 }; // Use channel 16 for rhythm.

/// <summary>
/// 
/// </summary>
bool midi_processor_t::IsSMF(std::vector<uint8_t> const & data)
{
    if (data.size() < 18)
        return false;

    if (::memcmp(&data[0], "MThd", 4) != 0)
        return false;

    if (data[4] != 0 || data[5] != 0 || data[6] != 0 || data[7] != 6)
        return false;

    int Format = (data[8] << 8) | data[9];

    if (Format > 2)
        return false;

    int TrackCount = (data[10] << 8) | data[11];

    if ((TrackCount == 0) || ((Format == 0) && (TrackCount != 1)))
        return false;

    if (data[12] == 0 && data[13] == 0)
        return false;

    if (::memcmp(&data[14], "MTrk", 4) != 0)
        return false;

    return true;
}

/// <summary>
/// 
/// </summary>
bool midi_processor_t::ProcessSMF(std::vector<uint8_t> const & data, midi_container_t & container)
{
    if (data.size() < 18)
        throw MIDIException("Insufficient data in the stream");

    if (::memcmp(&data[0], "MThd", 4) != 0)
        throw MIDIException("Bad SMF header chunk type");

    if (data[4] != 0 || data[5] != 0 || data[6] != 0 || data[7] != 6)
        throw MIDIException("Bad SMF header chunk size");

    int Format = (data[8] << 8) | data[9];

    if (Format > 2)
        throw MIDIException("Bad MIDI format in SMF header");

    int TrackCount = (data[10] << 8) | data[11];

    if ((TrackCount == 0) || ((Format == 0) && (TrackCount != 1)))
        throw MIDIException("Invalid track count in SMF header");

    int TimeDivision = (data[12] << 8) | data[13];

    if ((TimeDivision == 0))
        throw MIDIException("Invalid time division in SMF header");

    container.Initialize((uint32_t) Format, (uint32_t) TimeDivision);

    std::vector<uint8_t>::const_iterator Data = data.begin() + 14;
    std::vector<uint8_t>::const_iterator Tail = data.end();

    for (int i = 0; i < TrackCount; ++i)
    {
        if (Tail - Data < 8)
            throw MIDIException("Insufficient data in the stream");

        uint32_t ChunkSize = (uint32_t)((Data[4] << 24) | (Data[5] << 16) | (Data[6] << 8) | Data[7]);

        if (::memcmp(&Data[0], "MTrk", 4) == 0)
        {
            if (Tail - Data < (ptrdiff_t) (8 + ChunkSize))
                throw MIDIException("Insufficient data in the stream");

            Data += 8;

            std::vector<uint8_t>::const_iterator ChunkTail = Data + (int) ChunkSize;

            if (!ProcessSMFTrack(Data, ChunkTail, container, true))
                return false;

            Data = ChunkTail; // In case no all track data gets used.
        }
        // Skip unknown chunks in the stream.
        else
        {
            if (Tail - Data < (ptrdiff_t) (8 + ChunkSize))
                throw MIDIException("Insufficient data in the stream");

            Data += (int64_t)(8) + ChunkSize;

            continue;
        }
    }

    return true;
}

/// <summary>
/// 
/// </summary>
bool midi_processor_t::ProcessSMFTrack(std::vector<uint8_t>::const_iterator & data, std::vector<uint8_t>::const_iterator tail, midi_container_t & container, bool trackNeedsEndMarker)
{
    midi_track_t Track;

    uint32_t Timestamp = 0;
    uint8_t LastStatusCode = 0xFF;

    uint32_t SysExSize = 0;
    uint32_t SysExTimestamp = 0;

    std::vector<uint8_t> Temp(3);

    bool DetectedPercussionText = false;

    for (;;)
    {
        if (!trackNeedsEndMarker && (data == tail))
            break;

        int DeltaTime = DecodeVariableLengthQuantity(data, tail);

        if (data == tail)
            throw MIDIException("Insufficient data in the stream");

        if (DeltaTime < 0)
            DeltaTime = -DeltaTime; // "Encountered negative delta: " << delta << "; flipping sign."

        Timestamp += DeltaTime;

        uint32_t BytesRead = 0;

        uint8_t StatusCode = *data++;

        if (StatusCode < StatusCodes::NoteOff)
        {
            if (LastStatusCode == 0xFF)
                throw MIDIException("Bad first message in MIDI track");

            Temp.resize(3);

            Temp[BytesRead++] = StatusCode;

            StatusCode = LastStatusCode;
        }

        if (StatusCode < StatusCodes::SysEx)
        {
            if (SysExSize > 0)
            {
                Track.AddEvent(midi_event_t(SysExTimestamp, midi_event_t::Extended, 0, Temp.data(), SysExSize));
                SysExSize = 0;
            }

            LastStatusCode = StatusCode;

            if (!trackNeedsEndMarker && ((StatusCode & 0xF0) == StatusCodes::PitchBendChange))
                continue;

            if (BytesRead == 0)
            {
                if (data == tail)
                    throw MIDIException("Insufficient data in the stream");

                Temp.resize(3);

                Temp[0] = *data++;
                BytesRead = 1;
            }

            switch (StatusCode & 0xF0)
            {
                case StatusCodes::ProgramChange:
                case StatusCodes::ChannelPressure:
                    break;

                default:
                    if (data == tail)
                        throw MIDIException("Insufficient data in the stream");

                    Temp[BytesRead++] = *data++;
            }

            uint32_t ChannelNumber = (uint32_t) (StatusCode & 0x0F);

            // Assign percussion to channel 16 if it's first message was preceded with meta data containing the word "drum".
            {

                if ((ChannelNumber == 0x0F) && DetectedPercussionText)
                {
                    Track.AddEvent(midi_event_t(0, midi_event_t::Extended, 0, SysExUseForRhythmPartCh16, _countof(SysExUseForRhythmPartCh16)));

                    container.SetExtraPercussionChannel(ChannelNumber);

                    DetectedPercussionText = false;
                }
            }

            Track.AddEvent(midi_event_t(Timestamp, (midi_event_t::MIDIEventType) ((StatusCode >> 4) - 8), ChannelNumber, Temp.data(), BytesRead));
        }
        else
        if (StatusCode == StatusCodes::SysEx)
        {
            if (SysExSize > 0)
            {
                Track.AddEvent(midi_event_t(SysExTimestamp, midi_event_t::Extended, 0, Temp.data(), SysExSize));
                SysExSize = 0;
            }

            int Size = DecodeVariableLengthQuantity(data, tail);

            if (Size < 0)
                throw MIDIException("Invalid System Exclusive message");

            if (Size > tail - data)
                throw MIDIException("Insufficient data in the stream");

            {
                Temp.resize((size_t) (Size + 1));

                Temp[0] = StatusCodes::SysEx;

                std::copy(data, data + Size, Temp.begin() + 1);
                data += Size;

                SysExSize = (uint32_t) (Size + 1);
                SysExTimestamp = Timestamp;
            }
        }
        else
        if (StatusCode == StatusCodes::SysExEnd)
        {
            if (SysExSize == 0)
                throw MIDIException("Invalid System Exclusive End message");

            // Add the SysEx continuation to the current SysEx message
            int Size = DecodeVariableLengthQuantity(data, tail);

            if (Size < 0)
                throw MIDIException("Invalid System Exclusive message");

            if (Size > tail - data)
                throw MIDIException("Insufficient data in the stream");

            {
                Temp.resize((size_t) SysExSize + Size);

                std::copy(data, data + Size, Temp.begin() + (int) SysExSize);
                data += Size;

                SysExSize += Size;
            }
        }
        else
        if (StatusCode == StatusCodes::MetaData)
        {
            if (SysExSize > 0)
            {
                Track.AddEvent(midi_event_t(SysExTimestamp, midi_event_t::Extended, 0, Temp.data(), SysExSize));
                SysExSize = 0;
            }

            if (data == tail)
                throw MIDIException("Insufficient data for meta data message");

            uint8_t MetaDataType = *data++;

            if (MetaDataType > MetaDataTypes::SequencerSpecific)
                throw MIDIException("Unrecognized meta data type");

            int Size = DecodeVariableLengthQuantity(data, tail);

            if (Size < 0)
                throw MIDIException("Invalid meta data message");

            if (Size > tail - data)
                throw MIDIException("Insufficient data in the stream");

            // Remember when the track or instrument name contains the word "drum". We'll need it later.
            if ((MetaDataType == MetaDataTypes::Text) || (MetaDataType == MetaDataTypes::TrackName) || (MetaDataType == MetaDataTypes::InstrumentName))
            {
                const char * p = (const char *) &data[0];

                for (int n = Size; n > 3; --n, p++)
                {
                    if (::_strnicmp(p, "drum", 4) == 0)
                    {
                        DetectedPercussionText = true;
                        break;
                    }
                }
            }

            {
                Temp.resize((size_t)(Size + 2));

                Temp[0] = StatusCodes::MetaData;
                Temp[1] = MetaDataType;

                std::copy(data, data + Size, Temp.begin() + 2);
                data += Size;

                Track.AddEvent(midi_event_t(Timestamp, midi_event_t::Extended, 0, Temp.data(), (size_t) (Size + 2)));

                if (MetaDataType == MetaDataTypes::EndOfTrack) // Mandatory, Marks the end of the track.
                {
                    trackNeedsEndMarker = true;
                    break;
                }
            }
        }
        else
        if ((StatusCodes::SysExEnd < StatusCode) && (StatusCode < StatusCodes::MetaData)) // Sequencer specific events, single byte.
        {
            Temp[0] = StatusCode;

            Track.AddEvent(midi_event_t(Timestamp, midi_event_t::Extended, 0, Temp.data(), 1));
        }
        else
            throw MIDIException("Unknown status code");
    }

    if (!trackNeedsEndMarker)
    {
        Temp[0] = StatusCodes::MetaData;
        Temp[1] = MetaDataTypes::EndOfTrack;

        Track.AddEvent(midi_event_t(Timestamp, midi_event_t::Extended, 0, Temp.data(), 2));
    }

    container.AddTrack(Track);

    return true;
}
