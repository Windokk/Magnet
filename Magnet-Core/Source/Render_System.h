#pragma once
#include "Commons.h"
#include "Object.h"
#include "Renderer.h"
#include "Camera.h"

namespace Magnet {

	struct SimplePushConstantData {
		glm::mat4 transform{ 1.f };
		glm::mat4 normalMatrix{ 1.f };
	};

	class RenderSystem {
	public:

		RenderSystem(VKBase::Device &device, VkRenderPass renderPass);
		~RenderSystem();

		RenderSystem(const RenderSystem&) = delete;
		RenderSystem& operator=(const RenderSystem&) = delete;


		void renderObjects(VkCommandBuffer commandBuffer, std::vector<Magnet::EngineBase::Object> &objects, const EngineBase::Camera& camera);

	private:
		void create_pipelineLayout();
		void create_pipeline(VkRenderPass renderPass);

		Magnet::VKBase::Device& device;

		std::unique_ptr<Magnet::VKBase::Pipeline> pipeline;
		VkPipelineLayout pipelineLayout;
	};
}