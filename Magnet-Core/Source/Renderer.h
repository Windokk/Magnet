#pragma once
#include "Window.h"
#include "VK/Pipeline.h"
#include "VK/Swapchain.h"

namespace Magnet {

	class Renderer {
	public:

		Renderer(Window &window, VKBase::Device &device);
		~Renderer();

		Renderer(const Renderer&) = delete;
		Renderer& operator=(const Renderer&) = delete;

		VkRenderPass getSwapChainRenderpass() const { 
			return swapchain->getRenderPass();
		}
		float getAspectRatio() const { return swapchain->extentAspectRatio(); }
		bool isFrameInProgress() const { return isFrameStarted; }

		VkCommandBuffer getCurrentCommandBuffer() const {
			assert(isFrameStarted && "Cannot get command buffer when frame not in progress");
			return commandBuffers[currentFrameIndex];
		}

		int getFrameIndex() const {
			assert(isFrameStarted && "Cannot get frame index when frame not in progress");
			return currentFrameIndex;
		}

		VkCommandBuffer beginFrame();
		void endFrame();

		void beginSwapchainRenderpass(VkCommandBuffer commandBuffer);
		void endSwapchainRenderpass(VkCommandBuffer commandBuffer);

	private:
		void createCommandBuffers();
		void freeCommandBuffers();
		void recreateSwapchain();

		Window& window;
		VKBase::Device& device;
		std::unique_ptr < VKBase::SwapChain> swapchain;
		std::vector<VkCommandBuffer> commandBuffers;

		uint32_t currentImageIndex;
		int currentFrameIndex{0};
		bool isFrameStarted{ false };
	};
}
