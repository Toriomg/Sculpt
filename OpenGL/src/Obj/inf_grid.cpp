#include "inf_grid.h"

InfGrid::InfGrid() {
	m_Shader = std::make_unique<Shader>("shaders/grid.vert", "shaders/grid.frag");
    m_VAO = std::make_unique<VertexArray>();
}

InfGrid::~InfGrid() {
}

void InfGrid::OnRender(const Camera& camera) const {
    // Don't render if it's not supposed to be visible.
    if (!m_IsVisible) {
        return;
    }

    // The grid should not block objects behind it, so we disable depth writing.
    glDisable(GL_DEPTH_TEST);

    m_Shader->Bind();

    // Set the shader uniforms with data from the camera.
    m_Shader->SetUniformMat4fm("u_view", camera.GetViewMatrix());
    m_Shader->SetUniformMat4fm("u_projection", camera.GetProjectionMatrix(true));
    m_Shader->SetUniform3f("u_cameraPos", camera.m_Position.x, camera.m_Position.y, camera.m_Position.z);

    // Bind the VAO.
	m_VAO->Bind();

    // Make the draw call.
    // We tell OpenGL to draw 6 vertices. The vertex shader will generate them on-the-fly.
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // Clean up state for the next object to render.
    m_VAO->Bind();
    glEnable(GL_DEPTH_TEST); // Re-enable depth testing!
}