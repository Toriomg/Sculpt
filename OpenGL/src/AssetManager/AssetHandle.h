#pragma once
#include <cstdint>

// A lightweight, non-owning handle to an asset.
// This is what game objects will store instead of direct pointers.
struct AssetHandle {
    uint64_t ID = 0; // 0 is typically considered an invalid or "null" handle

    // Overload operators for easy use in maps and comparisons
    bool operator==(const AssetHandle& other) const { return ID == other.ID; }
    bool operator!=(const AssetHandle& other) const { return ID != other.ID; }
};

// Required to use AssetHandle as a key in std::unordered_map
namespace std {
    template<> struct hash<AssetHandle> {
        size_t operator()(const AssetHandle& handle) const {
            return hash<uint64_t>()(handle.ID);
        }
    };
}