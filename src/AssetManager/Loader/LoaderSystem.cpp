#include "LoaderSystem.hpp"
#include <filesystem>

void LoaderSystem::RegisterLoader(const std::string& extension, std::unique_ptr<IAssetLoader> loader) {
    m_Loaders[extension] = std::move(loader);
}

IAssetLoader* LoaderSystem::GetLoaderFor(const std::string& filepath) {
    std::string extension = std::filesystem::path(filepath).extension().string();

    auto it = m_Loaders.find(extension);
    if (it != m_Loaders.end()) {
        return it->second.get();
    }

    return nullptr;
}