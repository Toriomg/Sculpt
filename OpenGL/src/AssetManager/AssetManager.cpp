#include "AssetManager.h"
#include "Loader/LoaderSystem.h"
#include "AssetRegistry.h"
#include "Loader/TextureLoader.h"
#include "Loader/MeshLoader.h"
#include <optional>

struct AssetManagerData {
    LoaderSystem loaders;
    AssetRegistry registry;
    std::unordered_map<std::string, AssetHandle> pathToHandleMap;
};

static std::optional<AssetManagerData> s_Data;

void AssetManager::Init() {
    s_Data.emplace();
    s_Data->loaders.RegisterLoader(".obj", std::make_unique<MeshLoader>());
    s_Data->loaders.RegisterLoader(".png", std::make_unique<TextureLoader>());
    s_Data->loaders.RegisterLoader(".jpg", std::make_unique<TextureLoader>());
}

void AssetManager::Shutdown() {
    s_Data.reset();
}

AssetHandle AssetManager::Load(const std::string& filepath) {
    auto it = s_Data->pathToHandleMap.find(filepath);
    if (it != s_Data->pathToHandleMap.end())
        return it->second;

    IAssetLoader* loader = s_Data->loaders.GetLoaderFor(filepath);
    if (!loader) {
        LOG_ERROR("AssetManager: No loader for: {0}", filepath);
        return AssetHandle{};
    }

    std::shared_ptr<IAsset> asset = loader->Load(filepath);
    if (!asset) {
        LOG_ERROR("AssetManager: Failed to load: {0}", filepath);
        return AssetHandle{};
    }

    asset->Handle = AssetHandle::Create();
    s_Data->registry.Register(asset);
    s_Data->pathToHandleMap[filepath] = asset->Handle;

    LOG_INFO("AssetManager: Loaded '{0}' (ID {1})", filepath, asset->Handle.ID);
    return asset->Handle;
}

std::shared_ptr<IAsset> AssetManager::Get(AssetHandle handle) {
    return s_Data->registry.Get(handle);
}
