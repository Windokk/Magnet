#include "Window.h"

Magnet::Window::Window(int w, int h, std::string name): width{ w }, height{ h }, windowName{ name } {
	initWindow();
}

void Magnet::Window::createWindowSurface(VkInstance instance, VkSurfaceKHR* surface)
{
	if (glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS) {
		throw std::runtime_error("failed to create window surface!");
	}
}

void Magnet::Window::initWindow() {
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	window = glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr);
	glfwSetWindowUserPointer(window, this);
	glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
}

Magnet::Window::~Window() {
	glfwDestroyWindow(window);
	glfwTerminate();
}

void Magnet::Window::framebufferResizeCallback(GLFWwindow *window, int width, int height)
{
	auto _window = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));
	_window->frameBufferResized = true;
	_window->width = width;
	_window->height = height;

}