// Compiles GLSL programs from #type-delimited .shader files; caches uniform locations; used by
// Material.
#pragma once

#include "Platform/CoreUtils/glewDbg.hpp"
#include <string>
#include <string_view>
#include <unordered_map>

#include "Platform/CoreUtils/Log.hpp"
#include "Platform/CoreUtils/Math/maths.hpp"

struct ShaderProgramSource {
    std::string VertexSource;    // Source code for the vertex shader
    std::string FragmentSource;  // Source code for the fragment shader
};

class Shader {
private:
    std::string m_FilePath;
    unsigned int m_RendererID;
    mutable std::unordered_map<std::string, GLint> m_UniformLocationCache;

public:
    explicit Shader(std::string_view filepath);
    Shader(std::string_view vertFilepath, std::string_view fragFilepath);
    ~Shader();

    void Bind() const;
    void Unbind() const;

    // Set uniform functions
    void SetUniform1i(std::string const& name, int value);
    void SetUniform1ui(std::string const& name, unsigned int value);
    void SetUniform1iv(std::string const& name, int const* array, int size);
    void SetUniform1f(std::string const& name, float v);
    void SetUniform3f(std::string const& name, float v0, float v1, float v2);
    void SetUniform4f(std::string const& name, float v0, float v1, float v2, float v3);
    void SetUniformMat4f(std::string const& name, Matx4f const& matrix);

private:
    static std::string ReadFile(std::string_view filepath);
    static ShaderProgramSource ParseShader(std::string_view filepath);
    unsigned int CompileShader(std::string const& source, unsigned int type);
    unsigned int CreateShader(std::string const& vertexShader, std::string const& fragmentShader);
    GLint GetUniformLocation(std::string const& name) const;
};
