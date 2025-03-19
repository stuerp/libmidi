
/** $VER: MIDIProcessorXMF.cpp (2025.03.19) Extensible Music Format (https://www.midi.org/specifications/file-format-specifications/xmf-extensible-music-format/extensible-music-format-xmf-2) **/

#include "framework.h"

#include "MIDIProcessor.h"
#include "Encoding.h"

#undef WINAPI

#include <zlib.h>

enum StringFormatID
{
    ExtendedAsciiVisible = 0,       // Extended ASCII, visible to the user
    ExtendedAsciiHidden = 1,        // Extended ASCII, hidden from the user

    UnicodeVisible = 2,             // Unicode, visible to the user
    UnicodeHidden = 3,              // Unicode, hidden from the user

    CompressedUnicodeVisible = 4,   // Compressed Unicode, visible to the user
    CompressedUnicodeHidden = 5,    // Compressed Unicode, hidden from the user

    BinaryDataVisible = 6,          // Binary data, visible to the user
    BinaryDataHidden = 7,           // Binary data, hidden from the user
};

enum FieldSpecifierID
{
    XMFFileType = 0,                // XMF File Type including the revision ID (only valid in the root node).

    NodeName = 1,                   // Node name, unique
    NodeIDNumber = 2,               // Node ID number

    ResourceFormat = 3,             // Resource format (SMF, DLS, WAV, ...)

    FilenameOnDisk = 4,             // Filename on disk including filename extension.
    FilenameExtensionOnDisk = 5,    // Filename extention on disk

    MacOSFileTypeAndCreator = 6,    // MacOS file type and creator

    MimeType = 7,                   // MIME type

    Title = 8,                      // Title
    CopyrightNotice = 9,            // Copyright notice
    Comment = 10,                   // Comment

    Autostart = 11,                 // Node Name of the FileNode containing the SMF image to autostart when the XMF file loads
    Preload = 12,                   // Used to pre-load specific SMF and DLS file images.

    ContentDescription = 13,        // Used to characterize the set of resources needed to play an SMF file in the Mobile XMF document, (RP-42a, Mobile XMF Content Format Specification, December 2006)
    ID3Metadata = 14,               // Contains a block of ID3v2.4.0 Metadata (RP-47, ID3 Metadata for XMF Files)

    Custom = -1,
};

enum ResourceFormatID
{
    Standard = 0,
    MMAManufacturer = 1,
    Registered = 2,
    NonRegistered = 3,
};

enum StandardResourceFormatID
{
    StandardMidiFileType0 = 0,                  // Standard MIDI File (SMF), Type 0
    StandardMidiFileType1 = 1,                  // Standard MIDI File (SMF), Type 1

    DownloadableSoundsLevel1 = 2,               // Downloadable Sounds (DLS), Level 1
    DownloadableSoundsLevel2 = 3,               // Downloadable Sounds (DLS), Level 2
    DownloadableSoundsLevel2_1 = 4,             // Downloadable Sounds (DLS), Level 2.1

    MobileDownloadableSoundsInstrumentFile = 5, // 

    InvalidStandardResourceFormat = -1,
};

enum UnpackerID
{
    None = 0,
    MMAManufacturerUnpacker = 1,
    RegisteredUnpacker = 2,
    NonRegisteredUnpacker = 3,
};

enum StandardUnpackerID
{
    NoUnpacker = 0,
    Zlib = 1,

    InvalidUnpacker = -1,
};

enum ReferenceTypeID
{
    Invalid = 0,

    InLineResource = 1,
    InFileResource = 2,
    InFileNode = 3,

    ExternalResourceFile = 4,
    ExternalXMFResource = 5,

    XMFFileURIandNodeID = 6,
};

struct xmf_metadata_type_t
{
    uint32_t ID;
    StringFormatID StringFormat;
    std::string Language;                       // https://www.rfc-editor.org/rfc/rfc3282
};

struct xmf_international_content_t
{
    uint32_t ID;
    std::vector<uint8_t> Data;
};

struct xmf_metadata_item_t
{
    FieldSpecifierID ID;
    std::string Name;

    StringFormatID UniversalContentsFormat;
    std::vector<uint8_t> UniversalContentsData;

    std::vector<xmf_international_content_t> _InternationalContents;
};

struct xmf_unpacker_t
{
    UnpackerID ID;
    StandardUnpackerID StandardUnpackerID;
    int ManufacturerID;
    int InternalUnpackerID;
    size_t UnpackedSize;
};

struct xmf_reference_type_t
{
    ReferenceTypeID ID;
    size_t Offset;
    std::string Uri;
};

struct xmf_node_t
{
    size_t Size;        // Size of the complete node (in bytes)
    size_t HeaderSize;  // Relative offset to the node contents (in bytes)
    size_t ItemCount;   // 0 for a FileNode, or count for a FolderNode

    std::string Name;

    uint32_t XMFFileTypeID;
    uint32_t XMFFileTypeRevisionID;

    std::vector<xmf_metadata_item_t> MetaData;
    std::vector<xmf_unpacker_t> Unpackers;

    xmf_reference_type_t ReferenceType;

    std::vector<xmf_node_t> Children;

    std::vector<uint8_t> Unpack(const std::vector<uint8_t> & data);
};

struct xmf_file_t
{
    std::string XMFMetaFileVersion;

    uint32_t XMFFileTypeID;
    uint32_t XMFFileTypeRevisionID;

    uint32_t Size;
};

const size_t MagicSize = 4;

/// <summary>
/// Returns true if the byte vector contains XMF data.
/// </summary>
bool midi_processor_t::IsXMF(std::vector<uint8_t> const & data) noexcept
{
    if (data.size() < MagicSize)
        return false;

    if (data[ 0] != 'X' || data[ 1] != 'M' || data[ 2] != 'F' || data[ 3] != '_')
        return false;

    return true;
}

/// <summary>
/// Processes a byte vector with XMF data.
/// </summary>
bool midi_processor_t::ProcessXMF(std::vector<uint8_t> const & data, midi_container_t & container)
{
    xmf_file_t File = { };
    midi_metadata_table_t Metadata;

    auto Head = data.begin();
    auto Tail = data.end();

    auto Data = Head + MagicSize;

    // Read the XMFMetaFileVersion which indicates the version of the XMF Meta-File Format Specification being used.
    File.XMFMetaFileVersion = std::string(Data, Data + 4);
    Data += 4;

    Metadata.AddItem(midi_metadata_item_t(0, "xmf_meta_file_version", File.XMFMetaFileVersion.c_str()));

    // RP-043: XMF Meta File Format 2.0, September 2004
    if (std::atof(File.XMFMetaFileVersion.c_str()) >= 2.0f)
    {
        char s[16];;

        // Read the XMFFileTypeID: XMF Type 2 file (Mobile XMF)
        File.XMFFileTypeID = (uint32_t) ((Data[0] << 24) | (Data[1] << 16) | (Data[2] << 8) | Data[3]);
        Data += 4;

        ::sprintf_s(s, _countof(s), "%d", File.XMFFileTypeID);

        Metadata.AddItem(midi_metadata_item_t(0, "xmf_file_type", s));

        // Read the XMFFileTypeRevisionID: Version 1 of Mobile XMF spec
        File.XMFFileTypeRevisionID = (uint32_t) ((Data[0] << 24) | (Data[1] << 16) | (Data[2] << 8) | Data[3]);
        Data += 4;

        ::sprintf_s(s, _countof(s), "%d", File.XMFFileTypeRevisionID);

        Metadata.AddItem(midi_metadata_item_t(0, "xmf_file_type_revision", s));
    }

    File.Size = (uint32_t) DecodeVariableLengthQuantity(Data, Tail);

    // Read the MetadataTypesTable if present.
    const uint32_t TableSize = (uint32_t) DecodeVariableLengthQuantity(Data, Tail); // Total node length in bytes, including NodeContents

    if (TableSize != 0)
        throw midi_exception("XMF MetadataTypesTable is not yet supported");

    const auto TreeStart = DecodeVariableLengthQuantity(Data, Tail);
//  const auto TreeEnd   = DecodeVariableLengthQuantity(Data, Tail);

    // Move to the start of the tree.
    Data = data.begin() + TreeStart;

    // Read the tree.
    ProcessNode(Head, Tail, Data, Metadata, container);

    container.SetExtraMetaData(Metadata);

    return true;
}

/// <summary>
/// Processes a tree node.
/// </summary>
bool midi_processor_t::ProcessNode(std::vector<uint8_t>::const_iterator & head, std::vector<uint8_t>::const_iterator tail, std::vector<uint8_t>::const_iterator & data, midi_metadata_table_t & metadata, midi_container_t & container)
{
    const std::vector<uint8_t>::const_iterator HeaderHead = data;

    xmf_node_t Node = {};

    // Read the node header.
    Node.Size = (size_t) DecodeVariableLengthQuantity(data, tail);
    Node.ItemCount = (size_t) DecodeVariableLengthQuantity(data, tail);
    Node.HeaderSize = (size_t) DecodeVariableLengthQuantity(data, tail);

    auto StandardResourceFormat = StandardResourceFormatID::InvalidStandardResourceFormat;

    // Read the metadata.
    {
//      const auto MetaDataHead = data;
        const size_t MetaDataSize = (size_t) DecodeVariableLengthQuantity(data, tail);
        const auto MetaDataTail = data + (ptrdiff_t) MetaDataSize;

        while (data < MetaDataTail)
        {
            xmf_metadata_item_t MetadataItem = {};

            {
                const size_t Size = (size_t) DecodeVariableLengthQuantity(data, tail);

                if (Size == 0)
                {
                    MetadataItem.ID = (FieldSpecifierID) DecodeVariableLengthQuantity(data, tail);
                }
                else
                {
                    MetadataItem.ID = FieldSpecifierID::Custom;
                    MetadataItem.Name = std::string(data, data + (ptrdiff_t) Size);
                    data += (ptrdiff_t) Size;
                }
            }

            {
                const size_t InternationalContentsCount = (size_t) DecodeVariableLengthQuantity(data, tail);

                if (InternationalContentsCount == 0)
                {
                    // Get the universal content.
                    const size_t Size = (size_t) DecodeVariableLengthQuantity(data, tail);

                    if (Size > 0)
                    {
                        MetadataItem.UniversalContentsFormat = (StringFormatID) DecodeVariableLengthQuantity(data, tail);
                        MetadataItem.UniversalContentsData.assign(data, data + (ptrdiff_t) Size - 1);
                        data += (ptrdiff_t) Size - 1;

                        // Interpret the universal content.
                        auto Head = MetadataItem.UniversalContentsData.begin();
                        auto Tail = MetadataItem.UniversalContentsData.end();

                        switch (MetadataItem.ID)
                        {
                            case FieldSpecifierID::XMFFileType:
                            {
                                Node.XMFFileTypeID         = (uint32_t) DecodeVariableLengthQuantity(Head, Tail);
                                Node.XMFFileTypeRevisionID = (uint32_t) DecodeVariableLengthQuantity(Head, Tail);
                                break;
                            }

                            case FieldSpecifierID::NodeName:
                            {
                                Node.Name = std::string(Head, Tail);
                                break;
                            }

                            case FieldSpecifierID::NodeIDNumber:
                            {
                                break;
                            }

                            case FieldSpecifierID::ResourceFormat:
                            {
                                auto ResourceFormat = (ResourceFormatID) DecodeVariableLengthQuantity(Head, MetadataItem.UniversalContentsData.end());

                                if (ResourceFormat == ResourceFormatID::Standard)
                                {
                                    StandardResourceFormat = (StandardResourceFormatID) DecodeVariableLengthQuantity(Head, MetadataItem.UniversalContentsData.end());
                                }
                                else
                                    ; // Not yet supported.
                                break;
                            }

                            case FieldSpecifierID::FilenameOnDisk:
                            {
                                break;
                            }

                            case FieldSpecifierID::FilenameExtensionOnDisk:
                            {
                                break;
                            }

                            case FieldSpecifierID::MacOSFileTypeAndCreator:
                            case FieldSpecifierID::MimeType:
                            {
                                break;
                            }

                            case FieldSpecifierID::Title:
                            case FieldSpecifierID::CopyrightNotice:
                            case FieldSpecifierID::Comment:
                            case FieldSpecifierID::Autostart:
                            case FieldSpecifierID::Preload:
                            case FieldSpecifierID::ContentDescription:
                            case FieldSpecifierID::ID3Metadata:

                            case FieldSpecifierID::Custom:
                            {
                                break;
                            }

                            default:
                                break;
                        }
                    }
                }
                else
                {
                    throw midi_exception("International XMF content is not yet supported");
/*
                    // Get the international content.
                    for (int i = 0; i < InternationalContentsCount; ++i)
                    {
                        int MetaDataTypeID = DecodeVariableLengthQuantity(data, tail);

                        const size_t Size = DecodeVariableLengthQuantity(data, tail);

                        data += Size;
                    }
*/
                }
            }

            Node.MetaData.push_back(MetadataItem);
        }
    }

    // Read the unpackers.
    {
//      const auto UnpackersHead = data;
        const size_t UnpackersLength = (size_t) DecodeVariableLengthQuantity(data, tail);
        const auto UnpackersTail = data + (ptrdiff_t) UnpackersLength;

        while (data < UnpackersTail)
        {
            xmf_unpacker_t Unpacker = {};

            Unpacker.ID = (UnpackerID) DecodeVariableLengthQuantity(data, tail);

            switch (Unpacker.ID)
            {
                case UnpackerID::None:
                {
                    Unpacker.StandardUnpackerID = (StandardUnpackerID) DecodeVariableLengthQuantity(data, tail);
                    break;
                }

                case UnpackerID::MMAManufacturerUnpacker:
                {
                    int ManufacturerID = *data++;

                    if (ManufacturerID == 0)
                    {
                        ManufacturerID <<= 8; ManufacturerID |= *data++;
                        ManufacturerID <<= 8; ManufacturerID |= *data++;
                    }

                    Unpacker.ManufacturerID     = ManufacturerID;
                    Unpacker.InternalUnpackerID = DecodeVariableLengthQuantity(data, tail);
                    break;
                }

                case UnpackerID::RegisteredUnpacker:
                case UnpackerID::NonRegisteredUnpacker:
                {
                    throw midi_exception("Unsuppored XMF unpacker");
                }

                default:
                    throw midi_exception("Unknown XMF unpacker");
            }

            Unpacker.UnpackedSize = (size_t) DecodeVariableLengthQuantity(data, tail);

            Node.Unpackers.push_back(Unpacker);
        }
    }

    // Read the reference type.
    {
        data = HeaderHead + (ptrdiff_t) Node.HeaderSize;

        Node.ReferenceType.ID = (ReferenceTypeID) DecodeVariableLengthQuantity(data, tail);

        switch (Node.ReferenceType.ID)
        {
            case ReferenceTypeID::InLineResource:
            {
                Node.ReferenceType.Offset = (size_t) (data - head);
                break;
            }

            case ReferenceTypeID::InFileResource:
            case ReferenceTypeID::InFileNode:
            case ReferenceTypeID::ExternalResourceFile:
            case ReferenceTypeID::ExternalXMFResource:
            case ReferenceTypeID::XMFFileURIandNodeID:
                throw midi_exception("Unsupported XMF reference type");

            case ReferenceTypeID::Invalid:
            default:
                throw midi_exception("Unknown XMF reference type");
        }
    }

    // Read the content.
    {
        if (Node.ItemCount == 0)
        {
            // File node
            size_t Size = Node.Size - Node.HeaderSize - 1;

            std::vector<uint8_t> Data(data, data + (ptrdiff_t) Size);
            std::vector<uint8_t> UnpackedData;

            switch (StandardResourceFormat)
            {
                case StandardResourceFormatID::StandardMidiFileType0:
                case StandardResourceFormatID::StandardMidiFileType1:
                {
                    UnpackedData = Node.Unpack(Data);

                    ProcessSMF(UnpackedData, container);
                    break;
                }

                case StandardResourceFormatID::DownloadableSoundsLevel1:
                case StandardResourceFormatID::DownloadableSoundsLevel2:
                case StandardResourceFormatID::DownloadableSoundsLevel2_1:
                case StandardResourceFormatID::MobileDownloadableSoundsInstrumentFile:
                {
                    UnpackedData = Node.Unpack(Data);

                    container.SetSoundFontData(UnpackedData);
                    break;
                }

                case StandardResourceFormatID::InvalidStandardResourceFormat:
                default:
                    ; // Not yet supported.
            }

            data += (ptrdiff_t) Size;
        }
        else
        {
            // Folder node
            switch (Node.ReferenceType.ID)
            {
                case ReferenceTypeID::InLineResource:
                case ReferenceTypeID::InFileResource:
                {
                    auto Data = head + (ptrdiff_t) Node.ReferenceType.Offset;

                    for (size_t i = 0; i < Node.ItemCount; ++i)
                    {
                        ProcessNode(head, tail, Data, metadata, container);
                    }
                    break;
                }

                case ReferenceTypeID::InFileNode:
                case ReferenceTypeID::ExternalResourceFile:
                case ReferenceTypeID::ExternalXMFResource:
                case ReferenceTypeID::XMFFileURIandNodeID:
                case ReferenceTypeID::Invalid:
                default:
                    throw midi_exception("Unsupported XMF reference type");
            }
        }
    }

    return true;
}

/// <summary>
/// Unpackes the specified data, if necessary.
/// </summary>
std::vector<uint8_t> xmf_node_t::Unpack(const std::vector<uint8_t> & data)
{
    if (Unpackers.size() == 0)
        return data;

    const auto & Unpacker = Unpackers[0];
    std::vector<uint8_t> UnpackedData;

    if (Unpacker.StandardUnpackerID == StandardUnpackerID::Zlib)
    {
        UnpackedData.resize(Unpacker.UnpackedSize);

        midi_processor_t::Inflate(data, UnpackedData);
    }
    else
    if (Unpacker.InternalUnpackerID != 0)
    {
        if ((data.size() > 2) && (data[0] == 0x78) && (data[1] == 0xDA))
        {
            UnpackedData.resize(Unpacker.UnpackedSize);

            midi_processor_t::InflateRaw(data, UnpackedData);
        }
        else
            throw midi_exception(FormatText("Unknown unpacker 0x%02X from MMA manufacturer 0x%06X",  Unpacker.InternalUnpackerID,  Unpacker.ManufacturerID));
    }

    return UnpackedData;
}

/// <summary>
/// Inflates a zlib deflated data stream.
/// </summary>
int midi_processor_t::Inflate(const std::vector<uint8_t> & src, std::vector<uint8_t> & dst) noexcept
{
    z_stream Stream = { };

    Stream.total_in = Stream.avail_in = (uInt) src.size();
    Stream.next_in = (Bytef *) src.data();

    Stream.total_out = Stream.avail_out = (uInt) dst.size();
    Stream.next_out = (Bytef *) dst.data();

    int Status = ::inflateInit2(&Stream, MAX_WBITS);

    if (Status == Z_OK)
    {
        Status = ::inflate(&Stream, Z_FINISH);

        if (Status == Z_STREAM_END)
            Status = Z_OK;
    }

    ::inflateEnd(&Stream);

    return Status;
}

/// <summary>
/// Inflates a raw deflated data stream.
/// </summary>
int midi_processor_t::InflateRaw(const std::vector<uint8_t> & src, std::vector<uint8_t> & dst) noexcept
{
    z_stream Stream = { };

    Stream.avail_in = (uInt) src.size();
    Stream.next_in = (Bytef *) src.data();

    Stream.avail_out = (uInt) dst.size();
    Stream.next_out = (Bytef *) dst.data();

    int Status = ::inflateInit(&Stream);

    if (Status == Z_OK)
    {
        Status = ::inflate(&Stream, Z_NO_FLUSH);

        if (Status == Z_STREAM_END)
            Status = Z_OK;
    }

    ::inflateEnd(&Stream);

    return Status;
}
