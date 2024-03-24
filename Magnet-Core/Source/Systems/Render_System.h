#pragma once
#include "../Commons.h"
#include "../Engine/Object.h"
#include "../Renderer.h"
#include "../VK/FrameInfo.h"

namespace Magnet {

	struct SimplePushConstantData {
		glm::mat4 modelMatrix{ 1.f };
		glm::mat4 normalMatrix{ 1.f };
	};

	class RenderSystem {
	public:

		RenderSystem(VKBase::Device &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
		~RenderSystem();

		


		void renderObjects(VKBase::FrameInfo &frameInfo);

	private:
		void create_pipelineLayout(VkDescriptorSetLayout globalSetLayout);
		void create_pipeline(VkRenderPass renderPass);

		Magnet::VKBase::Device& device;

		std::unique_ptr<Magnet::VKBase::Pipeline> pipeline;
		VkPipelineLayout pipelineLayout;
	};
}