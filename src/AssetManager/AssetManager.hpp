// Static asset management facade: routes file loads through LoaderSystem and caches results in AssetRegistry.
#pragma once
#include "Loader/LoaderSystem.hpp"
#include "AssetRegistry.hpp"

class AssetManager {
public:
    // The public API
    static void Init();
    static void Shutdown();

	// Loads an asset from a file path. Returns an invalid handle if loading fails.
    static AssetHandle Load(const std::string& filepath);
	// Retrieves an asset by its handle. Returns nullptr if not found.
    static std::shared_ptr<IAsset> Get(AssetHandle handle);

    // Template helper to get an asset and cast it to the correct derived type.
    template<typename T>
    static std::shared_ptr<T> GetAs(AssetHandle handle) {
        return std::dynamic_pointer_cast<T>(Get(handle));
    }
private:
    // The implementation would be in a static instance inside the .cpp
    // For example: static AssetRegistry s_Registry;
};