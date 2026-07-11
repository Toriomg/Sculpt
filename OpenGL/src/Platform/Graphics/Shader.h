#pragma once

#include <string>
#include <string_view>
#include <unordered_map>
#include "Platform/CoreUtils/glewDbg.h"

#include "Platform/CoreUtils/Math/maths.h"
#include "Platform/CoreUtils/Log.h"

struct ShaderProgramSource {
	std::string VertexSource; // Source code for the vertex shader
	std::string FragmentSource; // Source code for the fragment shader
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
	void SetUniform1i(const std::string& name, int value);
	void SetUniform1ui(const std::string& name, unsigned int value);
	void SetUniform1iv(const std::string& name, const int* array, int size);
	void SetUniform1f(const std::string& name, float v);
	void SetUniform3f(const std::string& name, float v0, float v1, float v2);
	void SetUniform4f(const std::string& name, float v0, float v1, float v2, float v3);
	void SetUniformMat4f(const std::string& name, const Matx4f& matrix);
private:
	static std::string ReadFile(std::string_view filepath);
	static ShaderProgramSource ParseShader(std::string_view filepath);
	unsigned int CompileShader(const std::string& source, unsigned int type);
	unsigned int CreateShader(const std::string& vertexShader, const std::string& fragmentShader);
	GLint GetUniformLocation(const std::string& name) const;
};