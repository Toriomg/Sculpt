#pragma once
#include "IAssetLoader.h"

class TextureLoader : public IAssetLoader {
public:
    std::shared_ptr<IAsset> Load(const std::string& filepath) override;
};