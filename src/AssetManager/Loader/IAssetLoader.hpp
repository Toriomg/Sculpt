// Abstract loader interface; LoaderSystem owns concrete implementations and dispatches to them by
// file extension.
#pragma once
#include "Platform/CoreUtils/IAsset.hpp"
#include <memory>
#include <string>

class IAssetLoader {
public:
    virtual ~IAssetLoader() = default;

    // Loads data from a file path into a new asset object.
    virtual std::shared_ptr<IAsset> Load(std::string const& filepath) = 0;
};
