// IAssetLoader that produces Texture assets via stb_image; registered in LoaderSystem for image
// file extensions.
#pragma once
#include "IAssetLoader.hpp"

class TextureLoader : public IAssetLoader {
public:
    std::shared_ptr<IAsset> Load(std::string const& filepath) override;
};
