#pragma once
#include "AssetHandle.h"

class IAsset {
public:
    // A virtual destructor is essential for any class with virtual methods.
    virtual ~IAsset() = default;

    // A way for any asset to report its own handle.
    AssetHandle Handle;
};