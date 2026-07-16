#include "Renderer.hpp"
#include "AssetManager/AssetManager.hpp"
#include "Material.hpp"
#include "Mesh.hpp"
#include "Platform/Graphics/Shader.hpp"
#include "Platform/Graphics/Texture.hpp"

struct SceneData {
    Matx4f View;
    std::shared_ptr<Shader> WireframeShader;
    std::shared_ptr<Shader> DebugSelectionShader;
    std::shared_ptr<Shader> OutlineShader;
    bool DebugSelectionEnabled = false;
};

// We create a static instance of this data for the Renderer to use.
// It is OK as a single scene is rendered at a time, so this is thread-safe.
// NOT AN ATTRIBUTE OF RENDERER CLASS DUE TO SINGLETON PATTERN
static SceneData s_SceneData;
/*
Public API implementation
*/

void Renderer::Init() {
    CORE_LOG_INFO("Initializing Renderer");
    RenderCommand::Init();
    s_SceneData.WireframeShader = std::make_shared<Shader>("res/shaders/wireframe.shader");
    s_SceneData.DebugSelectionShader =
        std::make_shared<Shader>("res/shaders/debug_selection.shader");
    s_SceneData.OutlineShader = std::make_shared<Shader>("res/shaders/outline.shader");
}

void Renderer::SetDebugSelectionMode(bool enable) {
    s_SceneData.DebugSelectionEnabled = enable;
}

bool Renderer::IsDebugSelectionModeEnabled() {
    return s_SceneData.DebugSelectionEnabled;
}

std::shared_ptr<Shader> const& Renderer::GetDebugSelectionShader() {
    return s_SceneData.DebugSelectionShader;
}

void Renderer::Shutdown() { }

void Renderer::OnWindowResize(uint32_t width, uint32_t height) {
    // Pass the viewport resize command down to the RenderCommand system.
    RenderCommand::SetViewport(0, 0, width, height);
}

void Renderer::BeginScene(Matx4f const& ViewProjectionMatrix) {
    // At the beginning of rendering a scene, cache the camera's
    // combined view and projection matrix. This will be used for
    // every single object that gets drawn in this frame.
    RenderCommand::SetClearColor(Vec4(0.1f, 0.1f, 0.1f, 1.0f));
    RenderCommand::Clear();
    s_SceneData.View = ViewProjectionMatrix;
}

void Renderer::EndScene() { }

void Renderer::Submit(std::shared_ptr<Mesh> const& mesh, std::shared_ptr<Material> const& material,
                      Matx4f const& transform) {
    auto const& shader      = s_SceneData.DebugSelectionEnabled ? s_SceneData.DebugSelectionShader
                                                                : material->GetShader();
    auto const& vertexArray = mesh->GetVertexArray();
    auto const& indexBuffer = mesh->GetIndexBuffer();

    shader->Bind();

    shader->SetUniformMat4f("u_ViewProjection", s_SceneData.View);
    shader->SetUniformMat4f("u_Model", transform);
    shader->SetUniform4f("u_Color", 1.0f, 0.0f, 0.0f, 1.0f);

    bool hasTexture           = false;
    AssetHandle textureHandle = material->GetTextureHandle();
    if (textureHandle) {
        auto texture = std::static_pointer_cast<Texture>(AssetManager::Get(textureHandle));
        if (texture) {
            texture->Bind(0);
            shader->SetUniform1i("u_Texture", 0);
            hasTexture = true;
        } else {
            CORE_LOG_WARN("Texture handle is invalid or texture not found in AssetManager.");
        }
    }
    shader->SetUniform1i("u_HasTexture", hasTexture ? 1 : 0);

    RenderCommand::Draw(vertexArray, indexBuffer);
    shader->Unbind();
}

void Renderer::SubmitWireframe(std::shared_ptr<Mesh> const& mesh, Matx4f const& transform) {
    auto const& shader      = s_SceneData.WireframeShader;
    auto const& vertexArray = mesh->GetVertexArray();
    auto const& indexBuffer = mesh->GetIndexBuffer();

    shader->Bind();
    shader->SetUniformMat4f("u_ViewProjection", s_SceneData.View);
    shader->SetUniformMat4f("u_Model", transform);

    // Pass 1: solid black fill so the interior reads as black
    shader->SetUniform4f("u_Color", 0.0f, 0.0f, 0.0f, 1.0f);
    RenderCommand::Draw(vertexArray, indexBuffer);

    // Pass 2: white edges — polygon offset pulls lines in front of the fill to avoid z-fighting
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glEnable(GL_POLYGON_OFFSET_LINE);
    glPolygonOffset(-1.0f, -1.0f);
    shader->SetUniform4f("u_Color", 1.0f, 1.0f, 1.0f, 1.0f);
    RenderCommand::Draw(vertexArray, indexBuffer);
    glDisable(GL_POLYGON_OFFSET_LINE);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // Pass 3: white vertex dots
    shader->SetUniform4f("u_Color", 1.0f, 1.0f, 1.0f, 1.0f);
    RenderCommand::DrawPoints(vertexArray, mesh->GetVertexCount());

    shader->Unbind();
}

void Renderer::SubmitOutline(std::shared_ptr<Mesh> const& mesh, Vec4 const& color, float thickness,
                             Matx4f const& transform) {
    auto const& shader = s_SceneData.OutlineShader;
    shader->Bind();
    shader->SetUniformMat4f("u_ViewProjection", s_SceneData.View);
    shader->SetUniformMat4f("u_Model", transform);
    shader->SetUniform4f("u_OutlineColor", color.x, color.y, color.z, color.w);
    shader->SetUniform1f("u_OutlineThickness", thickness);
    RenderCommand::Draw(mesh->GetVertexArray(), mesh->GetIndexBuffer());
    shader->Unbind();
}

void Renderer::SubmitFlat(std::shared_ptr<Mesh> const& mesh, Vec4 const& color,
                          Matx4f const& transform) {
    auto const& shader = s_SceneData.WireframeShader;
    shader->Bind();
    shader->SetUniformMat4f("u_ViewProjection", s_SceneData.View);
    shader->SetUniformMat4f("u_Model", transform);
    shader->SetUniform4f("u_Color", color.x, color.y, color.z, color.w);
    RenderCommand::Draw(mesh->GetVertexArray(), mesh->GetIndexBuffer());
    shader->Unbind();
}
