#include "pst/PlayerData.h"
#include <fstream>
#include <cstring>
#include <iostream>
#include <algorithm>
#include <sstream>
#include <iomanip>

namespace PSM {

PlayerSavParser::PlayerSavParser() {}
PlayerSavParser::~PlayerSavParser() {}

bool PlayerSavParser::parse(const std::string& filePath) {
    std::ifstream file(filePath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) return false;

    size_t fileSize = file.tellg();
    file.seekg(0);

    std::vector<uint8_t> rawData(fileSize);
    file.read(reinterpret_cast<char*>(rawData.data()), fileSize);
    file.close();

    return parseFromBuffer(rawData);
}

bool PlayerSavParser::parseFromBuffer(const std::vector<uint8_t>& data) {
    if (data.size() < 16) return false;

    // Palworld .sav files have a header followed by compressed GVAS data
    // Header format:
    //   [4 bytes: uncompressed_size]
    //   [4 bytes: compressed_size]
    //   [2 bytes: magic / compression_type]
    //   [remaining: compressed data]

    // Check for raw GVAS first
    if (data.size() >= 4 && data[0] == 'G' && data[1] == 'V' && data[2] == 'A' && data[3] == 'S') {
        size_t offset = 0;
        return parseGvasProperties(data, offset);
    }

    // Try decompression
    std::vector<uint8_t> decompressed;
    if (!decompressData(data, decompressed)) {
        // If decompression fails, try parsing raw
        size_t offset = 0;
        return parseGvasProperties(data, offset);
    }

    size_t offset = 0;
    return parseGvasProperties(decompressed, offset);
}

bool PlayerSavParser::decompressData(const std::vector<uint8_t>& compressed,
                                      std::vector<uint8_t>& decompressed) {
    if (compressed.size() < 12) return false;

    // Read header
    uint32_t uncompressedSize;
    uint32_t compressedSize;
    memcpy(&uncompressedSize, compressed.data(), 4);
    memcpy(&compressedSize, compressed.data() + 4, 4);

    if (uncompressedSize == 0 || uncompressedSize > 512 * 1024 * 1024) return false;

    // The actual decompression would use zlib or the Palworld-specific
    // compression format. Palworld uses a custom double-compression scheme:
    // 1. The save file has a custom header
    // 2. Followed by zlib-compressed data
    // 3. Which contains GVAS format data

    // Placeholder - in production, integrate zlib:
    // decompressed.resize(uncompressedSize);
    // z_stream stream = {};
    // inflateInit(&stream);
    // stream.avail_in = compressedSize;
    // stream.next_in = compressed.data() + headerSize;
    // stream.avail_out = uncompressedSize;
    // stream.next_out = decompressed.data();
    // inflate(&stream, Z_FINISH);
    // inflateEnd(&stream);

    return false; // Stub until zlib integration
}

bool PlayerSavParser::parseGvasProperties(const std::vector<uint8_t>& data, size_t& offset) {
    if (data.size() < offset + 4) return false;

    // GVAS header
    // Magic: GVAS (4 bytes)
    if (data[offset] == 'G' && data[offset + 1] == 'V' &&
        data[offset + 2] == 'A' && data[offset + 3] == 'S') {
        offset += 4;

        // Save game version (4 bytes)
        int32_t saveGameVersion = readInt32(data, offset);

        // Package version (4 bytes)
        int32_t packageVersion = readInt32(data, offset);

        // Engine version - Major.Minor.Patch.Changelist
        int32_t engineMajor = readInt32(data, offset);
        int32_t engineMinor = readInt32(data, offset);
        int32_t enginePatch = readInt32(data, offset);
        int32_t engineChangelist = readInt32(data, offset);

        // Engine version string
        std::string engineVersionStr = readFString(data, offset);

        // Custom format version (4 bytes)
        int32_t customFormatVersion = readInt32(data, offset);

        // Custom format data count
        int32_t customFormatCount = readInt32(data, offset);

        // Skip custom format entries
        for (int32_t i = 0; i < customFormatCount && offset < data.size(); ++i) {
            // GUID (16 bytes)
            offset += 16;
            // Entry (4 bytes)
            offset += 4;
        }

        // Save game type string
        std::string saveGameType = readFString(data, offset);
    }

    // Now read properties until "None"
    m_rawProperties = nlohmann::json::object();

    while (offset < data.size()) {
        if (!readProperty(data, offset)) break;
    }

    return true;
}

bool PlayerSavParser::readProperty(const std::vector<uint8_t>& data, size_t& offset) {
    if (offset + 4 > data.size()) return false;

    // Property name
    std::string name = readFString(data, offset);
    if (name.empty() || name == "None") return false;

    // Property type
    std::string typeName = readFString(data, offset);

    // Property size
    int64_t propertySize = readInt64(data, offset);

    // Property index (for arrays)
    // int32_t index = readInt32(data, offset);

    if (typeName == "BoolProperty") {
        // Bool has special encoding: value is in the "index" field
        // Skip 1 byte (the bool value before the typical GUID marker)
        bool value = (data[offset] != 0);
        offset++;
        // Optional GUID (1 byte flag + 16 bytes if present)
        uint8_t hasGuid = data[offset++];
        if (hasGuid) offset += 16;

        m_rawProperties[name] = value;

        // Map known properties
        if (name == "IsPlayer") m_player.online = value;
    }
    else if (typeName == "IntProperty") {
        uint8_t hasGuid = data[offset++];
        if (hasGuid) offset += 16;
        int32_t value = readInt32(data, offset);
        m_rawProperties[name] = value;

        if (name == "Level") m_player.stats.level = value;
        else if (name == "Hp") m_player.stats.hp = value;
        else if (name == "MaxHp") m_player.stats.maxHp = value;
    }
    else if (typeName == "Int64Property") {
        uint8_t hasGuid = data[offset++];
        if (hasGuid) offset += 16;
        int64_t value = readInt64(data, offset);
        m_rawProperties[name] = value;

        if (name == "Exp") m_player.stats.exp = value;
    }
    else if (typeName == "FloatProperty") {
        uint8_t hasGuid = data[offset++];
        if (hasGuid) offset += 16;
        float value = readFloat(data, offset);
        m_rawProperties[name] = value;
    }
    else if (typeName == "StrProperty" || typeName == "NameProperty") {
        uint8_t hasGuid = data[offset++];
        if (hasGuid) offset += 16;
        std::string value = readFString(data, offset);
        m_rawProperties[name] = value;

        if (name == "NickName" || name == "PlayerName") m_player.name = value;
        else if (name == "PlayerUId") m_player.uid = value;
    }
    else if (typeName == "StructProperty") {
        // Struct type name
        std::string structTypeName = readFString(data, offset);
        // GUID (16 bytes)
        offset += 16;
        uint8_t hasGuid = data[offset++];
        if (hasGuid) offset += 16;

        // Read struct properties recursively
        // For simplicity, skip based on property size
        size_t endOffset = offset + static_cast<size_t>(propertySize);
        if (endOffset <= data.size()) {
            offset = endOffset;
        }
    }
    else if (typeName == "ArrayProperty") {
        std::string innerType = readFString(data, offset);
        uint8_t hasGuid = data[offset++];
        if (hasGuid) offset += 16;

        int32_t arrayCount = readInt32(data, offset);

        // Skip array contents based on size
        size_t remaining = static_cast<size_t>(propertySize) - 4; // minus the count
        if (offset + remaining <= data.size()) {
            // Process array elements based on innerType
            // For technology unlocks:
            if (name == "UnlockedRecipeTechnologyNames" && innerType == "NameProperty") {
                for (int32_t i = 0; i < arrayCount && offset < data.size(); ++i) {
                    std::string techName = readFString(data, offset);
                    if (!techName.empty()) {
                        m_unlockedTechs.push_back(techName);
                    }
                }
            } else {
                offset += remaining;
            }
        }
    }
    else if (typeName == "MapProperty") {
        // Key and value types
        std::string keyType = readFString(data, offset);
        std::string valueType = readFString(data, offset);
        uint8_t hasGuid = data[offset++];
        if (hasGuid) offset += 16;

        // Skip map based on size
        size_t endOffset = offset + static_cast<size_t>(propertySize);
        if (endOffset <= data.size()) {
            offset = endOffset;
        }
    }
    else if (typeName == "EnumProperty") {
        std::string enumType = readFString(data, offset);
        uint8_t hasGuid = data[offset++];
        if (hasGuid) offset += 16;
        std::string value = readFString(data, offset);
        m_rawProperties[name] = value;
    }
    else if (typeName == "TextProperty") {
        uint8_t hasGuid = data[offset++];
        if (hasGuid) offset += 16;
        // Skip text property based on size
        size_t endOffset = offset + static_cast<size_t>(propertySize);
        if (endOffset <= data.size()) {
            offset = endOffset;
        }
    }
    else if (typeName == "SoftObjectProperty" || typeName == "ObjectProperty") {
        uint8_t hasGuid = data[offset++];
        if (hasGuid) offset += 16;
        std::string value = readFString(data, offset);
        m_rawProperties[name] = value;
    }
    else {
        // Unknown property type - skip based on size
        uint8_t hasGuid = data[offset++];
        if (hasGuid) offset += 16;
        size_t endOffset = offset + static_cast<size_t>(propertySize);
        if (endOffset <= data.size()) {
            offset = endOffset;
        }
    }

    return true;
}

std::string PlayerSavParser::readFString(const std::vector<uint8_t>& data, size_t& offset) {
    if (offset + 4 > data.size()) return "";

    int32_t length;
    memcpy(&length, data.data() + offset, 4);
    offset += 4;

    if (length == 0) return "";

    bool isUnicode = (length < 0);
    size_t byteLength;
    if (isUnicode) {
        length = -length;
        byteLength = static_cast<size_t>(length) * 2;
    } else {
        byteLength = static_cast<size_t>(length);
    }

    if (offset + byteLength > data.size()) {
        offset = data.size();
        return "";
    }

    std::string result;
    if (isUnicode) {
        // Convert UTF-16LE to UTF-8 (simplified - just take low bytes)
        result.reserve(length);
        for (int32_t i = 0; i < length - 1; ++i) {
            char16_t ch;
            memcpy(&ch, data.data() + offset + i * 2, 2);
            if (ch < 0x80) {
                result += static_cast<char>(ch);
            } else {
                result += '?';
            }
        }
        offset += byteLength;
    } else {
        result = std::string(reinterpret_cast<const char*>(data.data() + offset), byteLength);
        offset += byteLength;
        // Remove null terminator if present
        if (!result.empty() && result.back() == '\0') {
            result.pop_back();
        }
    }

    return result;
}

int32_t PlayerSavParser::readInt32(const std::vector<uint8_t>& data, size_t& offset) {
    if (offset + 4 > data.size()) return 0;
    int32_t value;
    memcpy(&value, data.data() + offset, 4);
    offset += 4;
    return value;
}

int64_t PlayerSavParser::readInt64(const std::vector<uint8_t>& data, size_t& offset) {
    if (offset + 8 > data.size()) return 0;
    int64_t value;
    memcpy(&value, data.data() + offset, 8);
    offset += 8;
    return value;
}

float PlayerSavParser::readFloat(const std::vector<uint8_t>& data, size_t& offset) {
    if (offset + 4 > data.size()) return 0.0f;
    float value;
    memcpy(&value, data.data() + offset, 4);
    offset += 4;
    return value;
}

bool PlayerSavParser::readBool(const std::vector<uint8_t>& data, size_t& offset) {
    if (offset >= data.size()) return false;
    bool value = (data[offset] != 0);
    offset++;
    return value;
}

std::string PlayerSavParser::readGuid(const std::vector<uint8_t>& data, size_t& offset) {
    if (offset + 16 > data.size()) return "";

    uint32_t a, b, c, d;
    memcpy(&a, data.data() + offset, 4);
    memcpy(&b, data.data() + offset + 4, 4);
    memcpy(&c, data.data() + offset + 8, 4);
    memcpy(&d, data.data() + offset + 12, 4);
    offset += 16;

    std::ostringstream ss;
    ss << std::hex << std::setfill('0')
       << std::setw(8) << a << "-"
       << std::setw(4) << (b >> 16) << "-"
       << std::setw(4) << (b & 0xFFFF) << "-"
       << std::setw(4) << (c >> 16) << "-"
       << std::setw(4) << (c & 0xFFFF)
       << std::setw(8) << d;

    return ss.str();
}

Player PlayerSavParser::getPlayerData() const {
    return m_player;
}

std::vector<Item> PlayerSavParser::getInventory() const {
    return m_inventory;
}

std::vector<Pal> PlayerSavParser::getPals() const {
    return m_pals;
}

PlayerStats PlayerSavParser::getStats() const {
    return m_player.stats;
}

std::vector<std::string> PlayerSavParser::getUnlockedTechs() const {
    return m_unlockedTechs;
}

nlohmann::json PlayerSavParser::toJson() const {
    return m_rawProperties;
}

} // namespace PSM
