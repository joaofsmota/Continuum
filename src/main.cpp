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

struct GlobalState {
    GLFWwindow* window = NULL;
    Continuum::Camera::OrbCameraPositioner positioner;
    struct mouse_state_t
    {
        glm::vec2 pos = glm::vec2(0.0f);
        bool pressed_left = false;
        bool pressed_right = false;
    } mouse_state;
} app;

namespace Renderer {

    struct PerFrameData
    {
        glm::mat4 view = {};
        glm::mat4 proj = {};
        glm::vec4 cam_pos = {};
    };

}

int main(int argc, char** argv)
{

    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        exit(EXIT_FAILURE);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    app.window = glfwCreateWindow(1200, 800, "Simple example", NULL, NULL);
    if (!app.window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwSetKeyCallback(app.window, key_callback);

    glfwMakeContextCurrent(app.window);
    glfwSwapInterval(1);

    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
        std::cerr << "Error: " << glewGetErrorString(err) << std::endl;
        glfwTerminate();
        return -1;
    }

    app.positioner = Continuum::Camera::OrbCameraPositioner(
        glm::vec3(0.0f, 0.5f, 0.0f),
        glm::vec3(0.0f, 0.0f, -1.0f),
        glm::vec3(0.0f, 1.0f, 0.0f)
    );

    Continuum::Camera::camera_t camera(app.positioner);

    const GLsizeiptr k_uniform_buffer_size = sizeof(Renderer::PerFrameData);

    GLuint per_frame_data_buffer;
    glCreateBuffers(1, &per_frame_data_buffer);
    glNamedBufferStorage(per_frame_data_buffer, k_uniform_buffer_size, NULL, GL_DYNAMIC_STORAGE_BIT);
    glBindBufferRange(GL_UNIFORM_BUFFER, 0, per_frame_data_buffer, 0, k_uniform_buffer_size);

    GLuint vao;
    glCreateVertexArrays(1, &vao);
    glBindVertexArray(vao);

    Continuum::Graphics::glsl_program_t grid_prog = Continuum::Graphics::glsl_program_t();

    grid_prog.compile_shader("C:/work/dev/game/shader/grid/grid.vert");
    grid_prog.compile_shader("C:/work/dev/game/shader/grid/grid.frag");
    grid_prog.link();
    grid_prog.validate();

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glfwSetCursorPosCallback(
        app.window,
        [](auto* window, double x, double y)
        {
            int width, height;
            glfwGetFramebufferSize(window, &width, &height);
            app.mouse_state.pos.x = static_cast<float>(x / width);
            app.mouse_state.pos.y = static_cast<float>(y / height);
        }
    );

    glfwSetMouseButtonCallback(
        app.window,
        [](auto* window, int button, int action, int mods)
        {
            if (button == GLFW_MOUSE_BUTTON_LEFT)
                app.mouse_state.pressed_left = action == GLFW_PRESS;
        }
    );

    glfwSetMouseButtonCallback(
        app.window,
        [](auto* window, int button, int action, int mods)
        {
            if (button == GLFW_MOUSE_BUTTON_RIGHT)
                app.mouse_state.pressed_right = action == GLFW_PRESS;
        }
    );

    glfwSetKeyCallback(
        app.window,
        [](GLFWwindow* window, int key, int scancode, int action, int mods)
        {
            const bool pressed = action != GLFW_RELEASE;
            if (key == GLFW_KEY_ESCAPE && pressed) glfwSetWindowShouldClose(window, GLFW_TRUE);
            // Proc Camera Movement
            if (key == GLFW_KEY_W) app.positioner.MOVEMENT_.forward_ = pressed;
            if (key == GLFW_KEY_S) app.positioner.MOVEMENT_.backward_ = pressed;
            if (key == GLFW_KEY_A) app.positioner.MOVEMENT_.left_ = pressed;
            if (key == GLFW_KEY_D) app.positioner.MOVEMENT_.right_ = pressed;
            if (key == GLFW_KEY_1) app.positioner.MOVEMENT_.up_ = pressed;
            if (key == GLFW_KEY_2) app.positioner.MOVEMENT_.down_ = pressed;
            if (mods & GLFW_MOD_SHIFT) app.positioner.MOVEMENT_.fast_speed_ = pressed;
            if (key == GLFW_KEY_SPACE) app.positioner.set_up_vector(glm::vec3(0.0f, 1.0f, 0.0f));
        }
    );

    double time_stamp = glfwGetTime();
    float delta_seconds = 0.0f;

    while (!glfwWindowShouldClose(app.window))
    {
        app.positioner.update(delta_seconds, app.mouse_state.pos, app.mouse_state.pressed_left);

        const double new_time_stamp = glfwGetTime();
        delta_seconds = static_cast<float>(new_time_stamp - time_stamp);
        time_stamp = new_time_stamp;

        int width, height;
        glfwGetFramebufferSize(app.window, &width, &height);
        const float ratio = width / (float)height;

        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        const glm::mat4 p = glm::perspective(45.0f, ratio, 0.1f, 1000.0f);
        const glm::mat4 view = camera.get_view_matrix();

        const Renderer::PerFrameData per_frame_data = { .view = view, .proj = p, .cam_pos = glm::vec4(camera.get_position(), 1.0f) };
        glNamedBufferSubData(per_frame_data_buffer, 0, k_uniform_buffer_size, &per_frame_data);

        grid_prog.use();
        glDrawArraysInstancedBaseInstance(GL_TRIANGLES, 0, 6, 1, 0);

        glfwSwapBuffers(app.window);
        glfwPollEvents();
    }

    glDeleteBuffers(1, &per_frame_data_buffer);
    glDeleteVertexArrays(1, &vao);

    grid_prog.~glsl_program_t();

    glfwDestroyWindow(app.window);

    glfwTerminate();

    exit(EXIT_SUCCESS);
}