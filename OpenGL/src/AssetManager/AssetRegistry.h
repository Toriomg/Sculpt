#include "glhead.h"
#include "AssetHandle.h"
#include "IAsset.h"

class AssetRegistry {
public:
    // Add a new asset to the registry
    void Register(std::shared_ptr<IAsset> asset);

    // Find an asset by its handle
    std::weak_ptr<IAsset> Get(AssetHandle handle);

private:
    std::map<AssetHandle, std::weak_ptr<IAsset>> m_Assets;
}; ();