
/** $VER: MIDIProcessorXMF.cpp (2025.02.24) Extensible Music Format (https://www.midi.org/specifications/file-format-specifications/xmf-extensible-music-format/extensible-music-format-xmf-2) **/

#include "framework.h"

#include "MIDIProcessor.h"
#include "Encoding.h"

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

    uint32_t XMFFileTypeID;
    uint32_t XMFFileTypeRevisionID;

    std::vector<xmf_metadata_item_t> MetaData;
    std::vector<xmf_unpacker_t> Unpackers;

    xmf_reference_type_t ReferenceType;

    std::vector<xmf_node_t> Children;
};

struct xmf_file_t
{
    std::string Version;

    uint32_t XMFFileTypeID;
    uint32_t XMFFileTypeRevisionID;

    uint32_t Size;
};

const size_t MagicSize = 4;

/// <summary>
/// Returns true if the byte vector contains XMF data.
/// </summary>
bool midi_processor_t::IsXMF(std::vector<uint8_t> const & data)
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

    auto Head = data.begin();
    auto Tail = data.end();

    auto Data = Head + MagicSize;

    // Read the XMFMetaFileVersion which indicates the version of the XMF Meta-File Format Specification being used.
    File.Version = std::string(Data, Data + 4);
    Data += 4;

    if (std::atof(File.Version.c_str()) >= 2.0f)
    {
        // Read the XMFFileTypeID: XMF Type 2 file (Mobile XMF)
        File.XMFFileTypeID = (uint32_t ) ((Data[0] << 24) | (Data[1] << 16) | (Data[2] << 8) | Data[3]);
        Data += 4;

        // Read the XMFFileTypeRevisionID: Version 1 of Mobile XMF spec
        File.XMFFileTypeRevisionID = (uint32_t) ((Data[0] << 24) | (Data[1] << 16) | (Data[2] << 8) | Data[3]);
        Data += 4;
    }

    File.Size = (uint32_t) DecodeVariableLengthQuantity(Data, Tail);

    // Read the MetaDataTypesTable if present.
    const uint32_t TableSize = (uint32_t) DecodeVariableLengthQuantity(Data, Tail); // Total node length in bytes, including NodeContents

    if (TableSize != 0)
        DebugBreak();

    const auto TreeStart = DecodeVariableLengthQuantity(Data, Tail);
//  const auto TreeEnd   = DecodeVariableLengthQuantity(Data, Tail);

    // Move to the start of the tree.
    Data = data.begin() + TreeStart;

    // Read the tree.
    ProcessNode(Head, Tail, Data, container);

    return true;
}

/// <summary>
/// Processes a tree node.
/// </summary>
bool midi_processor_t::ProcessNode(std::vector<uint8_t>::const_iterator & head, std::vector<uint8_t>::const_iterator tail, std::vector<uint8_t>::const_iterator & data, midi_container_t & container)
{
    const std::vector<uint8_t>::const_iterator & HeaderHead = data;

    xmf_node_t Node = {};

    // Read the node header.
    Node.Size = (size_t) DecodeVariableLengthQuantity(data, tail);
    Node.ItemCount = (size_t) DecodeVariableLengthQuantity(data, tail);
    Node.HeaderSize = (size_t) DecodeVariableLengthQuantity(data, tail);

    auto StandardResourceFormat = StandardResourceFormatID::InvalidStandardResourceFormat;

    // Read the metadata.
    {
//      const std::vector<uint8_t>::const_iterator & MetaDataHead = data;
        const size_t MetaDataSize = (size_t) DecodeVariableLengthQuantity(data, tail);
        const std::vector<uint8_t>::const_iterator & MetaDataTail = data + (ptrdiff_t) MetaDataSize;

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

                        switch (MetadataItem.ID)
                        {
                            case FieldSpecifierID::XMFFileType:
                            {
                                auto Data = MetadataItem.UniversalContentsData.begin();

                                Node.XMFFileTypeID         = (uint32_t) DecodeVariableLengthQuantity(Data, MetadataItem.UniversalContentsData.end());
                                Node.XMFFileTypeRevisionID = (uint32_t) DecodeVariableLengthQuantity(Data, MetadataItem.UniversalContentsData.end());
                                break;
                            }

                            case FieldSpecifierID::NodeName:
                            case FieldSpecifierID::NodeIDNumber:
                            {
                                break;
                            }

                            case FieldSpecifierID::ResourceFormat:
                            {
                                auto Data = MetadataItem.UniversalContentsData.begin();

                                auto ResourceFormat = (ResourceFormatID) DecodeVariableLengthQuantity(Data, MetadataItem.UniversalContentsData.end());

                                if (ResourceFormat == ResourceFormatID::Standard)
                                {
                                    StandardResourceFormat = (StandardResourceFormatID) DecodeVariableLengthQuantity(Data, MetadataItem.UniversalContentsData.end());
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
//      const std::vector<uint8_t>::const_iterator & UnpackersHead = data;
        const size_t UnpackersLength = (size_t) DecodeVariableLengthQuantity(data, tail);
        const std::vector<uint8_t>::const_iterator & UnpackersTail = data + (ptrdiff_t) UnpackersLength;

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

                    int InternalUnpackerID = DecodeVariableLengthQuantity(data, tail);

                    throw midi_exception(FormatText("Unknown unpacker 0x%02X from MMA manufacturer 0x%06X", InternalUnpackerID, ManufacturerID));
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

            std::vector<uint8_t> Data;
            std::vector<uint8_t> UnpackedData;

            switch (StandardResourceFormat)
            {
                case StandardResourceFormatID::StandardMidiFileType0:
                case StandardResourceFormatID::StandardMidiFileType1:
                {
                    Data.assign(data, data + (ptrdiff_t) Size);

                    if ((Node.Unpackers.size() > 0) && (Node.Unpackers[0].StandardUnpackerID == StandardUnpackerID::Zlib))
                    {
                        UnpackedData.resize(Node.Unpackers[0].UnpackedSize);

                        Inflate(Data, UnpackedData);

                        ProcessSMF(UnpackedData, container);
                    }
                    else
                        ProcessSMF(Data, container);
                    break;
                }

                case StandardResourceFormatID::DownloadableSoundsLevel1:
                case StandardResourceFormatID::DownloadableSoundsLevel2:
                case StandardResourceFormatID::DownloadableSoundsLevel2_1:
                case StandardResourceFormatID::MobileDownloadableSoundsInstrumentFile:
                {
                    Data.assign(data, data + (ptrdiff_t) Size);

                    if ((Node.Unpackers.size() > 0) && (Node.Unpackers[0].StandardUnpackerID == StandardUnpackerID::Zlib))
                    {
                        UnpackedData.resize(Node.Unpackers[0].UnpackedSize);

                        Inflate(Data, UnpackedData);

                        container.SetSoundFontData(UnpackedData);
                    }
                    else
                        container.SetSoundFontData(Data);
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
                        ProcessNode(head, tail, Data, container);
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
/// Inflates the zlib deflated data.
/// </summary>
int midi_processor_t::Inflate(const std::vector<uint8_t> & src, std::vector<uint8_t> & dst) noexcept
{
    z_stream Stream = { };

    Stream.total_in = Stream.avail_in = (uInt) src.size();
    Stream.next_in = (Bytef *) src.data();

    Stream.total_out = Stream.avail_out = (uInt) dst.size();
    Stream.next_out = (Bytef *) dst.data();

    int Status = ::inflateInit2(&Stream, (15 + 32)); // Use 15 window bits. +32 tells zlib to to detect if using gzip or zlib.

    if (Status == Z_OK)
    {
        Status = ::inflate(&Stream, Z_FINISH);

        if (Status == Z_STREAM_END)
            Status = Z_OK;
    }

    ::inflateEnd(&Stream);

    return Status;
}
