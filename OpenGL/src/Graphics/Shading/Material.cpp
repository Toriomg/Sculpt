#include "Material.h"

Material::Material(const std::shared_ptr<Shader>& shader)
    : m_Shader(shader)
{
}

void Material::AddTexture(const std::shared_ptr<Texture>& texture)
{
    m_Textures.push_back(texture);
}

void Material::Bind() const
{
    // 1. Bind the shader program.
    if (m_Shader)
    {
        m_Shader->Bind();

        // 2. Bind all textures to their corresponding texture units.
        //    We create an array of sampler IDs (0, 1, 2, ...) to pass to the shader.
        //    This assumes your shader has a uniform like: uniform sampler2D u_Textures[...];
        if (!m_Textures.empty())
        {
            // This is a common limit, adjust if needed.
            const int MAX_TEXTURE_SAMPLERS = 32;
            int samplers[MAX_TEXTURE_SAMPLERS];

            for (int i = 0; i < m_Textures.size() && i < MAX_TEXTURE_SAMPLERS; ++i)
            {
                m_Textures[i]->Bind(i); // Bind texture to slot 'i'
                samplers[i] = i;        // Set sampler to look at slot 'i'
            }

            // 3. Set the uniform in the shader to tell it which slots to sample from.
            m_Shader->SetUniform1iv("u_Textures", samplers, m_Textures.size());
        }

        // Future extension: Bind other material properties here
        // m_Shader->SetUniform4f("u_Color", m_Color.r, m_Color.g, m_Color.b, m_Color.a);
    }
}

void Material::Unbind() const
{
    // Unbind textures by binding 0 to their slots
    for (int i = 0; i < m_Textures.size(); ++i)
    {
        GLCall(glActiveTexture(GL_TEXTURE0 + i));
        GLCall(glBindTexture(GL_TEXTURE_2D, 0));
    }

    if (m_Shader)
    {
        m_Shader->Unbind();
    }
}