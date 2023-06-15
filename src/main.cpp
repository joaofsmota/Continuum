// game.cpp : Defines the entry point for the application.
//

#include <iostream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

using namespace std;

int main(int argc, char** argv)
{
	cout << "Hello game." << endl;

	GLFWwindow* window = glfwCreateWindow(800, 800, "game", nullptr, nullptr);

	return 0;
}
