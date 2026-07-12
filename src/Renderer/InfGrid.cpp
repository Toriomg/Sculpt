#include "Renderer/InfGrid.hpp"
#include <GL/glew.h>

InfGrid::InfGrid() {
    m_Shader = std::make_unique<Shader>("res/shaders/grid.shader");
    glGenVertexArrays(1, &m_VAO);
}

InfGrid::~InfGrid() {
    glDeleteVertexArrays(1, &m_VAO);
}

void InfGrid::Draw(const Matx4f& view, const Matx4f& projection, const Vec3& cameraPos) const {
    // Grid vertices sit on the far plane (NDC z=1). GL_LEQUAL lets them pass where the
    // depth buffer still holds the clear value (1.0), i.e. pixels with no scene geometry.
    // Depth writes are suppressed so the grid does not occlude subsequent transparent draws.
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthFunc(GL_LEQUAL);
    glDepthMask(GL_FALSE);

    m_Shader->Bind();
    m_Shader->SetUniformMat4f("u_view", view);
    m_Shader->SetUniformMat4f("u_projection", projection);
    m_Shader->SetUniform3f("u_cameraPos", cameraPos.x, cameraPos.y, cameraPos.z);

    // The vertex positions are generated inside the vertex shader from gl_VertexID;
    // only a bound VAO is required by the core profile — no vertex data is uploaded.
    glBindVertexArray(m_VAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LESS);
    glDisable(GL_BLEND);
}
