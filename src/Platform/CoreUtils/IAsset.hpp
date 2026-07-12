// Base interface for all managed assets; Mesh and Texture derive from this and carry their own AssetHandle.
#pragma once
#include "Platform/CoreUtils/AssetHandle.hpp"

class IAsset {
public:
    // A virtual destructor is essential for any class with virtual methods.
    virtual ~IAsset() = default;

    // A way for any asset to report its own handle.
    AssetHandle Handle;
};
