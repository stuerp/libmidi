
/** $VER: MIDIContainer.h (2024.05.11) **/

#pragma once

#include "framework.h"

#include "MIDI.h"
#include "Range.h"

#pragma warning(disable: 4820) // x bytes padding added after data member 'y'

struct MIDIEvent
{
    enum MIDIEventType
    {
        NoteOff = 0,        // x080
        NoteOn,             // x090
        KeyPressure,        // x0A0
        ControlChange,      // x0B0
        ProgramChange,      // x0C0
        ChannelPressure,    // x0D0
        PitchBendChange,    // x0E0
        Extended            // x0F0
    };

    uint32_t Timestamp;
    MIDIEventType Type;
    uint32_t ChannelNumber;
    std::vector<uint8_t> Data;

    MIDIEvent() noexcept : Timestamp(0), Type(MIDIEventType::NoteOff), ChannelNumber(0) { }
    MIDIEvent(const MIDIEvent & midiEvent);
    MIDIEvent(uint32_t timestamp, MIDIEventType eventType, uint32_t channel, const uint8_t * data, size_t size);
};

class MIDITrack
{
public:
    MIDITrack() noexcept { }

    MIDITrack(const MIDITrack & track) noexcept
    {
        _Events = track._Events;
    }

    MIDITrack & operator=(const MIDITrack & track)
    {
        _Events = track._Events;

        return *this;
    }

    void AddEvent(const MIDIEvent & event);
    void RemoveEvent(size_t index);

    size_t GetLength() const noexcept
    {
        return _Events.size();
    }

    const MIDIEvent & operator[](size_t index) const noexcept
    {
        return _Events[index];
    }

    MIDIEvent & operator[](std::size_t index) noexcept
    {
        return _Events[index];
    }

public:
    using midi_events_t = std::vector<MIDIEvent>;

    using iterator       = midi_events_t::iterator;
    using const_iterator = midi_events_t::const_iterator;

    iterator begin() { return _Events.begin(); }
    iterator end() { return _Events.end(); }

    const_iterator begin() const { return _Events.begin(); }
    const_iterator end() const { return _Events.end(); }

    const_iterator cbegin() const { return _Events.cbegin(); }
    const_iterator cend() const { return _Events.cend(); }

    const MIDIEvent & front() const noexcept { return _Events.front(); }
    const MIDIEvent & back() const noexcept { return _Events.back(); }

private:
    std::vector<MIDIEvent> _Events;
};

struct TempoItem
{
    uint32_t Timestamp;
    uint32_t Tempo;

    TempoItem() noexcept : Timestamp(0), Tempo(0)
    {
    }

    TempoItem(uint32_t timestamp, uint32_t tempo);
};

class TempoMap
{
public:
    void Add(uint32_t tempo, uint32_t timestamp);
    uint32_t TimestampToMS(uint32_t timestamp, uint32_t division) const;

    size_t Size() const
    {
        return _Items.size();
    }

    const TempoItem & operator[](std::size_t p_index) const
    {
        return _Items[p_index];
    }

    TempoItem & operator[](size_t index)
    {
        return _Items[index];
    }

private:
    std::vector<TempoItem> _Items;
};

struct SysExItem
{
    size_t Offset;
    size_t Size;
    uint8_t PortNumber;

    SysExItem() noexcept : Offset(0), Size(0), PortNumber(0) { }
    SysExItem(const SysExItem & src);
    SysExItem(uint8_t portNumber, std::size_t offset, std::size_t size);
};

class SysExTable
{
public:
    size_t AddItem(const uint8_t * data, std::size_t size, uint8_t portNumber);
    bool GetItem(size_t index, const uint8_t * & data, std::size_t & size, uint8_t & portNumber) const;

    size_t Size() const
    {
        return _Items.size();
    }

private:
    std::vector<SysExItem> _Items;
    std::vector<uint8_t> _Data;
};

struct MIDIStreamEvent
{
    uint32_t Timestamp; // in ms
    uint32_t Data;

    MIDIStreamEvent() noexcept : Timestamp(0), Data(0)
    {
    }

    MIDIStreamEvent(uint32_t timestamp, uint32_t data)
    {
        Timestamp = timestamp;
        Data = data;
    }
};

struct MIDIMetaDataItem
{
    uint32_t Timestamp;
    std::string Name;
    std::string Value;

    MIDIMetaDataItem() noexcept : Timestamp(0) { }

    MIDIMetaDataItem(const MIDIMetaDataItem & item) noexcept { operator=(item); };
    MIDIMetaDataItem & operator=(const MIDIMetaDataItem & other) noexcept
    {
        Timestamp = other.Timestamp;
        Name = other.Name;
        Value = other.Value;

        return *this;
    }

    MIDIMetaDataItem(MIDIMetaDataItem && item) { operator=(item); }
    MIDIMetaDataItem & operator=(MIDIMetaDataItem && other)
    {
        Timestamp = other.Timestamp;
        Name = std::move(Name);
        Value = std::move(Value);

        return *this;
    }

    virtual ~MIDIMetaDataItem() { }

    MIDIMetaDataItem(uint32_t timestamp, const char * name, const char * value) noexcept
    {
        Timestamp = timestamp;
        Name = name;
        Value = value;
    }
};

class MIDIMetaData
{
public:
    MIDIMetaData() noexcept { }

    void AddItem(const MIDIMetaDataItem & item);
    void Append(const MIDIMetaData & data);
    bool GetItem(const char * name, MIDIMetaDataItem & item) const;
    bool GetBitmap(std::vector<uint8_t> & bitmap) const;
    void AssignBitmap(std::vector<uint8_t>::const_iterator const & begin, std::vector<uint8_t>::const_iterator const & end);
    std::size_t GetCount() const;

    const MIDIMetaDataItem & operator[](size_t index) const;

private:
    std::vector<MIDIMetaDataItem> _Items;
    std::vector<uint8_t> _Bitmap;
};

class MIDIContainer
{
public:
    MIDIContainer() : _Format(0), _TimeDivision(0), _ExtraPercussionChannel(~0u)
    {
        _DeviceNames.resize(16);
    }

    void Initialize(uint32_t format, uint32_t division);

    void AddTrack(const MIDITrack & track);
    void AddEventToTrack(size_t trackIndex, const MIDIEvent & event);

    // These functions are really only designed to merge and later remove System Exclusive message dumps.
    void MergeTracks(const MIDIContainer & source);
    void SetTrackCount(uint32_t count);
    void SetExtraMetaData(const MIDIMetaData & data);

    void ApplyHack(uint32_t hack);

    void SerializeAsStream(size_t subSongIndex, std::vector<MIDIStreamEvent> & stream, SysExTable & sysExTable, uint32_t & loopBegin, uint32_t & loopEnd, uint32_t cleanFlags) const;
    void SerializeAsSMF(std::vector<uint8_t> & data) const;

    void PromoteToType1();

    void TrimStart();

    typedef std::string(* SplitCallback)(uint8_t bank_msb, uint8_t bank_lsb, uint8_t instrument);

    void SplitByInstrumentChanges(SplitCallback callback = nullptr);

    size_t GetSubSongCount() const;
    size_t GetSubSong(size_t index) const;

    uint32_t GetDuration(size_t subsongIndex, bool ms = false) const;

    uint32_t GetFormat() const;
    uint32_t GetTrackCount() const;
    uint32_t GetChannelCount(size_t subSongIndex) const;

    uint32_t GetLoopBeginTimestamp(size_t subSongIndex, bool ms = false) const;
    uint32_t GetLoopEndTimestamp(size_t subSongIndex, bool ms = false) const;

    std::vector<MIDITrack> & GetTracks() { return _Tracks; }

    void GetMetaData(size_t subSongIndex, MIDIMetaData & data);

    void SetExtraPercussionChannel(uint32_t channelNumber) noexcept { _ExtraPercussionChannel = channelNumber; }
    uint32_t GetExtraPercussionChannel() const noexcept { return _ExtraPercussionChannel; }

    void DetectLoops(bool detectXMILoops, bool detectMarkerLoops, bool detectRPGMakerLoops, bool detectTouhouLoops);

    static void EncodeVariableLengthQuantity(std::vector<uint8_t> & data, uint32_t delta);

public:
    using miditracks_t = std::vector<MIDITrack>;
    using iterator = miditracks_t::iterator;
    using const_iterator = miditracks_t::const_iterator;

    iterator begin() { return _Tracks.begin(); }
    iterator end() { return _Tracks.end(); }

    const_iterator begin() const { return _Tracks.begin(); }
    const_iterator end() const { return _Tracks.end(); }
    const_iterator cbegin() const { return _Tracks.cbegin(); }
    const_iterator cend() const { return _Tracks.cend(); }

public:
    enum
    {
        CleanFlagEMIDI = 1 << 0,
        CleanFlagInstruments = 1 << 1,
        CleanFlagBanks = 1 << 2,
    };

private:
    void TrimRange(size_t start, size_t end);
    void TrimTempoMap(size_t index, uint32_t base_timestamp);

    uint32_t TimestampToMS(uint32_t timestamp, size_t subsongIndex) const;

    #pragma warning(disable: 4267)
    // Normalize port numbers properly
    template <typename T> void LimitPortNumber(T & number)
    {
        for (size_t i = 0; i < _PortNumbers.size(); ++i)
        {
            if (_PortNumbers[i] == number)
            {
                number = (T) i;

                return;
            }
        }

        _PortNumbers.push_back((uint8_t) number);

        number = _PortNumbers.size() - 1;
    }

    template <typename T> void LimitPortNumber(T & number) const
    {
        for (size_t i = 0; i < _PortNumbers.size(); i++)
        {
            if (_PortNumbers[i] == number)
            {
                number = (T) i;

                return;
            }
        }
    }
    #pragma warning(default: 4267)

    void AssignString(const char * src, size_t srcLength, std::string & dst) const
    {
        dst.assign(src, src + srcLength);
    }

private:
    uint32_t _Format;
    uint32_t _TimeDivision;
    uint32_t _ExtraPercussionChannel;

    std::vector<uint64_t> _ChannelMask;
    std::vector<TempoMap> _TempoMaps;
    std::vector<MIDITrack> _Tracks;

    std::vector<uint8_t> _PortNumbers;

    std::vector<std::vector<std::string>> _DeviceNames;

    MIDIMetaData _ExtraMetaData;

    std::vector<uint32_t> _EndTimestamps;

    std::vector<range_t> _Loop;
};
