#pragma once
#include <GLFW/glfw3.h>

#include <vulkan/vulkan.hpp>

struct Window {
	int width;
	int height;
	bool UseWindowBar;
	char* window_name;
};

class Engine {

public :

	Engine();
	~Engine();

private:

	bool debugging = true;

	Window window_params;

	GLFWwindow* window{ nullptr };

	void build_window();
};