#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <iostream>

static unsigned int CompileShader(const std::string& source, unsigned int type) {
	unsigned int id = glCreateShader(GL_VERTEX_SHADER); // Create a shader object of the specified type
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


    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);


    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return -1;
    }

	std::cout << glGetString(GL_VERSION) << std::endl;

    float positions[6] = {
        -0.5f, -0.5f, // Bottom left
         0.5f, -0.5f, // Bottom right
         0.0f,  0.5f  // Top
	};

	unsigned int bufferID;
	glGenBuffers(1, &bufferID); // Generate a buffer ID
	glBindBuffer(GL_ARRAY_BUFFER, bufferID); // Bind the buffer to the GL_ARRAY_BUFFER target
	glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(float), positions, GL_STATIC_DRAW); // Upload the vertex data to the buffer

	glEnableVertexAttribArray(0); // Enable the vertex attribute at index 0
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0); // 2 floats per vertex, no normalization, stride of 2 floats, starting at the beginning of the buffer

    std::string vertexShader = 
        "#version 330 core\n"
        "layout(location = 0) in vec4 position;\n"
        "void main() {\n"
        "    gl_Position = position;\n"
	    "}\n";

    std::string fragmentShader =
        "#version 330 core\n"
        "layout(location = 0) out vec4 color;\n"
        "void main() {\n"
        "    color = vec4(1.0, 0.0, 0.0, 0.0);\n"
        "}\n";

	unsigned int shader = CreateShader(vertexShader, fragmentShader); // Create the shader program
    glUseProgram(shader);

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT);

		glDrawArrays(GL_TRIANGLES, 0, 3);
		//glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, nullptr);

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}