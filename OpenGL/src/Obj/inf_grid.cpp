#include "inf_grid.h"

InfGrid::InfGrid() {
	m_Shader = std::make_unique<Shader>("res/shaders/vert/grid.vs", "res/shaders/frag/grid.fs");
}

InfGrid::~InfGrid() {
}

void InfGrid::OnRender(const Camera& camera, bool CameraPersEnabled) const {
    // Don't render if it's not supposed to be visible.
    if (!m_IsVisible) {
        return;
    }

    // The grid should not block objects behind it, so we disable depth writing.
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    m_Shader->Bind();

    // Set the shader uniforms with data from the camera.
    m_Shader->SetUniformMat4f("u_view", camera.GetViewMatrix());
    m_Shader->SetUniformMat4f("u_projection", camera.GetProjectionMatrix(CameraPersEnabled));
    m_Shader->SetUniform3f("u_cameraPos", camera.m_Position.x, camera.m_Position.y, camera.m_Position.z);

    // Make the draw call.
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // Clean up state for the next object to render.
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
}