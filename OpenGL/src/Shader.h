#pragma once

#include <string>
#include <unordered_map>

#include <glm/glm.hpp> // Include GLM for matrix types

struct ShaderProgramSource {
	std::string VertexSource; // Source code for the vertex shader
	std::string FragmentSource; // Source code for the fragment shader
};

class Shader {
private:
	std::string m_FilePath;
	unsigned int m_RendererID;
	std::unordered_map<std::string, int> m_UniformLocationCache; // Cache for uniform locations
public:
	Shader(const std::string& filepath);
	~Shader();

	void Bind() const;
	void Unbind() const;

	// Set uniform functions
	void SetUniform1i(const std::string& name, int value);
	void SetUniform4f(const std::string& name, float v0, float v1, float v2, float v3);
	void SetUniformMat4f(const std::string& name, const glm::mat4& matrix);
private:
	ShaderProgramSource ParseShader(const std::string& filepath);
	unsigned int CompileShader(const std::string& source, unsigned int type);
	unsigned int CreateShader(const std::string& vertexShader, const std::string& fragmentShader);
	int GetUniformLocation(const std::string& name);
};