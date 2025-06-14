#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

// Macro to assert conditions, triggering a breakpoint if false
#define ASSERT(x) if (!(x)) __debugbreak() 
// Macro to clear OpenGL errors before and after a function call
#define GLCall(x) GLClearError(); x; ASSERT(GLLogCall(#x, __FILE__, __LINE__)) 

static void GLClearError() {
    while (glGetError() != GL_NO_ERROR); // Clear all OpenGL errors
}

static bool GLLogCall(const char* function, const char* file, int line) {
    while (GLenum error = glGetError()) { // Check for OpenGL errors
        std::cerr << "[OpenGL Error] (" << error << "): " 
			<< function << " " << file << ":" << line << std::endl; // Log the error
		return false; // Return false if an error occurred
    }
	return true; // Return true if no errors occurred
}

struct ShaderProgramSource {
    std::string VertexSource; // Source code for the vertex shader
    std::string FragmentSource; // Source code for the fragment shader
};

static ShaderProgramSource ParseShader(const std::string& filepath) {
	std::ifstream stream(filepath); // Open the shader file

	enum class ShaderType{
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

static unsigned int CompileShader(const std::string& source, unsigned int type) {
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

static unsigned int CreateShader(const std::string& vertexShader, const std::string& fragmentShader) {
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

int main(void)
{
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // Set the major version of OpenGL
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3); // Set the minor version of OpenGL
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // Use the core profile of OpenGL

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

	glfwSwapInterval(1); // Enable vsync

    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return -1;
    }

	std::cout << glGetString(GL_VERSION) << std::endl;

    float positions[] = {
        -0.5f, -0.5f, // Bottom left
         0.5f, -0.5f, // Bottom right
         0.5f,  0.5f,  // Top right
        -0.5f,  0.5f // Top left
	};

	unsigned int indices[6] = { 
        0, 1, 2,
		2, 3, 0
    };

	unsigned int vaoID;
    GLCall(glGenVertexArrays(1, &vaoID)); // Generate a Vertex Array Object (VAO)
    GLCall(glBindVertexArray(vaoID)); // Bind the VAO
	// Create a Vertex Buffer Object (VBO) and upload vertex data

	unsigned int bufferID;
    GLCall(glGenBuffers(1, &bufferID)); // Generate a buffer ID
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, bufferID)); // Bind the buffer to the GL_ARRAY_BUFFER target
    GLCall(glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW)); // Upload the vertex data to the buffer

    GLCall(glEnableVertexAttribArray(0)); // Enable the vertex attribute at index 0
    GLCall(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0)); // 2 floats per vertex, no normalization, stride of 2 floats, starting at the beginning of the buffer

    unsigned int iboID;
    GLCall(glGenBuffers(1, &iboID));
    GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboID));
    GLCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW));

	// Parse the shader file
	ShaderProgramSource source = ParseShader("res/shaders/Basic.shader"); // Parse the shader file

    unsigned int shader = CreateShader(source.VertexSource, source.FragmentSource); // Create the shader program
    GLCall(glUseProgram(shader));

	GLCall(int location = glGetUniformLocation(shader, "u_Color")); // Get the location of the uniform variable
	ASSERT(location != -1); // Ensure the uniform variable was found
    GLCall(glUniform4f(location, 0.2f, 0.3f, 0.8f, 1.0f));

	GLCall(glBindVertexArray(vaoID)); // Bind the VAO
	GLCall(glUseProgram(0));
	GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0));
	GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));

    float r = 0.2f;
    float g = 0.3f;
    float b = 0.8f;
    float a = 1.0f;
	float var = 0.01f; // Variable to control color change
    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT);

        GLCall(glUseProgram(shader));
        GLCall(glUniform4f(location, r, g, b, a));

		GLCall(glBindVertexArray(vaoID)); // Bind the VAO
        GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboID));

		GLCall(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr));

        if (r >= 1.0f) {
			var = -0.01f; // Reverse direction when reaching 1.0
		}
		else if (r <= 0.0f) {
			var = 0.01f; // Reverse direction when reaching 0.0
		}
		r += var; // Increment the red component

        /* Swap front and back buffers */
        glfwSwapBuffers(window);
        /* Poll for and process events */
        glfwPollEvents();
    }

	glDeleteProgram(shader); // Delete the shader program
    glfwTerminate();
    return 0;
}