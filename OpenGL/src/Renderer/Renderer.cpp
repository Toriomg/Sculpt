#include "Renderer.h"
#include "Material.h"
#include "Mesh.h"
#include "AssetManager/AssetManager.h"

// This struct is a private implementation detail of the Renderer.
// It holds data that is constant for an entire scene render pass.
struct SceneData {
    Matx4f View;
};

// We create a static instance of this data for the Renderer to use.
// It is OK as a single scene is rendered at a time, so this is thread-safe.
// NOT AN ATTRIBUTE OF RENDERER CLASS DUE TO SINGLETON PATTERN
static SceneData s_SceneData;
/*
Public API implementation
*/

void Renderer::Init() {
	CORE_LOG_TRACE("Initializing Renderer");
	RenderCommand::Init();
}

void Renderer::Shutdown() {
}

void Renderer::OnWindowResize(uint32_t width, uint32_t height) {
    // Pass the viewport resize command down to the RenderCommand system.
    RenderCommand::SetViewport(0, 0, width, height);
}

void Renderer::BeginScene(const Matx4f& ViewProjectionMatrix) {
    // At the beginning of rendering a scene, cache the camera's
    // combined view and projection matrix. This will be used for
    // every single object that gets drawn in this frame.
	RenderCommand::SetClearColor(Vec4(0.1f, 0.1f, 0.1f, 1.0f));
    RenderCommand::Clear();
    s_SceneData.View = ViewProjectionMatrix;
}

void Renderer::EndScene() {
}

void Renderer::Submit(
    const std::shared_ptr<Mesh>& mesh,
    const std::shared_ptr<Material>& material,
    const Matx4f& transform
) {
    const auto& shader = material->GetShader();
    const auto& vertexArray = mesh->GetVertexArray();
    const auto& indexBuffer = mesh->GetIndexBuffer();

    shader->Bind();

    shader->SetUniformMat4f("u_ViewProjection", s_SceneData.View);
    shader->SetUniformMat4f("u_Model", transform);
    shader->SetUniform4f("u_Color", 1.0f, 0.0f, 0.0f, 1.0f);
    shader->SetUniform3f("u_objectColor", 0.0f, 1.0f, 0.0f);
    
    AssetHandle textureHandle = material->GetTextureHandle();
	LOG_TRACE("Texture Handle ID: {0}", textureHandle.ID);

    if (textureHandle) {
        auto texture = std::static_pointer_cast<Texture>(AssetManager::Get(textureHandle));
        if (texture)
        {
            int textureSlot = 0;
            // Tell OpenGL to place our texture object into the chosen slot.
            texture->Bind(textureSlot);
            // and tell it to get its data from slot 0.
            shader->SetUniform1i("u_Texture", textureSlot); // The value is the SLOT, not the texture ID!

        }
    }

	RenderCommand::Draw(vertexArray, indexBuffer);
}