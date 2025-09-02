#include "AssetManager.h"
#include "Loader/LoaderSystem.h"
#include "AssetRegistry.h"
#include "Loader/TextureLoader.h"

// This struct holds the static instance data
struct AssetManagerData {
    LoaderSystem loaders;
    AssetRegistry registry;
    std::unordered_map<std::string, AssetHandle> pathToHandleMap;
};

static AssetManagerData* s_Data = nullptr;

void AssetManager::Init() {
    s_Data = new AssetManagerData();

    // --- Register all your concrete loaders here ---
	// TODO: Implement MeshLoader
    //s_Data->loaders.RegisterLoader(".fbx", std::make_unique<MeshLoader>());
    //s_Data->loaders.RegisterLoader(".obj", std::make_unique<MeshLoader>());
    s_Data->loaders.RegisterLoader(".png", std::make_unique<TextureLoader>());
    s_Data->loaders.RegisterLoader(".jpg", std::make_unique<TextureLoader>());

    // You can also register "pseudo-loaders" for procedural assets
    //s_Data->loaders.RegisterLoader(".cube", std::make_unique<MeshLoader>()); // For "primitives://cube.cube"
}

void AssetManager::Shutdown() {
    delete s_Data;
    s_Data = nullptr;
}

AssetHandle AssetManager::Load(const std::string& filepath) {
    // 1. Check if we already have a handle for this path
    auto it = s_Data->pathToHandleMap.find(filepath);
    if (it != s_Data->pathToHandleMap.end()) {
        return it->second;
    }

    // 2. Find the correct loader for this file type
    IAssetLoader* loader = s_Data->loaders.GetLoaderFor(filepath);
    if (!loader) {
        LOG_ERROR("AssetManager: No loader available for file type: {0}", filepath);
        return AssetHandle(); // Return invalid handle with ID = 0
    }

    // 3. Load the asset from disk
    std::shared_ptr<IAsset> asset = loader->Load(filepath);
    if (!asset) {
        LOG_ERROR("AssetManager: Failed to load asset from: {0}", filepath);
        return AssetHandle(); // Return invalid handle with ID = 0
    }

    // 4. Assign a new handle and register the asset
    asset->Handle = AssetHandle::Create();
    s_Data->registry.Register(asset);
    s_Data->pathToHandleMap[filepath] = asset->Handle;

    LOG_INFO("AssetManager: Loaded '{0}' with Handle ID {1}", filepath, asset->Handle.ID);
    return asset->Handle;
}

std::shared_ptr<IAsset> AssetManager::Get(AssetHandle handle) {
    return s_Data->registry.Get(handle);
}