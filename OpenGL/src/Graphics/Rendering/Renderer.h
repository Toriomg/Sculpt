#pragma once
#include "RenderContext.h"
#include "RenderCommand.h"
#include "../../Scene/Components/MeshRendererComponent.h"
#include "../../Editor/inf_grid.h"

class Renderer
{
public:
	void RenderScene(RenderContext* context, const Matx4f& globalTransform, const Matx4f& MVP, const InfGrid& grid);
private:
	RenderCommand m_RendererCommand;
	float m_VertexHighlightRadius = 1.0f;
};