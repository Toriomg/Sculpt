#include "Application.hpp"
#include "Platform/CoreUtils/Log.hpp"
#include "Platform/System/Time.hpp"
#include "AssetManager/AssetManager.hpp"
#include "Renderer/Renderer.hpp"
#include "Editor/EditorLayer.hpp"
#include "Editor/ImGuiLayer.hpp"

Application::Application(const std::string& name, unsigned int width, unsigned int height)
    : m_Window(nullptr), m_AppName(name)
{
	Log::Init(); // Initialize the logging system
	AssetManager::Init(); // Initialize the Asset Manager

    m_Window = Window::Create();
    Input::Init(m_Window.get());
	Renderer::Init(); // Initialize the Renderer

	m_Window->SetVSync(true);
    m_Window->SetEventCallback([this](Event& e) { OnEvent(e); });

    // Setup Time
	Time::Init();

    m_LayerStack.PushLayer(std::make_unique<EditorLayer>([this]() { m_Running = false; }));

    auto imguiLayer = std::make_unique<ImGuiLayer>(
        static_cast<GLFWwindow*>(m_Window->GetNativeWindow())
    );
    m_ImGuiLayer = imguiLayer.get();
    m_LayerStack.PushLayer(std::move(imguiLayer));

    // Push a synthetic resize so camera projection and picking texture are sized to the
    // actual framebuffer before the first frame — otherwise they use their default dimensions.
    WindowResizeEvent initResize(m_Window->GetWidth(), m_Window->GetHeight());
    OnEvent(initResize);

	CORE_LOG_INFO("Application Initialized");
	LOG_SEPARATOR();
}

Application::~Application()
{
	Input::Shutdown(); // Shutdown the input system
    CORE_LOG_INFO("Program CORRECTLY ended");
}

void Application::Run()
{
    while (m_Running)
    {
		Time::Update();
		Input::OnUpdate(); // Update the input system
        for (auto& layer : m_LayerStack) {
            layer->OnUpdate(Time::GetDeltaTime());
        }
        m_ImGuiLayer->Begin();
        for (auto& layer : m_LayerStack) {
            layer->OnImGuiRender();
        }
        m_ImGuiLayer->End();
        m_Window->OnUpdate();
    }
}

void Application::OnEvent(Event& e) {
    // Use the dispatcher to route the event to the correct handler
    EventDispatcher dispatcher(e);
    dispatcher.Dispatch<WindowCloseEvent>([this](WindowCloseEvent& e) { return OnWindowClose(e); });

    // Reverse iteration gives the topmost (most recently pushed) layer first,
    // so overlay layers can consume events before layers beneath them.
    for (auto it = m_LayerStack.rbegin(); it != m_LayerStack.rend(); ++it) {
        if (e.Handled) {
            break;
        }
        (*it)->OnEvent(e);
    }
}

bool Application::OnWindowClose(WindowCloseEvent& e) {
    m_Running = false;
    CORE_LOG_INFO("Window close event received. Shutting down.");
    return true; // Return true: we handled it, no other layer needs it.
}