#include "TextureLoader.h"
#include "Platform/Graphics/Texture.h"

#include "stb_image/stb_image.h"

std::shared_ptr<IAsset> TextureLoader::Load(const std::string& filepath) {
    int width, height, channels;

    // 1. Load the pixel data from the file into a CPU buffer
    stbi_set_flip_vertically_on_load(1);
    unsigned char* data = stbi_load(filepath.c_str(), &width, &height, &channels, 0); // Force 4 channels (RGBA)

    if (data == nullptr) {
        LOG_ERROR("TextureLoader: Failed to load image from {0}", filepath);
        stbi_image_free(data); // Even if it's null, it's safe to call
        return nullptr;
    }

    CORE_LOG_INFO("TextureLoader: Loaded texture {0} ({1}x{2})", filepath, width, height);

    // 2. Create the specification for the new texture
    TextureSpecification spec;
    spec.Width = width;
    spec.Height = height;
	spec.channels = 4;

    // 3. Create the GPU Texture object using the loaded data
    std::shared_ptr<Texture> texture = std::make_shared<Texture>(spec, data);

    // 4. Free the CPU buffer, as the data is now on the GPU
    stbi_image_free(data);

    // 5. Return the new texture as a generic IAsset pointer
    return texture;
}