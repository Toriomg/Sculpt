#include "ServiceLayer.h"
ServiceLayer::ServiceLayer()
    : Layer("EditorLayer") {
}

void ServiceLayer::OnAttach() {
    // This is where you can initialize resources, set up the scene, etc.
    CORE_LOG_INFO("ServiceLayer attached!");
	Renderer::Init();
}

void ServiceLayer::OnUpdate(float deltaTime) {
	Renderer::BeginScene(m_EditorCamera.GetViewProjectionMatrix());
	Matx4f transform = Matx4f::translation(m_CubePosition);
	Renderer::Submit(m_CubeMesh, m_CubeMaterial, transform);
	Renderer::EndScene();
}