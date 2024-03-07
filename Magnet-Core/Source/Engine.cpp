#include "Engine.h"
#include <iostream>

Engine::Engine()
{
	if (debugging) {
		std::cout << "Creating a Magnet Instance";
	}
	build_window();
}

Engine::~Engine()
{
	if (debugging) {
		std::cout << "Closing the Engine";
	}

	glfwTerminate();
}

void Engine::build_window()
{
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	if (window = glfwCreateWindow(window_params.width, window_params.height, window_params.window_name, nullptr, nullptr)) {
		if (debugging) {
			std::cout << "GLFW window creation : success";
			std::cout << "name : " << window_params.window_name << "  width : " << window_params.width << "  height : " << window_params.height;
		}
	}
	else {
		if (debugging) {
			std::cout << "GLFW window creation : failure";
		}
	}

}
