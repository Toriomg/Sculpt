#include <string>
#include <map>
#include "IAssetLoader.h"

class LoaderSystem {
public:
    void RegisterLoader(const std::string& extension, std::unique_ptr<IAssetLoader> loader);
    IAssetLoader* GetLoaderFor(const std::string& filepath);
private:
    std::map<std::string, std::unique_ptr<IAssetLoader>> m_Loaders;
};