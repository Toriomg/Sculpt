#include "inf_grid.h"

InfGrid::InfGrid() {
	m_Shader = std::make_unique<Shader>("res/shaders/vert/grid.vs", "res/shaders/frag/grid.fs");
}

InfGrid::~InfGrid() {
}

void InfGrid::Draw(const Matx4f& view, const Matx4f& projection, const Vec3& cameraPos) const {
    m_Shader->Bind();
    m_Shader->SetUniformMat4f("u_view", view);
    m_Shader->SetUniformMat4f("u_projection", projection);
    m_Shader->SetUniform3f("u_cameraPos", cameraPos.x, cameraPos.y, cameraPos.z);

    // This is a special full-screen triangle trick, so we draw 6 vertices.
    glDrawArrays(GL_TRIANGLES, 0, 6);
}