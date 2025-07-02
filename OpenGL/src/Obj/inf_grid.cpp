#include "inf_grid.h"

InfGrid::InfGrid() {
	m_Shader = std::make_unique<Shader>("shaders/grid.vert", "shaders/grid.frag");
	glGenVertexArrays(1, &m_VaoID);
}

InfGrid::~InfGrid() {
	glDeleteVertexArrays(1, &m_VaoID);
	// m_Shader is a unique_ptr, so it's automatically deleted.
}

void InfGrid::OnRender(const Camera& camera) const {
    // Don't render if it's not supposed to be visible.
    if (!m_IsVisible) {
        return;
    }

    // The grid should not block objects behind it, so we disable depth writing.
    glDisable(GL_DEPTH_TEST);

    m_Shader->use();

    // Set the shader uniforms with data from the camera.
    m_Shader->SetUniformMat4fm("u_view", camera.GetViewMatrix());
    m_Shader->SetUniformMat4fm("u_projection", camera.GetProjectionMatrix(true));
    m_Shader->SetUniform3f("u_cameraPos", camera.m_Position.x, camera.m_Position.y, camera.m_Position.z);

    // Bind the VAO.
    glBindVertexArray(m_VaoID);

    // Make the draw call.
    // We tell OpenGL to draw 6 vertices. The vertex shader will generate them on-the-fly.
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // Clean up state for the next object to render.
    glBindVertexArray(0);
    glEnable(GL_DEPTH_TEST); // Re-enable depth testing!
}