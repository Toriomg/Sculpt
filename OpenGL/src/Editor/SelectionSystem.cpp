#include "SelectionSystem.h"

SelectionSystem::SelectionSystem(int WindowWidth, int WindowHeight)
	: m_PickingShader("res/shaders/Picking.shader"),
	m_PickingTexture(WindowWidth, WindowHeight) 
{
}

void SelectionSystem::UpdatePickingPass(const Scene& scene, const Camera& camera, const Matx4f& globalTransform, const Matx4f& MVP) {
	// Chunk of spaghetti code to handle picking
	m_PickingTexture.EnableWriting();


	// Empty the buffer color and depth
	const GLuint clearColor[4] = { 0, 0, 0, 0 };
	GLCall(glClearBufferuiv(GL_COLOR, 0, clearColor));
	GLCall(glClear(GL_DEPTH_BUFFER_BIT)); // También limpia el buffer de profundidad

	m_PickingShader.Bind();

	// 5. Dibujar cada objeto seleccionable
	unsigned int objectId = 1; // Empezamos los IDs en 1 (0 es para el fondo)
	for (auto& go : scene.GetAllGameObjects()) {
		MeshRendererComponent* meshRenderer = go->GetComponent<MeshRendererComponent>();
		if (meshRenderer) {
			// Asignar el ID al objeto (puedes guardarlo en el propio GameObject)
			go->SetPickingID(objectId);
			m_PickingShader.SetUniform1ui("objectID", go->GetPickingID());

			Matx4f model_MVP = MVP * go->GetTransformMatrix();

			m_PickingShader.SetUniformMat4f("u_Model", globalTransform * go->GetTransformMatrix());
			m_PickingShader.SetUniformMat4f("u_MVP", model_MVP);

			// Draw the mesh with the picking shader
			auto& mesh = meshRenderer->m_Mesh;
			m_RenderCommand.Draw(mesh->GetVAO(), mesh->GetIBO(), m_PickingShader);

			objectId++;
		}
	}

	m_PickingTexture.DisableWriting();
}
void SelectionSystem::ProcessSelectionClick(const Scene& scene, const Matx4f& globalTransform, int mouseX, int mouseY, float windowHeight, SelectionContext& context) {
	if (!g_MouseState.rightButtonFirstPress)
		return;

	unsigned int mouseX = static_cast<unsigned int>(g_MouseState.lastX);
	unsigned int mouseY = static_cast<unsigned int>(windowHeight - g_MouseState.lastY);

	std::cout << "Mouse clicked at: (" << mouseX << ", " << mouseY << ")" << std::endl;
	PickingTexture::PixelInfo Pixel = m_PickingTexture.ReadPixel(mouseX, mouseY);

	if (Pixel.ObjectID == 0) {
		context.pSelectedObject = nullptr;
		context.SelectedObjectID = 0;
		context.SelectedTriangleID = -1;
		context.IsVertexSelected = false;
		std::cout << "No object clicked. Clearing selection." << std::endl;
		return;
	}

	context.SelectedObjectID = Pixel.ObjectID;
	context.SelectedTriangleID = Pixel.PrimID;
	context.IsVertexSelected = false; // Reset until we calculate the vertex
	context.pSelectedObject = nullptr; // Reset until we find the object

	for (auto& go : scene.GetAllGameObjects()) {
		if (go->GetPickingID() == context.SelectedObjectID) {
			context.pSelectedObject = go.get();
			break;
		}
	}
	if (!context.pSelectedObject) {
		std::cerr << "No object found with ID: " << context.SelectedObjectID << std::endl;
		return;
	}
	Vec3 clickedWorldPos = m_PickingTexture.ReadWorldPosition(mouseX, mouseY);

	// Get mesh and transform
	MeshRendererComponent* mrc = context.pSelectedObject->GetComponent<MeshRendererComponent>();

	Matx4f modelMatrix = globalTransform * context.pSelectedObject->GetTransformMatrix();

	auto& indices = mrc->m_Mesh->GetIndices();
	auto& vertices = mrc->m_Mesh->GetVertices();

	// Get the 3 vertices of the selected triangle
	unsigned int i0 = indices[context.SelectedTriangleID * 3 + 0];
	unsigned int i1 = indices[context.SelectedTriangleID * 3 + 1];
	unsigned int i2 = indices[context.SelectedTriangleID * 3 + 2];

	Vec3 v0_local = vertices[i0].pos;
	Vec3 v1_local = vertices[i1].pos;
	Vec3 v2_local = vertices[i2].pos;

	// Transform them to world space
	Vec3 v0_world = modelMatrix.transformPoint(v0_local);
	Vec3 v1_world = modelMatrix.transformPoint(v1_local);
	Vec3 v2_world = modelMatrix.transformPoint(v2_local);

	// Find which vertex is closest to the click point
	float d0 = (clickedWorldPos - v0_world).length();
	float d1 = (clickedWorldPos - v1_world).length();
	float d2 = (clickedWorldPos - v2_world).length();

	float min_dist = std::min({ d0, d1, d2 });

	context.IsVertexSelected = true;
	if (min_dist == d0) context.SelectedVertexWorldPos = v0_world;
	else if (min_dist == d1) context.SelectedVertexWorldPos = v1_world;
	else context.SelectedVertexWorldPos = v2_world;

	std::cout << "Selected Vertex Pos: (" << context.SelectedVertexWorldPos.x << ", "
		<< context.SelectedVertexWorldPos.y << ", " << context.SelectedVertexWorldPos.z << ")\n";
}