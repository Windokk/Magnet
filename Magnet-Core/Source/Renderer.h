#pragma once
#include "Commons.h"

#include "Window.h"

#include "VK/Device.h"
#include "VK/Swapchain.h"


namespace Magnet {

	class Renderer {
	public:

		Renderer(Window& window, VKBase::Device& device);
		~Renderer();

		Renderer(const Renderer&) = delete;
		Renderer& operator=(const Renderer&) = delete;

		


	private:
		void createCommandBuffers();
		void freeCommandBuffers();
		void recreateSwapchain();

		Window& window;
		VKBase::Device& device;
		std::vector<VkCommandBuffer> commandBuffers;

		uint32_t currentImageIndex;
		int currentFrameIndex{ 0 };
		bool isFrameStarted{ false };
	};
}