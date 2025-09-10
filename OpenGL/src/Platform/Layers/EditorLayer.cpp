#include "EditorLayer.h"
#include "Core/Components/Component.h"
#include "Editor/EditorCameraController.h"
#include "AssetManager/AssetManager.h"

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

    std::shared_ptr<Mesh> CubeMesh = Mesh::CreateCube(2.0f);
    std::shared_ptr<Mesh> SphereMesh = Mesh::CreateSphere(1.0f, 100, 100);
	std::shared_ptr<Mesh> PyramidMesh = Mesh::CreatePyramid(2.0f);
    std::shared_ptr<Mesh> TorusMesh = Mesh::CreateTorus(1.0f, 0.5f, 100, 100);

    // --- Create a Material ---
    auto simpleShader = std::make_shared<Shader>(
        //"res/shaders/test/testTexture.shader"
        "res/shaders/modelmesh.shader"
    );
    std::shared_ptr<Material> myMaterial = std::make_shared<Material>(simpleShader);
	myMaterial->SetTexture(AssetManager::Load("res/textures/texture1.png"));


    // Set the initial positions
    Vec3 CubePosition    = { 0.0f, 0.0f, 0.0f };
    Vec3 SpherePosition  = { 5.0f, 0.0f, 0.0f };
    Vec3 PyramidPosition = {-5.0f, 0.0f, 0.0f };
    Vec3 TorusPosition   = {10.0f, 0.0f, 0.0f };
	
    
    m_CubeEntity = m_ActiveScene->CreateGameObject("Cube");
    m_SphereEntity = m_ActiveScene->CreateGameObject("Sphere");
    m_PyramidEntity = m_ActiveScene->CreateGameObject("Pyramid");
    m_TorusEntity = m_ActiveScene->CreateGameObject("Torus");

    m_ActiveScene->AddComponent<MeshComponent>(m_CubeEntity, CubeMesh, myMaterial);
    m_ActiveScene->SetComponent<TransformComponent>(m_CubeEntity, Matx4f::translation(CubePosition));

    m_ActiveScene->AddComponent<MeshComponent>(m_SphereEntity, SphereMesh, myMaterial);
    m_ActiveScene->SetComponent<TransformComponent>(m_SphereEntity, Matx4f::translation(SpherePosition));
    
    m_ActiveScene->AddComponent<MeshComponent>(m_PyramidEntity, PyramidMesh, myMaterial);
    m_ActiveScene->SetComponent<TransformComponent>(m_PyramidEntity, Matx4f::translation(PyramidPosition));

    m_ActiveScene->AddComponent<MeshComponent>(m_TorusEntity, TorusMesh, myMaterial);
    m_ActiveScene->SetComponent<TransformComponent>(m_TorusEntity, Matx4f::translation(TorusPosition));

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