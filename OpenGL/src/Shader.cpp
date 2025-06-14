#include <string>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include "Renderer.h"
#include "Shader.h"

Shader::Shader(const std::string& filepath) 
	: m_FilePath(filepath), m_RendererID(0)
{
	ShaderProgramSource source = ParseShader(filepath); // Parse the shader fill
	m_RendererID = CreateShader(source.VertexSource, source.FragmentSource); // Create the shader program
}

Shader::~Shader() {
	GLCall(glDeleteProgram(m_RendererID)); // Delete the shader program
}

ShaderProgramSource Shader::ParseShader(const std::string& filepath) {
	std::ifstream stream(filepath); // Open the shader file

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
			}
			// Check if the shader is a fragment shader
			else if (line.find("fragment") != std::string::npos) {
				type = ShaderType::FRAGMENT; // Set the shader type to FRAGMENT
			}
		}
		else {
			// If the line does not contain a shader directive
			// Append the line to the appropriate shader type's stringstream
			ss[(int)type] << line << '\n';
		}
	}

	return { ss[0].str(), ss[1].str() }; // Return the vertex and fragment shader sources as a ShaderProgramSource struct
}
unsigned int Shader::CompileShader(const std::string& source, unsigned int type) {
	unsigned int id = glCreateShader(type); // Create a shader object of the specified type
	const char* src = source.c_str(); // Convert the source string to a C-style string
	glShaderSource(id, 1, &src, nullptr); // Attach the source code to the shader object
	glCompileShader(id); // Compile the shader

	int result;
	glGetShaderiv(id, GL_COMPILE_STATUS, &result); // Check if the shader compiled successfully
    if (result == GL_FALSE) { // If compilation failed
        int length;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length); // Get the length of the error message
		char* message = (char*)_malloca(length * sizeof(char)); // Create a buffer for the error message
        glGetShaderInfoLog(id, length, &length, message); // Get the error message
        std::cerr << "Failed to compile" 
            << (type == GL_VERTEX_SHADER ? "vertex" : "fragment")
            << "shader!" << std::endl; // Print an error message
        std::cerr << message << std::endl; // Print the error message
        glDeleteShader(id); // Delete the shader object
		return 0; // Return -1 to indicate failure
    }

    return id;
}

unsigned int Shader::CreateShader(const std::string& vertexShader, const std::string& fragmentShader) {
	unsigned int program = glCreateProgram(); // Create a shader program
	unsigned int vs = CompileShader(vertexShader, GL_VERTEX_SHADER); // Compile the vertex shader
	unsigned int fs = CompileShader(fragmentShader, GL_FRAGMENT_SHADER); // Compile the vertex shader

	glAttachShader(program, vs);
	glAttachShader(program, fs);
	glLinkProgram(program); // Link the shader program
	glValidateProgram(program); // Validate the shader program

	glDeleteShader(vs); // Delete the vertex shader object
	glDeleteShader(fs); // Delete the fragment shader object

	return program; // Return the shader program ID
}

void Shader::Bind() const {
	GLCall(glUseProgram(m_RendererID)); // Bind the shader program for use
}

void Shader::Unbind() const {
	GLCall(glUseProgram(0)); // Unbind the shader program
}

void Shader::SetUniform4f(const std::string& name, float v0, float v1, float v2, float v3) {
	GLCall(glUniform4f(GetUniformLocation(name), v0, v1, v2, v3)); // Set a 4D float uniform variable in the shader
}

int Shader::GetUniformLocation(const std::string& name) {
	if (m_UniformLocationCache.find(name) != m_UniformLocationCache.end()) {
		return m_UniformLocationCache[name]; // Return cached location if it exists
	}
	GLCall(int location = glGetUniformLocation(m_RendererID, name.c_str()));
	if (location == -1) {
		std::cerr << "Warning: uniform '" << name << "' doesn't exist!" << std::endl;
	}
	m_UniformLocationCache[name] = location; // Cache the uniform location if found
	return location;
}