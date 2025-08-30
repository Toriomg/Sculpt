#include "EditorLayer.h"

EditorLayer::EditorLayer()
    : Layer("EditorLayer"){
}

void EditorLayer::OnAttach() {
    // This is where you can initialize resources, set up the scene, etc.
    CORE_LOG_INFO("EditorLayer attached!");
    // --- Create a Cube Mesh ---
    // A cube has 8 vertices, but 24 are needed for correct normals/uvs per face.
    float vertices[] = {
        // Positions          
        -0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
        -0.5f,  0.5f, -0.5f,

        -0.5f, -0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f
    };

    uint32_t indices[] = {
        0, 1, 2, 2, 3, 0, // Front face
        4, 5, 6, 6, 7, 4, // Back face
        3, 2, 6, 6, 7, 3, // Top face
        0, 1, 5, 5, 4, 0, // Bottom face
        0, 3, 7, 7, 4, 0, // Left face
        1, 2, 6, 6, 5, 1  // Right face
    };

    auto vbo = std::make_shared<VertexBuffer>(&vertices, sizeof(vertices), true);
    auto ibo = std::make_shared<IndexBuffer>(indices, 36); // 12 triangles * 3 indices

    // 2. Define the layout of the vertex data
    VertexBufferLayout layout;
    layout.Push<float>(3); // One attribute: a vec3 for position

    // 3. Create the state object (VAO) and link the buffers and layout
    auto vao = std::make_shared<VertexArray>();
    vao->AddBufferPtr(vbo, layout);

    // 4. Create the high-level Mesh object (Layer 3)
    m_CubeMesh = std::make_shared<Mesh>(vao, ibo);

    // --- Create a Material ---
    // 1. Create a shader from a file (assuming you have a Shader factory)
    auto simpleShader = std::make_shared<Shader>("res/shaders/Basic.shader");

    // 2. Create the Material (Layer 3)
    m_CubeMaterial = std::make_shared<Material>(simpleShader);

    // Set a uniform on the material's shader
    m_CubeMaterial->GetShader()->Bind();
    m_CubeMaterial->GetShader()->SetUniform4f("u_Color", 0.8f, 0.2f, 0.3f, 1.0f);
    m_CubeMaterial->GetShader()->SetUniformMat4f("u_ViewProjection", m_EditorCamera.GetViewProjectionMatrix());
    m_CubeMaterial->GetShader()->SetUniformMat4f("u_Model", Matx4f::translation(m_CubePosition));

    // Set the cube's initial position
    m_CubePosition = { 0.0f, 0.0f, 0.0f };
}

void EditorLayer::OnUpdate(float deltaTime) {
    
    // 2. Prepare for rendering
    RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.85f, 1.0f });
    RenderCommand::Clear();

    // 3. Begin the rendering pass
    Renderer::BeginScene(m_EditorCamera.GetViewProjectionMatrix());

    // 4. Submit our cube to the renderer
    Matx4f transform = Matx4f::translation(m_CubePosition);
    Renderer::Submit(m_CubeMesh, m_CubeMaterial, transform);

    // 5. End the rendering pass
    Renderer::EndScene();
}

void EditorLayer::OnEvent(Event& e) {
    // This layer uses its own dispatcher to handle only the events it cares about.
    EventDispatcher dispatcher(e);
    dispatcher.Dispatch<MouseButtonPressedEvent>(std::bind(&EditorLayer::OnMouseButtonPressed, this, std::placeholders::_1));
    dispatcher.Dispatch<MouseButtonReleasedEvent>(std::bind(&EditorLayer::OnMouseButtonReleased, this, std::placeholders::_1));
    dispatcher.Dispatch<KeyPressedEvent>(std::bind(&EditorLayer::OnKeyPressed, this, std::placeholders::_1));
    dispatcher.Dispatch<KeyReleasedEvent>(std::bind(&EditorLayer::OnKeyReleased, this, std::placeholders::_1));
	dispatcher.Dispatch<MouseMovedEvent>(std::bind(&EditorLayer::OnMouseMoved, this, std::placeholders::_1));
}

bool EditorLayer::OnMouseButtonPressed(MouseButtonPressedEvent& e) {
    LOG_INFO("Mouse button {0} was pressed!", e.GetMouseButton());
    return false;
}

bool EditorLayer::OnMouseButtonReleased(MouseButtonReleasedEvent& e) {
    LOG_INFO("Mouse button {0} was released!", e.GetMouseButton());
    return false;
}

bool EditorLayer::OnMouseMoved(MouseMovedEvent& e) {
    float mouseX = e.GetX();
    float mouseY = e.GetY();
    LOG_INFO("Mouse moved to ({0}, {1})", mouseX, mouseY);
    return false;
}

bool EditorLayer::OnKeyPressed(KeyPressedEvent& e) {
    int keyName = e.GetKeyCode();
    LOG_INFO("Key '{0}' was pressed! (Repeat: {1})", Input::GetKeyName(keyName), e.IsRepeat());
    return false;
}

bool EditorLayer::OnKeyReleased(KeyReleasedEvent& e) {
    LOG_INFO("Key {0} was released!", Input::GetKeyName(e.GetKeyCode()));
    return false;
}