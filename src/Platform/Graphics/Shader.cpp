#include <string>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include "Shader.hpp"

Shader::Shader(std::string_view filepath)
	: m_FilePath(filepath), m_RendererID(0)
{
	ShaderProgramSource source = ParseShader(m_FilePath);
	m_RendererID = CreateShader(source.VertexSource, source.FragmentSource);
}

Shader::Shader(std::string_view vertFilepath, std::string_view fragFilepath)
	: m_FilePath(vertFilepath), m_RendererID(0)
{
	std::string vertexSource   = ReadFile(std::string(vertFilepath));
	std::string fragmentSource = ReadFile(std::string(fragFilepath));

	// Check if reading was successful before proceeding
	if (vertexSource.empty() || fragmentSource.empty()) {
		m_RendererID = 0; // Mark this shader as invalid
		return;
	}

	// Pass the actual shader source code to CreateShader
	m_RendererID = CreateShader(vertexSource, fragmentSource);
}

Shader::~Shader() {
	GLCall(glDeleteProgram(m_RendererID)); // Delete the shader program
}

std::string Shader::ReadFile(std::string_view filepath) {
	std::ifstream file{std::string(filepath)};
	if (!file.is_open()) {
		LOG_ERROR( "Error: Could not open shader file: {0}", filepath);
		return ""; // Return empty string on failure
	}
	std::stringstream buffer;
	buffer << file.rdbuf();
	return buffer.str();
}

ShaderProgramSource Shader::ParseShader(std::string_view filepath) {
	std::ifstream stream{std::string(filepath)};

	enum class ShaderType {
		NONE = -1, VERTEX = 0, FRAGMENT = 1
	};

	std::string line; // String to hold each line of the shader file
	std::stringstream ss[2];
	ShaderType type = ShaderType::NONE; // Initialize the shader type to NONE

	// Read each line of the shader file
	while (getline(stream, line)) {
		if (line.find("#shader") != std::string::npos) { // Check if the line contains a shader directive
			// Check if the shader is a vertex shader
			if (line.find("vertex") != std::string::npos) {
				type = ShaderType::VERTEX; // Set the shader type to VERTEX
				CORE_LOG_TRACE("Shader Parser: Found Vertex Shader section");
			}
			// Check if the shader is a fragment shader
			else if (line.find("fragment") != std::string::npos) {
				type = ShaderType::FRAGMENT; // Set the shader type to FRAGMENT
				CORE_LOG_TRACE("Shader Parser: Found Fragment Shader section");
			}
		}
		else {
			// If the line does not contain a shader directive
			// Append the line to the appropriate shader type's stringstream
			if (type != ShaderType::NONE) {
				ss[static_cast<int>(type)] << line << '\n';
			}
		}
	}

	//CORE_LOG_TRACE("Vertex Shader Source:\n---\n{0}\n---", ss[0].str());
	//CORE_LOG_TRACE("Fragment Shader Source:\n---\n{0}\n---", ss[1].str());

	return { ss[0].str(), ss[1].str() }; // Return the vertex and fragment shader sources as a ShaderProgramSource struct
}

unsigned int Shader::CompileShader(const std::string& source, unsigned int type) {
	unsigned int id = glCreateShader(type); // Create a shader object of the specified type
	const char* src = source.c_str(); // Convert the source string to a C-style string
	GLCall(glShaderSource(id, 1, &src, nullptr)); // Attach the source code to the shader object
	GLCall(glCompileShader(id)); // Compile the shader

	int result;
	GLCall(glGetShaderiv(id, GL_COMPILE_STATUS, &result)); // Check if the shader compiled successfully
	if (result == GL_FALSE) { // If compilation failed
		int length;
		GLCall(glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length));

		std::vector<char> message(length); // <-- If length is 0, vector is empty
		GLCall(glGetShaderInfoLog(id, length, &length, message.data())); // <-- message.data() might be null

		LOG_ERROR("Failed to compile shader:\n{0}", message.data());

		GLCall(glDeleteShader(id)); // Delete the shader object
		return 0; // Return -1 to indicate failure
	}

	return id;
}

unsigned int Shader::CreateShader(const std::string& vertexShader, const std::string& fragmentShader) {
	/*Must pass the parsed shader not the filepath*/
	unsigned int program = glCreateProgram(); // Create a shader program
	unsigned int vs = CompileShader(vertexShader, GL_VERTEX_SHADER); // Compile the vertex shader
	unsigned int fs = CompileShader(fragmentShader, GL_FRAGMENT_SHADER); // Compile the vertex shader

	// If compilation failed, vs or fs will be 0. Abort creation.
	if (vs == 0 || fs == 0) {
		// The error message was already printed inside CompileShader
		GLCall(glDeleteProgram(program)); // Clean up the program objec)t
		// Clean up the successfully compiled shader
		if (vs != 0) GLCall(glDeleteShader(vs));
		if (fs != 0) GLCall(glDeleteShader(fs));
		return 0;
	}

	GLCall(glAttachShader(program, vs));
	GLCall(glAttachShader(program, fs));
	// Link the shader program
	GLCall(glLinkProgram(program)); 

	int linkResult;
	GLCall(glGetProgramiv(program, GL_LINK_STATUS, &linkResult));
	if (linkResult == GL_FALSE) {
		int length;
		GLCall(glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length));
		std::vector<char> message(length);
		GLCall(glGetProgramInfoLog(program, length, &length, message.data()));

		LOG_ERROR("Failed to link shader program!\n\t\tSource File: {0}\n\t\tGLSL Linker Error : {1}",
			m_FilePath, message.data());

		GLCall(glDeleteProgram(program));
		GLCall(glDeleteShader(vs));
		GLCall(glDeleteShader(fs));
		return 0;
	}

	// Validate the shader program
	GLCall(glValidateProgram(program)); 

	int validateResult;
	GLCall(glGetProgramiv(program, GL_VALIDATE_STATUS, &validateResult));
	if (validateResult == GL_FALSE) {
		int length;
		GLCall(glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length));
		std::vector<char> message(length);
		GLCall(glGetProgramInfoLog(program, length, &length, message.data()));

		LOG_ERROR("Failed to validate shader program!\n\t\tSource File: {0}\n\t\tGLSL Validater Error : {1}",
			m_FilePath, message.data());

	}

	GLCall(glDeleteShader(vs)); // Delete the vertex shader object
	GLCall(glDeleteShader(fs)); // Delete the fragment shader object
	CORE_LOG_TRACE("Shader {0} Created correctly", m_FilePath);
	return program; // Return the shader program ID
}

void Shader::Bind() const {
	GLCall(glUseProgram(m_RendererID));
}

void Shader::Unbind() const {
	GLCall(glUseProgram(0));
}

void Shader::SetUniform1i(const std::string& name, int value) {
	GLCall(glUniform1i(GetUniformLocation(name), value));
}

void Shader::SetUniform1ui(const std::string& name, unsigned int value) {
	GLCall(glUniform1ui(GetUniformLocation(name), value));
}

void Shader::SetUniform1iv(const std::string& name, const int* array, int size) {
	GLCall(glUniform1iv(GetUniformLocation(name), size, array));
}

void Shader::SetUniform1f(const std::string& name, float v) {
	GLCall(glUniform1f(GetUniformLocation(name), v));
}

void Shader::SetUniform3f(const std::string& name, float v0, float v1, float v2) {
	GLCall(glUniform3f(GetUniformLocation(name), v0, v1, v2));
}

void Shader::SetUniform4f(const std::string& name, float v0, float v1, float v2, float v3) {
	GLCall(glUniform4f(GetUniformLocation(name), v0, v1, v2, v3));
}

void Shader::SetUniformMat4f(const std::string& name, const Matx4f& matrix) {
    // GL_TRUE: GLSL expects column-major order, but Matx4f is row-major, so we transpose on upload.
	GLCall(glUniformMatrix4fv(GetUniformLocation(name), 1, GL_TRUE, &matrix.m[0][0]));
}

GLint Shader::GetUniformLocation(const std::string& name) const {
	auto [it, inserted] = m_UniformLocationCache.try_emplace(name, -1);
	if (inserted) {
		it->second = glGetUniformLocation(m_RendererID, name.c_str());
		if (it->second == -1)
			LOG_ERROR("Warning: uniform '{0}' doesn't exist!", name);
	}
	return it->second;
}