#pragma once
#include <string>
#include <memory>
#include "AssetManager/IAsset.h"
#include "Platform/CoreUtils/Log.h"

class IAssetLoader {
public:
    virtual ~IAssetLoader() = default;

    // Loads data from a file path into a new asset object.
    virtual std::shared_ptr<IAsset> Load(const std::string& filepath) = 0;
};