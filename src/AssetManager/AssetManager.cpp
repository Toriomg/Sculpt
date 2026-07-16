#include "AssetManager.hpp"
#include "AssetRegistry.hpp"
#include "Loader/LoaderSystem.hpp"
#include "Loader/MeshLoader.hpp"
#include "Loader/TextureLoader.hpp"
#include "Platform/Jobs/TaskQueue.hpp"
#include <functional>
#include <optional>

namespace {

    struct AssetManagerData {
        LoaderSystem loaders;
        AssetRegistry registry;
        std::unordered_map<std::string, AssetHandle> pathToHandleMap;
    };

    // Optional gives us a clear Init/Shutdown lifecycle without a heap allocation or a separate
    // "initialized" flag.
    std::optional<AssetManagerData>
        s_Data;  // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

}  // namespace

void AssetManager::Init() {
    s_Data.emplace();
    s_Data->loaders.RegisterLoader(".obj", std::make_unique<MeshLoader>());
    s_Data->loaders.RegisterLoader(".png", std::make_unique<TextureLoader>());
    s_Data->loaders.RegisterLoader(".jpg", std::make_unique<TextureLoader>());
}

void AssetManager::Shutdown() {
    s_Data.reset();
}

AssetHandle AssetManager::Load(std::string const& filepath) {
    if (!s_Data) { return AssetHandle{}; }
    // Return the cached handle if this path was already loaded, preventing duplicate GPU uploads.
    auto it = s_Data->pathToHandleMap.find(filepath);
    if (it != s_Data->pathToHandleMap.end()) { return it->second; }

    IAssetLoader* loader = s_Data->loaders.GetLoaderFor(filepath);
    if (loader == nullptr) {
        LOG_ERROR("AssetManager: No loader for: {0}", filepath);
        return AssetHandle{};
    }

    std::shared_ptr<IAsset> const asset = loader->Load(filepath);
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
    if (!s_Data) { return nullptr; }
    return s_Data->registry.Get(handle);
}

namespace {

    // Defers the full load to the main thread via Finalize(); Execute() is reserved for future
    // CPU-only work (file read, decode) once loaders are split into CPU and GPU phases.
    class AssetLoadTask final : public ITask {
    public:
        AssetLoadTask(std::string path, std::function<void(AssetHandle)> onComplete)
            : m_Path(std::move(path)), m_OnComplete(std::move(onComplete)) { }

        [[nodiscard]] std::string_view GetName() const override { return "AssetLoad"; }
        void Execute() override { }

        void Finalize() override {
            AssetHandle const handle = AssetManager::Load(m_Path);
            if (m_OnComplete) { m_OnComplete(handle); }
        }

    private:
        std::string m_Path;
        std::function<void(AssetHandle)> m_OnComplete;
    };

}  // namespace

void AssetManager::LoadAsync(std::string const& filepath,
                             std::function<void(AssetHandle)> onComplete) {
    auto result = TaskQueue::Submit<AssetLoadTask>(filepath, std::move(onComplete));
    if (!result) {
        LOG_ERROR("AssetManager: LoadAsync failed for '{}': {}", filepath, result.error());
    }
}
