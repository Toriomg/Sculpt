// Loads image files via stb_image and returns a Texture.
#pragma once
#include <memory>
#include <string>

class Texture;

class TextureLoader {
public:
    std::shared_ptr<Texture> Load(std::string const& filepath);
};
