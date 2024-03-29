#pragma once
#include "Commons.h"

namespace Magnet {

	class Window {
	public:
		Window(int w, int h, std::string name);
		~Window();

		Window(const Window&) = delete;
		Window& operator=(const Window&) = delete;

		bool shouldClose() { return glfwWindowShouldClose(window); }
		VkExtent2D getExtent() { return {static_cast<uint32_t>(width), static_cast<uint32_t>(height)}; }
		bool wasWindowResized() { return frameBufferResized; };
		void resetWindowResizedFlag() { frameBufferResized = false; };
		GLFWwindow* getGLFWWindow() const { return window; }


		void createWindowSurface(VkInstance instance, VkSurfaceKHR* surface);

		int getWidth() { return width; }
		int getHeight() { return height; }

	private:
		static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
		void initWindow();

		int width;
		int height;
		bool frameBufferResized = false;

		std::string windowName;
		GLFWwindow* window;
	};
}