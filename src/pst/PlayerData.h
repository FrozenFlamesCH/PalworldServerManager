#pragma once

#include <string>
#include <vector>
#include <map>
#include <cstdint>
#include <nlohmann/json.hpp>
#include "models/Player.h"
#include "models/Pal.h"
#include "models/Item.h"

namespace PSM {

// Individual player save data parser
// Palworld stores per-player data in:
//   Pal/Saved/SaveGames/0/<ServerID>/Players/<PlayerUID>.sav

class PlayerSavParser {
public:
    PlayerSavParser();
    ~PlayerSavParser();

    bool parse(const std::string& filePath);
    bool parseFromBuffer(const std::vector<uint8_t>& data);

    Player getPlayerData() const;
    std::vector<Item> getInventory() const;
    std::vector<Pal> getPals() const;
    PlayerStats getStats() const;
    std::vector<std::string> getUnlockedTechs() const;

    // Raw JSON export of all parsed properties
    nlohmann::json toJson() const;

private:
    bool decompressData(const std::vector<uint8_t>& compressed, std::vector<uint8_t>& decompressed);
    bool parseGvasProperties(const std::vector<uint8_t>& data, size_t& offset);

    // GVAS property readers
    bool readProperty(const std::vector<uint8_t>& data, size_t& offset);
    std::string readFString(const std::vector<uint8_t>& data, size_t& offset);
    int32_t readInt32(const std::vector<uint8_t>& data, size_t& offset);
    int64_t readInt64(const std::vector<uint8_t>& data, size_t& offset);
    float readFloat(const std::vector<uint8_t>& data, size_t& offset);
    bool readBool(const std::vector<uint8_t>& data, size_t& offset);
    std::string readGuid(const std::vector<uint8_t>& data, size_t& offset);

    Player m_player;
    std::vector<Item> m_inventory;
    std::vector<Pal> m_pals;
    std::vector<std::string> m_unlockedTechs;
    nlohmann::json m_rawProperties;
};

} // namespace PSM
