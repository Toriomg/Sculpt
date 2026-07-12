// Typed 64-bit asset handle with a static ID factory; used as a key into AssetRegistry by AssetManager.
#pragma once
#include <cstdint>
#include <functional>
#include "Platform/CoreUtils/Log.hpp"

// A lightweight, non-owning handle to an asset.
// This is what game objects will store instead of direct pointers.
struct AssetHandle {
    uint64_t ID = 0; // 0 is typically considered an invalid or "null" handle

    AssetHandle() = default;
    AssetHandle(uint64_t id) : ID(id) {}

    // Overload operators for easy use in maps and comparisons
    bool operator==(const AssetHandle& other) const { return ID == other.ID; }
    bool operator!=(const AssetHandle& other) const { return ID != other.ID; }
    operator bool() const { return ID != 0; } // Check for validity

    static AssetHandle Create() {
		// Simple static counter for unique IDs
        static uint64_t s_Counter = 1; // Start at 1, 0 is invalid
		CORE_LOG_TRACE("Creating AssetHandle with ID {0}", s_Counter);
        return AssetHandle(s_Counter++);
    }
};

// Required to use AssetHandle as a key in std::unordered_map
namespace std {
    template<> struct hash<AssetHandle> {
        size_t operator()(const AssetHandle& handle) const {
            return hash<uint64_t>()(handle.ID);
        }
    };
}
