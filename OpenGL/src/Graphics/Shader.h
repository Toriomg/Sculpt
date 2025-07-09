#pragma once

#include <string>
#include <unordered_map>

#include "../Math/maths.h"

struct ShaderProgramSource {
	std::string VertexSource; // Source code for the vertex shader
	std::string FragmentSource; // Source code for the fragment shader
};

class Shader {
private:
	std::string m_FilePath;
	unsigned int m_RendererID;
	mutable std::unordered_map<std::string, GLint> m_UniformLocationCache; // Cache for uniform locations
public:
	Shader(const std::string& filepath);
	Shader(const std::string& vertFilepath, const std::string& fragFilepath);
	~Shader();

	void Bind() const;
	void Unbind() const;

	// Set uniform functions
	void SetUniform1i(const std::string& name, int value);
	void SetUniform1iv(const std::string& name, const int* array, int size);
	void SetUniform3f(const std::string& name, float v0, float v1, float v2);
	void SetUniform4f(const std::string& name, float v0, float v1, float v2, float v3);
	void SetUniformMat4f(const std::string& name, const Matx4f& matrix);
private:
	static std::string ReadFile(const std::string& filepath);
	ShaderProgramSource ParseShader(const std::string& filepath);
	unsigned int CompileShader(const std::string& source, unsigned int type);
	unsigned int CreateShader(const std::string& vertexShader, const std::string& fragmentShader);
	GLint GetUniformLocation(const std::string& name) const;
};