#include "EditorLayer.hpp"
#include "Core/Components/Component.hpp"
#include "Core/Systems/SelectionSystem.hpp"
#include "Core/Systems/PickingSystem.hpp"
#include "Editor/EditorCameraController.hpp"
#include "AssetManager/AssetManager.hpp"
#include "Platform/System/Input/Input.hpp"
#include "Platform/System/Input/KeyCodes.hpp"

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

    auto monkeyMesh = std::static_pointer_cast<Mesh>(AssetManager::Get(AssetManager::Load("res/models/monkey.obj")));
    std::shared_ptr<Mesh> SphereMesh = Mesh::CreateSphere(1.0f, 32, 32);
	std::shared_ptr<Mesh> PyramidMesh = Mesh::CreateIcosahedron(2.0f);
    std::shared_ptr<Mesh> TorusMesh = Mesh::CreateTorus(1.0f, 0.5f, 32, 32);

    // --- Create a Material ---
    auto simpleShader = std::make_shared<Shader>(
        //"res/shaders/test/testTexture.shader"
        "res/shaders/modelmesh.shader"
    );
    std::shared_ptr<Material> myMaterial = std::make_shared<Material>(simpleShader);
	myMaterial->SetTexture(AssetManager::Load("res/textures/texture1.png"));


    // Set the initial positions
    Vec3 MonkeyPosition  = { 0.0f, 0.0f, 0.0f };
    Vec3 SpherePosition  = { 5.0f, 0.0f, 0.0f };
    Vec3 PyramidPosition = {-5.0f, 0.0f, 0.0f };
    Vec3 TorusPosition   = {10.0f, 0.0f, 0.0f };

    m_MonkeyEntity = m_ActiveScene->CreateGameObject("Monkey");
    m_SphereEntity = m_ActiveScene->CreateGameObject("Sphere");
    m_PyramidEntity = m_ActiveScene->CreateGameObject("Pyramid");
    m_TorusEntity = m_ActiveScene->CreateGameObject("Torus");

    m_ActiveScene->AddComponent<MeshComponent>(m_MonkeyEntity, monkeyMesh, myMaterial);
    m_ActiveScene->SetComponent<TransformComponent>(m_MonkeyEntity, Matx4f::translation(MonkeyPosition));
    m_ActiveScene->AddComponent<SelectionComponent>(m_MonkeyEntity);

    m_ActiveScene->AddComponent<MeshComponent>(m_SphereEntity, SphereMesh, myMaterial);
    m_ActiveScene->SetComponent<TransformComponent>(m_SphereEntity, Matx4f::translation(SpherePosition));
    m_ActiveScene->AddComponent<SelectionComponent>(m_SphereEntity);

    m_ActiveScene->AddComponent<MeshComponent>(m_PyramidEntity, PyramidMesh, myMaterial);
    m_ActiveScene->SetComponent<TransformComponent>(m_PyramidEntity, Matx4f::translation(PyramidPosition));
    m_ActiveScene->AddComponent<SelectionComponent>(m_PyramidEntity);

    m_ActiveScene->AddComponent<MeshComponent>(m_TorusEntity, TorusMesh, myMaterial);
    m_ActiveScene->SetComponent<TransformComponent>(m_TorusEntity, Matx4f::translation(TorusPosition));
    m_ActiveScene->AddComponent<SelectionComponent>(m_TorusEntity);

    // Stores a raw pointer into the ECS component. If m_CameraEntity is ever destroyed
    // or the registry reallocates, this pointer would dangle — safe only while the entity lives.
    m_CameraController = std::make_unique<EditorCameraController>(&camComp.SceneCamera);
    m_Grid = std::make_unique<InfGrid>();
}

void EditorLayer::OnUpdate(float deltaTime) {
	m_CameraController->OnUpdate(deltaTime);
    
    auto& camComp = m_ActiveScene->GetComponent<CameraComponent>(m_CameraEntity);
    auto& camTransform = m_ActiveScene->GetComponent<TransformComponent>(m_CameraEntity);
    // Camera position is driven by EditorCameraController (not the ECS transform), so we
    // manually sync the TransformComponent each frame to keep both representations consistent.
    camTransform.Transform = Matx4f::translation(camComp.SceneCamera.GetPosition());

    RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.85f, 1.0f });
    m_ActiveScene->OnUpdate(deltaTime);

    // Grid is drawn after scene geometry so it only covers pixels at maximum depth (no geometry).
    m_Grid->Draw(
        camComp.SceneCamera.GetViewMatrix(),
        camComp.SceneCamera.GetProjectionMatrix(),
        camComp.SceneCamera.GetPosition()
    );
}

void EditorLayer::OnEvent(Event& e) {
    // This layer uses its own dispatcher to handle only the events it cares about.
    EventDispatcher dispatcher(e);
    dispatcher.Dispatch<MouseButtonPressedEvent>([this](MouseButtonPressedEvent& e)  { return OnMouseButtonPressed(e); });
    dispatcher.Dispatch<MouseButtonReleasedEvent>([this](MouseButtonReleasedEvent& e) { return OnMouseButtonReleased(e); });
    dispatcher.Dispatch<KeyPressedEvent>([this](KeyPressedEvent& e)                  { return OnKeyPressed(e); });
    dispatcher.Dispatch<KeyReleasedEvent>([this](KeyReleasedEvent& e)                { return OnKeyReleased(e); });
    dispatcher.Dispatch<MouseMovedEvent>([this](MouseMovedEvent& e)                  { return OnMouseMoved(e); });
    dispatcher.Dispatch<MouseScrolledEvent>([this](MouseScrolledEvent& e)            { return OnMouseScrolled(e); });
    dispatcher.Dispatch<WindowResizeEvent>([this](WindowResizeEvent& e)              { return OnWindowResize(e); });
}

bool EditorLayer::OnMouseButtonPressed(MouseButtonPressedEvent& e) {
    // Button 0 is left mouse button in GLFW; GetMouseButton() returns a raw GLFW int, not MouseCode.
    if (e.GetMouseButton() != 0) {
        return false;
    }

    Vec2 mousePos = Input::GetMousePosition();
    bool isShiftHeld = Input::IsKeyPressed(KeyCode::LeftShift);

    auto selSystem = m_ActiveScene->GetSystem<SelectionSystem>();
    if (!selSystem) {
        return false;
    }
    selSystem->OnMouseClick(static_cast<uint32_t>(mousePos.x), static_cast<uint32_t>(mousePos.y), isShiftHeld);

    return true;
}

bool EditorLayer::OnMouseButtonReleased(MouseButtonReleasedEvent& e) {
    LOG_INFO("Mouse button {0} was released!", e.GetMouseButton());
    return false;
}

bool EditorLayer::OnMouseMoved(MouseMovedEvent& e) {
    return false;
}

bool EditorLayer::OnMouseScrolled(MouseScrolledEvent& e) {
    m_CameraController->OnScrolled(e.GetYOffset());
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

    auto pickSystem = m_ActiveScene->GetSystem<PickingSystem>();
    if (pickSystem) {
        pickSystem->OnWindowResize(width, height);
    }

	CORE_LOG_INFO("Camera resized to {0}x{1}", camComp.SceneCamera.GetViewportWidth(), camComp.SceneCamera.GetViewportHeight());
    // Return false to indicate that other layers might also want to process this event.
    return false;
}