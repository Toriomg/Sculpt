#include "TextureLoader.hpp"
#include "Platform/Graphics/Texture.hpp"

#include "stb_image/stb_image.h"

std::shared_ptr<IAsset> TextureLoader::Load(std::string const& filepath) {
    int width = 0, height = 0, channels = 0;

    // stb_image stores rows top-to-bottom; OpenGL expects bottom-to-top. This is a global stb_image
    // flag.
    stbi_set_flip_vertically_on_load(1);
    unsigned char* data = stbi_load(filepath.c_str(), &width, &height, &channels, 0);

    if (data == nullptr) {
        LOG_ERROR("TextureLoader: Failed to load image from {0}", filepath);
        stbi_image_free(data);  // Even if it's null, it's safe to call
        return nullptr;
    }

    CORE_LOG_INFO("TextureLoader: Loaded texture {0} ({1}x{2})", filepath, width, height);

    TextureSpecification spec;
    spec.Width  = width;
    spec.Height = height;
    // channels is hardcoded to 4 here but stbi_load was called with 0 (keep original).
    // Texture constructor uses spec.channels to pick GL_RGBA8 vs GL_RGB8, so 3-channel
    // files would be uploaded incorrectly. Fix: pass 4 to stbi_load to force RGBA conversion.
    spec.channels = 4;

    std::shared_ptr<Texture> texture = std::make_shared<Texture>(spec, data);
    stbi_image_free(data);
    return texture;
}
