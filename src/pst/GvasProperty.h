#pragma once

#include <string>
#include <vector>
#include <variant>
#include <map>
#include <cstdint>
#include <memory>

namespace PSM {

// Simplified GVAS property system for reading Palworld save files
enum class GvasType {
    None,
    BoolProperty,
    IntProperty,
    Int64Property,
    FloatProperty,
    StrProperty,
    NameProperty,
    EnumProperty,
    StructProperty,
    ArrayProperty,
    MapProperty,
    SetProperty,
    SoftObjectProperty,
    ObjectProperty,
    TextProperty
};

struct GvasProperty;
using GvasPropertyPtr = std::shared_ptr<GvasProperty>;

struct GvasProperty {
    std::string name;
    GvasType type = GvasType::None;
    std::string typeName;
    size_t size = 0;

    // Value types
    bool boolVal = false;
    int32_t intVal = 0;
    int64_t int64Val = 0;
    float floatVal = 0;
    std::string strVal;

    // Struct / nested
    std::string structType;
    std::vector<GvasPropertyPtr> children;
    std::map<std::string, GvasPropertyPtr> childMap;

    // Array
    std::string arrayInnerType;
    std::vector<GvasPropertyPtr> arrayElements;

    GvasPropertyPtr getChild(const std::string& childName) const {
        auto it = childMap.find(childName);
        if (it != childMap.end()) return it->second;
        return nullptr;
    }
};

} // namespace PSM
