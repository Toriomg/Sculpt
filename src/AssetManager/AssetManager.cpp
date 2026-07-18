#include "AssetManager.hpp"
#include "Loader/MeshLoader.hpp"
#include "Loader/TextureLoader.hpp"
#include "Platform/CoreUtils/Log.hpp"
#include "Platform/Graphics/Texture.hpp"
#include "Platform/Jobs/TaskQueue.hpp"
#include "Renderer/Mesh.hpp"
#include <filesystem>
#include <functional>
#include <map>
#include <optional>
#include <unordered_map>

namespace {

    struct AssetEntry {
        AssetHandle handle;
        std::shared_ptr<void> asset;
    };

    struct AssetManagerData {
        // Extension → loader: returns (asset ptr, handle-setter callback)
        std::map<std::string, std::function<std::shared_ptr<void>(std::string const&, AssetHandle)>>
            loaders;
        std::map<AssetHandle, std::shared_ptr<void>> assets;
        std::unordered_map<std::string, AssetHandle> pathToHandleMap;
    };

    std::optional<AssetManagerData>
        s_Data;  // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

}  // namespace

void AssetManager::Init() {
    s_Data.emplace();
    s_Data->loaders[".obj"] = [](std::string const& p, AssetHandle h) -> std::shared_ptr<void> {
        auto mesh = MeshLoader{}.Load(p);
        if (mesh) { mesh->Handle = h; }
        return mesh;
    };
    auto texLoad = [](std::string const& p, AssetHandle h) -> std::shared_ptr<void> {
        auto tex = TextureLoader{}.Load(p);
        if (tex) { tex->Handle = h; }
        return tex;
    };
    s_Data->loaders[".png"] = texLoad;
    s_Data->loaders[".jpg"] = texLoad;
}

void AssetManager::Shutdown() {
    s_Data.reset();
}

AssetHandle AssetManager::Load(std::string const& filepath) {
    if (!s_Data) { return AssetHandle{}; }
    auto it = s_Data->pathToHandleMap.find(filepath);
    if (it != s_Data->pathToHandleMap.end()) { return it->second; }

    std::string const ext = std::filesystem::path(filepath).extension().string();
    auto loaderIt         = s_Data->loaders.find(ext);
    if (loaderIt == s_Data->loaders.end()) {
        LOG_ERROR("AssetManager: No loader for: {0}", filepath);
        return AssetHandle{};
    }

    AssetHandle const handle          = AssetHandle::Create();
    std::shared_ptr<void> const asset = loaderIt->second(filepath, handle);
    if (!asset) {
        LOG_ERROR("AssetManager: Failed to load: {0}", filepath);
        return AssetHandle{};
    }

    s_Data->assets[handle]            = asset;
    s_Data->pathToHandleMap[filepath] = handle;

    LOG_INFO("AssetManager: Loaded '{0}' (ID {1})", filepath, handle.ID);
    return handle;
}

std::shared_ptr<void> AssetManager::Get(AssetHandle handle) {
    if (!s_Data) { return nullptr; }
    auto it = s_Data->assets.find(handle);
    if (it != s_Data->assets.end()) { return it->second; }
    CORE_LOG_WARN("Asset with handle ID {0} not found.", handle.ID);
    return nullptr;
}

void AssetManager::LoadAsync(std::string const& filepath,
                             std::function<void(AssetHandle)> onComplete) {
    TaskQueue::Submit([path = filepath, cb = std::move(onComplete)]() {
        AssetHandle const handle = AssetManager::Load(path);
        if (cb) { cb(handle); }
    });
}
