// Static asset management facade: routes file loads through LoaderSystem and caches results in AssetRegistry.
#pragma once
#include "Loader/LoaderSystem.hpp"
#include "AssetRegistry.hpp"
#include <functional>

class AssetManager {
public:
    static void Init();
    static void Shutdown();

    // Blocking load — returns an invalid handle on failure.
    static AssetHandle Load(const std::string& filepath);
    // Non-blocking load — onComplete is called on the main thread once the asset is ready.
    static void LoadAsync(const std::string& filepath, std::function<void(AssetHandle)> onComplete = {});

    // Retrieves an asset by its handle. Returns nullptr if not found.
    static std::shared_ptr<IAsset> Get(AssetHandle handle);

    template<typename T>
    static std::shared_ptr<T> GetAs(AssetHandle handle) {
        return std::dynamic_pointer_cast<T>(Get(handle));
    }
};