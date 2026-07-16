// Maps file extensions to IAssetLoader implementations; owned by AssetManager, which calls
// GetLoaderFor on each Load().
#pragma once
#include "IAssetLoader.hpp"
#include <map>
#include <string>

class LoaderSystem {
public:
    // Registers a loader for a specific file extension (e.g., ".png").
    void RegisterLoader(std::string const& extension, std::unique_ptr<IAssetLoader> loader);
    // Finds the appropriate loader for a given file path.
    IAssetLoader* GetLoaderFor(std::string const& filepath);

private:
    std::map<std::string, std::unique_ptr<IAssetLoader>> m_Loaders;
};
