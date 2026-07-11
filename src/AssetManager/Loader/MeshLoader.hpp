#pragma once
#include "IAssetLoader.hpp"

class MeshLoader : public IAssetLoader {
public:
    std::shared_ptr<IAsset> Load(const std::string& filepath) override;
private:
};