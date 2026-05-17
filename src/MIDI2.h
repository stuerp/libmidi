
/** $VER: MIDI2.h (2026.05.17) **/

#pragma once

#include <cstdint>
#include <vector>

namespace midi2
{

enum class MessageType : uint8_t
{
    Utility           =  0,
    System            =  1,
    MIDI1ChannelVoice =  2,
    Data              =  3,
    MIDI2ChannelVoice =  4,
    ExtendedData      =  5,

    FlexData          = 13,

    Stream            = 15,
};

enum class StatusBank : uint8_t
{
    Setup             =  0, // Setup & Performance
    MetadataText      =  1,
    PerformanceText   =  2, // Including lyrics
};

class ump_t
{
public:
    ump_t() = default;
    ump_t(const uint8_t * data, size_t size) noexcept;

    MessageType GetMessageType() const noexcept
    {
        const auto Type = (MessageType) (Data[0] >> 4);

        return Type;
    }

    static size_t GetPacketSize(MessageType mt) noexcept;
    static std::vector<ump_t> FromBytes(const uint8_t * data, size_t size) noexcept;

public:
    uint8_t Data[16];
    size_t Size;
};

class message_t
{
public:
    message_t() = default;
    message_t(const ump_t & packet) noexcept : _Packet(packet) { }

    virtual ~message_t() = default;

    virtual uint16_t Status() const noexcept
    {
        return (uint16_t) (_Packet.Data[1] >> 4);
    }

protected:
    const ump_t & _Packet;
};

class utility_message_t : public message_t
{
public:
    utility_message_t() = default;
    utility_message_t(const ump_t & packet) noexcept : message_t(packet) { }

    uint16_t Payload() const noexcept
    {
        return (uint16_t) ((_Packet.Data[2] << 8) | _Packet.Data[3]);
    }
};

class system_message_t : public message_t
{
public:
    system_message_t() = default;
    system_message_t(const ump_t & packet) noexcept : message_t(packet) { }

    uint8_t Group() const noexcept
    {
        return (uint8_t) (_Packet.Data[0] & 0x0F);
    }

    uint16_t Status() const noexcept override final
    {
        return (uint16_t) _Packet.Data[1];
    }
};

class midi1_channel_voice_message_t : public message_t
{
public:
    midi1_channel_voice_message_t() = default;
    midi1_channel_voice_message_t(const ump_t & packet) noexcept : message_t(packet) { }

    uint8_t Group() const noexcept
    {
        return (uint8_t) (_Packet.Data[0] & 0x0F);
    }

    uint16_t Status() const noexcept override final
    {
        return (uint16_t) (_Packet.Data[1] & 0xF0);
    }
};

class sysex7_message_t : public message_t
{
public:
    sysex7_message_t() = default;
    sysex7_message_t(const ump_t & packet) noexcept : message_t(packet) { }

    uint8_t Group() const noexcept
    {
        return (uint8_t) (_Packet.Data[0] & 0x0F);
    }

    uint8_t Size() const noexcept
    {
        return (uint8_t) (_Packet.Data[1] & 0x0F);
    }
};

class sysex8_message_t : public message_t
{
public:
    sysex8_message_t() = default;
    sysex8_message_t(const ump_t & packet) noexcept : message_t(packet) { }

    uint8_t Group() const noexcept
    {
        return (uint8_t) (_Packet.Data[0] & 0x0F);
    }

    uint8_t Size() const noexcept
    {
        return (uint8_t) (_Packet.Data[1] & 0x0F);
    }

    uint8_t StreamId() const noexcept
    {
        return _Packet.Data[2];
    }
};

class mixed_data_set_message_t : public message_t
{
public:
    mixed_data_set_message_t() = default;
    mixed_data_set_message_t(const ump_t & packet) noexcept : message_t(packet) { }

    uint8_t Group() const noexcept
    {
        return (uint8_t) (_Packet.Data[0] & 0x0F);
    }

    uint8_t Id() const noexcept
    {
        return (uint8_t) (_Packet.Data[1] & 0x0F);
    }

    uint16_t ChunkSize() const noexcept
    {
        return (uint16_t) ((_Packet.Data[2] << 8) | _Packet.Data[3]);
    }

    uint16_t ChunkCount() const noexcept
    {
        return (uint16_t) ((_Packet.Data[4] << 8) | _Packet.Data[5]);
    }

    uint16_t ChunkNumber() const noexcept
    {
        return (uint16_t) ((_Packet.Data[6] << 8) | _Packet.Data[7]);
    }

    uint16_t ManufacturerId() const noexcept
    {
        return (uint16_t) ((_Packet.Data[8] << 8) | _Packet.Data[9]);
    }

    uint16_t DeviceId() const noexcept
    {
        return (uint16_t) ((_Packet.Data[10] << 8) | _Packet.Data[11]);
    }

    uint16_t SubId1() const noexcept
    {
        return (uint16_t) ((_Packet.Data[12] << 8) | _Packet.Data[13]);
    }

    uint16_t SubId2() const noexcept
    {
        return (uint16_t) ((_Packet.Data[14] << 8) | _Packet.Data[15]);
    }
};

class midi2_channel_voice_message_t : public message_t
{
public:
    midi2_channel_voice_message_t() = default;
    midi2_channel_voice_message_t(const ump_t & packet) noexcept : message_t(packet) { }

    uint8_t Group() const noexcept
    {
        return (uint8_t) (_Packet.Data[0] & 0x0F);
    }

    uint16_t Status() const noexcept override final
    {
        return (uint16_t) (_Packet.Data[1] & 0xF0);
    }

    uint8_t Channel() const noexcept
    {
        return (uint16_t) (_Packet.Data[1] & 0x0F);
    }

    uint16_t Index() const noexcept
    {
        return (uint16_t) ((_Packet.Data[2] << 8) | _Packet.Data[3]);
    }
};

class flex_data_message_t : public message_t
{
public:
    flex_data_message_t() = default;
    flex_data_message_t(const ump_t & packet) noexcept : message_t(packet) { }

    uint8_t Group() const noexcept
    {
        return (uint8_t) (_Packet.Data[0] & 0x0F);
    }

    uint8_t Format() const noexcept
    {
        return (uint8_t) ((_Packet.Data[1] & 0xC0) >> 6);
    }

    uint8_t Address() const noexcept
    {
        return (uint8_t) ((_Packet.Data[1] & 0x30) >> 4);
    }

    uint8_t Channel() const noexcept
    {
        return (uint8_t) (_Packet.Data[1] & 0x0F);
    }

    uint8_t StatusBank() const noexcept
    {
        return _Packet.Data[2];
    }

    uint16_t Status() const noexcept override final
    {
        return (uint16_t) _Packet.Data[3];
    }
};

class stream_message_t : public message_t
{
public:
    stream_message_t() = default;
    stream_message_t(const ump_t & packet) noexcept : message_t(packet) { }

    uint8_t Format() const noexcept
    {
        return (uint8_t) ((_Packet.Data[0] & 0x0C) >> 2);
    }

    uint16_t Status() const noexcept override final
    {
        return (uint16_t) (((_Packet.Data[0] & 0x03) << 8) | _Packet.Data[1]);
    }
};

}
