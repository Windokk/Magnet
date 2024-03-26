#include "Renderer.h"

Magnet::Renderer::Renderer(Window& window, VKBase::Device& device) : window{ window }, device{ device }
{
	recreateSwapchain();
	createCommandBuffers();
}


Magnet::Renderer::~Renderer()
{
	freeCommandBuffers();
}

void Magnet::Renderer::createCommandBuffers()
{
    commandBuffers.resize(VKBase::SwapChain::MAX_FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = device.getCommandPool();
    allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

    if (vkAllocateCommandBuffers(device.device(), &allocInfo, commandBuffers.data()) !=
        VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers!");
    }
}