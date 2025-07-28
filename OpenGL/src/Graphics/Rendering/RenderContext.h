#pragma once
#include "../../Core/Camera.h"
#include "../../Scene/Scene.h"
#include "SelectionContext.h"

struct RenderContext {
	Camera& m_Camera;
	Scene& m_Scene;
	SelectionContext& m_SelectionContext;
};