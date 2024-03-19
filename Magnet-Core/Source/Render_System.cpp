#include "Render_System.h"

Magnet::RenderSystem::RenderSystem(VKBase::Device& device, VkRenderPass renderPass) : device{device} 
{
	create_pipelineLayout();
	create_pipeline(renderPass);
}

Magnet::RenderSystem::~RenderSystem()
{
	vkDestroyPipelineLayout(device.device(), pipelineLayout, nullptr);
}

void Magnet::RenderSystem::renderObjects(VkCommandBuffer commandBuffer, std::vector<Magnet::EngineBase::Object>& objects, const EngineBase::Camera& camera)
{
    pipeline->bind(commandBuffer);

    auto projectionView = camera.getProjection() * camera.getView();

    for (auto& obj : objects) {
        
        SimplePushConstantData push{};
        auto modelMatrix = obj.transform.mat4();
        push.transform = projectionView * modelMatrix;
        push.normalMatrix = obj.transform.normalMatrix();


        vkCmdPushConstants(
            commandBuffer,
            pipelineLayout,
            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
            0,
            sizeof(SimplePushConstantData),
            &push);
        obj.model->bind(commandBuffer);
        obj.model->draw(commandBuffer);
    }
}

void Magnet::RenderSystem::create_pipelineLayout()
{
    VkPushConstantRange pushConstantRange{};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(SimplePushConstantData);


    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0;
    pipelineLayoutInfo.pSetLayouts = nullptr;
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
    if (vkCreatePipelineLayout(device.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) !=
        VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    }
}

void Magnet::RenderSystem::create_pipeline(VkRenderPass renderPass)
{
    assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

    Magnet::VKBase::PipelineConfigInfo pipelineConfig{};
    Magnet::VKBase::Pipeline::defaultPipelineConfigInfo(pipelineConfig);
    pipelineConfig.renderPass = renderPass;
    pipelineConfig.pipelineLayout = pipelineLayout;
    pipeline = std::make_unique<Magnet::VKBase::Pipeline>(
        device,
        "Shaders/shader.vert.spv",
        "Shaders/shader.frag.spv",
        pipelineConfig);
}
