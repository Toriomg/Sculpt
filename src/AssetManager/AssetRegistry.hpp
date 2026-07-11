// Internal asset store: maps AssetHandle to shared_ptr<IAsset>; owned and queried exclusively by AssetManager.
#pragma once
#include "Core/glhead.hpp"
#include "AssetHandle.hpp"
#include "IAsset.hpp"

class AssetRegistry {
public:
    // Add a new asset to the registry
    void Register(std::shared_ptr<IAsset> asset);

    // Retrieves an asset from the registry.
    // Returns a shared_ptr, which will be empty if the asset is gone.
    std::shared_ptr<IAsset> Get(AssetHandle handle);

	// Should be done automatically when the last shared_ptr to an asset is destroyed,
    void Unload(AssetHandle handle);
private:
    std::map<AssetHandle, std::shared_ptr<IAsset>> m_Assets;
};