// game.cpp : Defines the entry point for the application.
//
#include "core/ccore.h"

#include <iostream>

static const char* vertex_shader_text = R"GLSL(
		#version 330 core
        layout (location = 0) in vec3 aPos;
		void main()
		{
			gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);
		}
	)GLSL";

static const char* fragment_shader_text = R"GLSL(
		#version 330 core
		out vec4 FragColor;
		void main()
		{
			FragColor = vec4(1.0f, 1.0f, 0.2f, 1.0f);
		}
	)GLSL";

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

static Continuum::CameraPositionerOrb positioner(
    glm::vec3(0.0f, 0.5f, 0.0f),
    glm::vec3(0.0f, 0.0f, -1.0f),
    glm::vec3(0.0f, 1.0f, 0.0f)
);
static Continuum::camera_t camera(positioner);

struct mouse_state_t
{
    glm::vec2 pos = glm::vec2(0.0f);
    bool pressed_left = false;
    bool pressed_right = false;
} mouse_state;

int main(int argc, char** argv)
{
    GLFWwindow* window;
    GLuint VBO, VAO;
    Continuum::Graphics::glsl_program_t test_program = Continuum::Graphics::glsl_program_t();

    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        exit(EXIT_FAILURE);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(1200, 800, "Simple example", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwSetKeyCallback(window, key_callback);

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
        std::cerr << "Error: " << glewGetErrorString(err) << std::endl;
        glfwTerminate();
        return -1;
    }

    test_program.compile_shader(vertex_shader_text, Continuum::Graphics::GLSLShader::VERTEX, NULL);
    test_program.compile_shader(fragment_shader_text, Continuum::Graphics::GLSLShader::FRAGMENT, NULL);
    test_program.link();

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    float vertices[] = {
        -0.5f, -0.5f, 0.0f, // left  
         0.5f, -0.5f, 0.0f, // right 
         0.0f,  0.5f, 0.0f  // top   
    };

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
    // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
    glBindVertexArray(0);

    // uncomment this call to draw in wireframe polygons.
//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    test_program.validate();

    glfwSetCursorPosCallback(
        window,
        [](auto* window, double x, double y)
        {
            int width, height;
            glfwGetFramebufferSize(window, &width, &height);
            mouse_state.pos.x = static_cast<float>(x / width);
            mouse_state.pos.y = static_cast<float>(y / height);
        }
    );

    glfwSetMouseButtonCallback(
        window,
        [](auto* window, int button, int action, int mods)
        {
            if (button == GLFW_MOUSE_BUTTON_LEFT)
                mouse_state.pressed_left = action == GLFW_PRESS;
        }
    );

    glfwSetMouseButtonCallback(
        window,
        [](auto* window, int button, int action, int mods)
        {
            if (button == GLFW_MOUSE_BUTTON_RIGHT)
                mouse_state.pressed_right = action == GLFW_PRESS;
        }
    );

    glfwSetKeyCallback(
        window,
        [](GLFWwindow* window, int key, int scancode, int action, int mods)
        {
            const bool pressed = action != GLFW_RELEASE;
            if (key == GLFW_KEY_ESCAPE && pressed) glfwSetWindowShouldClose(window, GLFW_TRUE);
            // Proc Camera Movement
            if (key == GLFW_KEY_W) positioner.MOVEMENT_.forward_ = pressed;
            if (key == GLFW_KEY_S) positioner.MOVEMENT_.backward_ = pressed;
            if (key == GLFW_KEY_A) positioner.MOVEMENT_.left_ = pressed;
            if (key == GLFW_KEY_D) positioner.MOVEMENT_.right_ = pressed;
            if (key == GLFW_KEY_1) positioner.MOVEMENT_.up_ = pressed;
            if (key == GLFW_KEY_2) positioner.MOVEMENT_.down_ = pressed;
            if (mods & GLFW_MOD_SHIFT) positioner.MOVEMENT_.fast_speed_ = pressed;
            if (key == GLFW_KEY_SPACE) positioner.set_up_vector(glm::vec3(0.0f, 1.0f, 0.0f));
        }
    );

    double timeStamp = glfwGetTime();
    float deltaSeconds = 0.0f;

    while (!glfwWindowShouldClose(window))
    {
        float ratio;
        int width, height;



        glfwGetFramebufferSize(window, &width, &height);
        ratio = width / (float)height;

        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT);



        test_program.use();
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    test_program.~glsl_program_t();

    glfwDestroyWindow(window);

    glfwTerminate();

    exit(EXIT_SUCCESS);
}