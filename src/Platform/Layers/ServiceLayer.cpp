#include "ServiceLayer.hpp"
ServiceLayer::ServiceLayer()
    : Layer("EditorLayer") {
}

void ServiceLayer::OnAttach() {
    // This is where you can initialize resources, set up the scene, etc.
    CORE_LOG_INFO("ServiceLayer attached!");
	//Renderer::Init();
	//AssetManager::Init();
}

void ServiceLayer::OnUpdate(float deltaTime) {
	//nothing for now
}