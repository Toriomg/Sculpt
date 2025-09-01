#include "EditorLayer.h"
#include "Core/Components/Component.h"
#include "Editor/EditorCameraController.h"

EditorLayer::EditorLayer()
    : Layer("EditorLayer"){
}

void EditorLayer::OnAttach() {
    // This is where you can initialize resources, set up the scene, etc.
    CORE_LOG_INFO("EditorLayer attached!");

	m_ActiveScene = std::make_unique<Scene>();

    m_CameraEntity = m_ActiveScene->CreateGameObject("Main Camera");
    auto& camComp = m_ActiveScene->AddComponent<CameraComponent>(m_CameraEntity);
    auto& camTransform = m_ActiveScene->GetComponent<TransformComponent>(m_CameraEntity);
    camTransform.Transform = Matx4f::translation(Vec3(0.0f, 0.0f, 5.0f));
    camComp.SceneCamera.SetPosition({ 0.0f, 0.0f, 5.0f });

    //m_CubeMesh = Mesh::CreateSphere(3.0f, 100, 100);
    m_CubeMesh = Mesh::CreatePyramid(1.0f);

    // --- Create a Material ---
    // 1. Create a shader from a file (assuming you have a Shader factory)
    auto simpleShader = std::make_shared<Shader>("res/shaders/modelmesh.shader");

    // 2. Create the Material (Layer 3)
    m_CubeMaterial = std::make_shared<Material>(simpleShader);


    // Set the cube's initial position
    m_CubePosition = { 0.0f, 0.0f, 0.0f };
	
    
    m_CubeEntity = m_ActiveScene->CreateGameObject("Cube");
    std::shared_ptr<Shader> myShader = simpleShader;
    std::shared_ptr<Material> myMaterial = m_CubeMaterial;
    std::shared_ptr<Mesh> myCubeMesh = m_CubeMesh;

    m_ActiveScene->AddComponent<MeshComponent>(m_CubeEntity, myCubeMesh, myMaterial);
	m_ActiveScene->SetComponent<TransformComponent>(m_CubeEntity, Matx4f::translation(m_CubePosition));

    m_CameraController = std::make_unique<EditorCameraController>(&camComp.SceneCamera);
}

void EditorLayer::OnUpdate(float deltaTime) {
	m_CameraController->OnUpdate(deltaTime);
    
    auto& camComp = m_ActiveScene->GetComponent<CameraComponent>(m_CameraEntity);
    auto& camTransform = m_ActiveScene->GetComponent<TransformComponent>(m_CameraEntity);
    camTransform.Transform = Matx4f::translation(camComp.SceneCamera.GetPosition());

    // 2. Prepare for rendering
    RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.85f, 1.0f });
    m_ActiveScene->OnUpdate(deltaTime); 
}

void EditorLayer::OnEvent(Event& e) {
    // This layer uses its own dispatcher to handle only the events it cares about.
    EventDispatcher dispatcher(e);
    dispatcher.Dispatch<MouseButtonPressedEvent>(std::bind(&EditorLayer::OnMouseButtonPressed, this, std::placeholders::_1));
    dispatcher.Dispatch<MouseButtonReleasedEvent>(std::bind(&EditorLayer::OnMouseButtonReleased, this, std::placeholders::_1));
    dispatcher.Dispatch<KeyPressedEvent>(std::bind(&EditorLayer::OnKeyPressed, this, std::placeholders::_1));
    dispatcher.Dispatch<KeyReleasedEvent>(std::bind(&EditorLayer::OnKeyReleased, this, std::placeholders::_1));
    dispatcher.Dispatch<MouseMovedEvent>(std::bind(&EditorLayer::OnMouseMoved, this, std::placeholders::_1));
    dispatcher.Dispatch<WindowResizeEvent>(std::bind(&EditorLayer::OnWindowResize, this, std::placeholders::_1));
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

bool EditorLayer::OnWindowResize(WindowResizeEvent& e)
{
    // Get the new width and height from the event object.
    uint32_t width = e.GetWidth();
    uint32_t height = e.GetHeight();

    // Don't do anything if the window is minimized.
    // A zero height will cause a division-by-zero in the aspect ratio calculation.
    if (width == 0 || height == 0) {
        return false;
    }

    // Get the camera component from our active scene.
    auto& camComp = m_ActiveScene->GetComponent<CameraComponent>(m_CameraEntity);

    // Tell the camera its new viewport size.
    camComp.SceneCamera.SetViewportSize(width, height);

    // We can also tell our low-level renderer about the viewport change.
    RenderCommand::SetViewport(0, 0, width, height);
	CORE_LOG_INFO("Camera resized to {0}x{1}", camComp.SceneCamera.GetViewportWidth(), camComp.SceneCamera.GetViewportHeight());
    // Return false to indicate that other layers might also want to process this event.
    return false;
}