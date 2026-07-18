// Static asset management facade: loads assets from disk and caches them by handle.
#pragma once
#include "Platform/CoreUtils/AssetHandle.hpp"
#include <functional>
#include <memory>
#include <string>

class AssetManager {
public:
    static void Init();
    static void Shutdown();

    // Blocking load — returns an invalid handle on failure.
    static AssetHandle Load(std::string const& filepath);
    // Non-blocking load — onComplete is called on the main thread once the asset is ready.
    static void LoadAsync(std::string const& filepath,
                          std::function<void(AssetHandle)> onComplete = {});

    // Retrieves an asset by its handle as void*. Prefer GetAs<T>.
    static std::shared_ptr<void> Get(AssetHandle handle);

    template <typename T> static std::shared_ptr<T> GetAs(AssetHandle handle) {
        return std::static_pointer_cast<T>(Get(handle));
    }
};
