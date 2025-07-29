#pragma once
#include "../Graphics/Rendering/RenderContext.h"
#include "PickingTexture.h"
#include "../Graphics/Shading/Shader.h"
#include "../Graphics/Rendering/RenderCommand.h"
#include "../Scene/Components/MeshRendererComponent.h"
#include "../Graphics/Geometry/Mesh.h"

class SelectionSystem{
public:
	SelectionSystem(int WindowWidth, int WindowHeight);

	void UpdatePickingPass(const Scene& scene, const Camera& camera, const Matx4f& globalTransform, const Matx4f& MVP);
	void ProcessSelectionClick(const Scene& scene, const Matx4f& globalTransform, int mouseX, int mouseY, SelectionContext& context);
private:
	PickingTexture m_PickingTexture;
	Shader m_PickingShader;
	RenderCommand m_RenderCommand;
};
