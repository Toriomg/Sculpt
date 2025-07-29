#include "Renderer.h"

void SetHighlightUniforms(const std::shared_ptr<Shader>& shader, bool isObject, bool isTriangle, bool isVertex,
	int triangleID, const Vec3& vertexPos, float vertexRadius)
{
	shader->SetUniform1i("u_IsSelected", isObject);
	shader->SetUniform1i("u_IsTriangleSelected", isTriangle);
	shader->SetUniform1i("u_IsVertexSelected", isVertex);

	if (isObject) {
		shader->SetUniform4f("u_HighlightColor", 0.0f, 0.0f, 0.55f, 1.0f);
	}
	if (isTriangle) {
		shader->SetUniform1i("u_SelectedTriangleID", triangleID);
		shader->SetUniform4f("u_TriangleHighlightColor", 0.7f, 0.0f, 0.0f, 1.0f);
	}
	if (isVertex) {
		shader->SetUniform3f("u_SelectedVertexWorldPos", vertexPos.x, vertexPos.y, vertexPos.z);
		shader->SetUniform4f("u_VertexHighlightColor", 1.0f, 0.0f, 1.0f, 1.0f);
		shader->SetUniform1f("u_VertexHighlightRadius", vertexRadius);
	}
}

void Renderer::RenderScene(RenderContext* context, const Matx4f& globalTransform, const Matx4f& MVP, const InfGrid& grid) {
	GLCall(glClearColor(0.7f, 0.5f, 0.5f, 1.0f)); // Set the clear color
	GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)); // Clear the color buffer

	const auto& camera = *context->m_Camera;
	Matx4f view = camera.GetViewMatrix();
	Matx4f projection = camera.GetProjectionMatrix();

	GLCall(glDisable(GL_DEPTH_TEST)); // Grid shouldn't hide objects
	GLCall(glEnable(GL_BLEND));
	GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

	// Call the grid's simple Draw method
	grid.Draw(view, projection, camera.m_Position);

	// 4. RESTORE STATE
	// Always clean up state for the next frame or the next renderer (e.g., ImGui).
	GLCall(glEnable(GL_DEPTH_TEST));
	GLCall(glDisable(GL_BLEND));


	for (auto& go : context->m_Scene->GetAllGameObjects()) { // Assuming Scene has a method to get all objects
		if (!go->m_IsVisible) continue;

		// Get the required components
		MeshRendererComponent* meshRenderer = go->GetComponent<MeshRendererComponent>();
		if (meshRenderer) {
			auto& material = meshRenderer->m_Material;
			auto& mesh = meshRenderer->m_Mesh;

			material->Bind();

			material->m_Shader->SetUniform1f("u_VertexHighlightRadius", m_VertexHighlightRadius);

			if (go.get() == context->m_SelectionContext->pSelectedObject) {
				SetHighlightUniforms(material->m_Shader, true, true, context->m_SelectionContext->IsVertexSelected,
					context->m_SelectionContext->SelectedTriangleID,
					context->m_SelectionContext->SelectedVertexWorldPos, m_VertexHighlightRadius);
			}
			else {
				SetHighlightUniforms(material->m_Shader, false, false, false, -1, {}, 0.0f);
			}

			
			Matx4f mvp = MVP * go->GetTransformMatrix();

			// 3. Set the MVP uniform (the material already bound the shader)
			material->m_Shader->SetUniformMat4f("u_MVP", mvp);
			material->m_Shader->SetUniformMat4f("u_Model", globalTransform * go->GetTransformMatrix());
			material->m_Shader->SetUniform3f("u_cameraPos", 
				context->m_Camera->m_Position.x,
				context->m_Camera->m_Position.y,
				context->m_Camera->m_Position.z);

			// 4. Draw the mesh
			m_RendererCommand.Draw(mesh->GetVAO(), mesh->GetIBO(), *material->m_Shader);
		}
	}


}