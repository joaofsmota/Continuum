// game.cpp : Defines the entry point for the application.
//

#include <iostream>
#include <assert.h>
#include <memory>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

using namespace std;

struct GLFWIntegration
{

	GLFWIntegration();
	GLFWIntegration(const int win_width, const int win_height, const char* win_title, GLFWmonitor* monitor, GLFWwindow* share);
	~GLFWIntegration();
	void init(const int win_width, const int win_height, const char* win_title, GLFWmonitor* monitor, GLFWwindow* share);
	inline void realease(void) {
		this->~GLFWIntegration();
	}
	inline GLFWwindow* get_window(void){
		return window; 
	}
private: 
	GLFWwindow* window;
};

GLFWIntegration::GLFWIntegration()
{
	glfwInit();
	window = NULL; 
}
GLFWIntegration::GLFWIntegration(const int win_width, const int win_height, const char* win_title, GLFWmonitor* monitor, GLFWwindow* share)
{
	glfwInit();
	this->window = glfwCreateWindow(win_width, win_height, win_title, monitor, share);
}
GLFWIntegration::~GLFWIntegration()
{
	glfwDestroyWindow(window);
	window = NULL;
	glfwTerminate();
}
void GLFWIntegration::init(const int win_width, const int win_height, const char* win_title, GLFWmonitor* monitor, GLFWwindow* share)
{
	if (window == NULL) window = glfwCreateWindow(win_width, win_height, win_title, monitor, share);
	else std::cout << "Failed to create glfw window, a glfw window already exists for this object" << std::endl;
}


int main(int argc, char** argv)
{
	GLFWIntegration glfw_integration = GLFWIntegration(800, 800, "game2", nullptr, nullptr);

	glfw_integration.init(1200, 800, "game2", nullptr, nullptr);

	glfw_integration.realease();

	return 0;
}