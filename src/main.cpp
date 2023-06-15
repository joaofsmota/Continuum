// game.cpp : Defines the entry point for the application.
//

#include <iostream>
#include <assert.h>
#include <memory>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

using namespace std;

#define glfw_window_ptr GLFWwindow* 

static glfw_window_ptr window = NULL; 
constexpr int WIN_WIDTH = 1200;
constexpr int WIN_HEIGHT = 800;


int main(int argc, char** argv)
{
	if (!glfwInit())
	{
		std::cout << "Failed to initialize glfw\n";
		return -1;
	}

	glfw_window_ptr window = NULL;

	if (!(window = glfwCreateWindow(WIN_WIDTH, WIN_HEIGHT, "game", nullptr, nullptr)))
	{
		glfwTerminate();
		return -1;
	}

	bool quit = true;

	while (!quit)
	{

	}

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}