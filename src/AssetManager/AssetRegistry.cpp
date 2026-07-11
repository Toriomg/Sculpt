#include "AssetRegistry.hpp"


void AssetRegistry::Register(std::shared_ptr<IAsset> asset) {
    if (asset && asset->Handle) {
        m_Assets[asset->Handle] = asset;
    }
}

std::shared_ptr<IAsset> AssetRegistry::Get(AssetHandle handle) {
    auto it = m_Assets.find(handle);
    if (it != m_Assets.end()) {
        return it->second;
    }
	CORE_LOG_WARN("Asset with handle ID {0} not found in registry.", handle.ID);
    return nullptr;
}

void AssetRegistry::Unload(AssetHandle handle) {
    m_Assets.erase(handle);
}