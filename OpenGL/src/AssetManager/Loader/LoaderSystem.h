#include <string>
#include <map>
#include "IAssetLoader.h"

class LoaderSystem {
public:
    // Registers a loader for a specific file extension (e.g., ".png").
    void RegisterLoader(const std::string& extension, std::unique_ptr<IAssetLoader> loader);
    // Finds the appropriate loader for a given file path.
    IAssetLoader* GetLoaderFor(const std::string& filepath);
private:
    std::map<std::string, std::unique_ptr<IAssetLoader>> m_Loaders;
};