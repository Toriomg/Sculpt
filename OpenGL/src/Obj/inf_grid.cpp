#include "inf_grid.h"

InfGrid::InfGrid() {
	m_Shader = std::make_unique<Shader>("res/shaders/vert/grid.vert", "res/shaders/frag/grid.frag");
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
    //glDepthFunc(GL_LEQUAL);
    //glDepthMask(GL_FALSE);

    m_Shader->Bind();

    // Set the shader uniforms with data from the camera.
    m_Shader->SetUniformMat4fm("u_view", camera.GetViewMatrix());
    m_Shader->SetUniformMat4fm("u_projection", camera.GetProjectionMatrix(true));
    m_Shader->SetUniform3f("u_cameraPos", camera.m_Position.x, camera.m_Position.y, camera.m_Position.z);

    // Ujbind the VAO.
    m_VAO->Unbind();

    // Make the draw call.
    // We tell OpenGL to draw 6 vertices. The vertex shader will generate them on-the-fly.
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // Clean up state for the next object to render.
    //glDepthMask(GL_TRUE);
    //glDepthFunc(GL_LESS); // Re-enable depth testing!
}