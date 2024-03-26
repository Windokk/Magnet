#include "Engine.h"



Magnet::Engine::Engine()
{
    init();
}

Magnet::Engine::~Engine()
{
    vkDestroyPipeline(device.device(), pipelines.solid, nullptr);
    if (pipelines.wireframe != VK_NULL_HANDLE) {
        vkDestroyPipeline(device.device(), pipelines.wireframe, nullptr);
    }
    vkDestroyPipelineLayout(device.device(), pipelineLayout, nullptr);
    vkDestroyDescriptorSetLayout(device.device(), descriptorSetLayouts.matrices, nullptr);
    vkDestroyDescriptorSetLayout(device.device(), descriptorSetLayouts.textures, nullptr);
    shaderData.buffer.destroy();
}

void Magnet::Engine::init()
{

    camera.type = EngineBase::Camera::CameraType::lookat;
    camera.flipY = true;
    camera.setPosition(glm::vec3(0.0f, -0.1f, -1.0f));
    camera.setRotation(glm::vec3(0.0f, 45.0f, 0.0f));
    camera.setPerspective(60.0f, (float)WIDTH / (float)HEIGHT, 0.1f, 256.0f);

    //loadObjects();

    createPipelineCache();
    setupFrameBuffer();
    loadAssets();
    prepareUniformBuffers();
    setupDescriptors();
    preparePipelines();
    buildCommandBuffers();
    
}

void Magnet::Engine::run() {

    glfwPollEvents();
}

void Magnet::Engine::waitIdle()
{
    vkDeviceWaitIdle(device.device());
    glfwDestroyWindow(window.getGLFWWindow());
    glfwTerminate();
}

bool Magnet::Engine::shouldClose()
{
    return window.shouldClose();
}


