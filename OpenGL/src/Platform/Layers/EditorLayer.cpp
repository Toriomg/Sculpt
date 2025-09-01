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

	m_CubeMesh = Mesh::CreateSphere(3.0f, 100, 100);

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
    //LOG_INFO("Mouse moved to ({0}, {1})", mouseX, mouseY);
    /*
    if (Input::IsMouseButtonPressed(MouseCode::Right))
    {
		Camera& cam = m_ActiveScene->GetComponent<CameraComponent>(m_CameraEntity).SceneCamera;
        Vec2 currentPos = { mouseX, mouseY };
        Vec2 delta = (currentPos - m_LastMousePosition) * 0.15f; // Sensitivity factor

        // Update camera orientation
        cam.SetPitch(cam.GetPitch() + delta.y);
        cam.SetYaw(cam.GetYaw() - delta.x); // Pitch, Yaw
		LOG_TRACE("Camera Rotating. Pitch: {0}, Yaw: {1}", cam.GetPitch(), cam.GetYaw());
    }
    
    m_LastMousePosition = { mouseX, mouseY };
    */
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