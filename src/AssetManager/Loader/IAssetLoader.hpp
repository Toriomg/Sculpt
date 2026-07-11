// Abstract loader interface; LoaderSystem owns concrete implementations and dispatches to them by file extension.
#pragma once
#include <string>
#include <memory>
#include "AssetManager/IAsset.hpp"
#include "Platform/CoreUtils/Log.hpp"

class IAssetLoader {
public:
    virtual ~IAssetLoader() = default;

    // Loads data from a file path into a new asset object.
    virtual std::shared_ptr<IAsset> Load(const std::string& filepath) = 0;
};