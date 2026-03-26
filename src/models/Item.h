#pragma once

#include <string>
#include <nlohmann/json.hpp>

namespace PSM {

struct Item {
    std::string id;
    std::string staticId;
    std::string name;
    int count = 0;
    int maxStack = 0;
    int durability = 0;
    int maxDurability = 0;
    bool isEquipped = false;
    std::string container; // "inventory", "equipment", "key_items", etc.

    nlohmann::json toJson() const {
        return {
            {"id", id},
            {"staticId", staticId},
            {"name", name},
            {"count", count},
            {"maxStack", maxStack},
            {"container", container}
        };
    }
};

} // namespace PSM
