#include "Render_System.h"

Magnet::RenderSystem::RenderSystem(VKBase::Device& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout) : device{device}
{
	create_pipelineLayout(globalSetLayout);
	create_pipeline(renderPass);
}

Magnet::RenderSystem::~RenderSystem()
{
	vkDestroyPipelineLayout(device.device(), pipelineLayout, nullptr);
}

void Magnet::RenderSystem::renderObjects(VKBase::FrameInfo& frameInfo, std::vector<Magnet::EngineBase::Object>& objects)
{
    pipeline->bind(frameInfo.commandBuffer);

    vkCmdBindDescriptorSets(
        frameInfo.commandBuffer, 
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        pipelineLayout,
        0,
        1,
        &frameInfo.globalDescriptorSet,
        0,
        nullptr);

    for (auto& obj : objects) {
        
        SimplePushConstantData push{};
        push.modelMatrix = obj.transform.mat4();
        push.normalMatrix = obj.transform.normalMatrix();


        vkCmdPushConstants(
            frameInfo.commandBuffer,
            pipelineLayout,
            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
            0,
            sizeof(SimplePushConstantData),
            &push);
        obj.model->bind(frameInfo.commandBuffer);
        obj.model->draw(frameInfo.commandBuffer);
    }
}

void Magnet::RenderSystem::create_pipelineLayout(VkDescriptorSetLayout globalSetLayout)
{
    VkPushConstantRange pushConstantRange{};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(SimplePushConstantData);

    std::vector<VkDescriptorSetLayout> descriptorSetLayouts{globalSetLayout};

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
    pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
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
        "assets/defaults/shaders/shader.vert.spv",
        "assets/defaults/shaders/shader.frag.spv",
        pipelineConfig);
}
