#pragma once
#include "Commons.h"
#include "Object.h"
#include "Renderer.h"
#include "FrameInfo.h"

namespace Magnet {

	struct SimplePushConstantData {
		glm::mat4 modelMatrix{ 1.f };
		glm::mat4 normalMatrix{ 1.f };
	};

	class RenderSystem {
	public:

		RenderSystem(VKBase::Device &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
		~RenderSystem();

		RenderSystem(const RenderSystem&) = delete;
		RenderSystem& operator=(const RenderSystem&) = delete;


		void renderObjects(VKBase::FrameInfo &frameInfo, std::vector<Magnet::EngineBase::Object> &objects);

	private:
		void create_pipelineLayout(VkDescriptorSetLayout globalSetLayout);
		void create_pipeline(VkRenderPass renderPass);

		Magnet::VKBase::Device& device;

		std::unique_ptr<Magnet::VKBase::Pipeline> pipeline;
		VkPipelineLayout pipelineLayout;
	};
}