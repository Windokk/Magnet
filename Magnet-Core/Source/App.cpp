#include "App.h"



Magnet::AppBase::AppBase()
{
    globalPool = VKBase::DescriptorPool::Builder(device)
        .setMaxSets(VKBase::SwapChain::MAX_FRAMES_IN_FLIGHT)
        .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VKBase::SwapChain::MAX_FRAMES_IN_FLIGHT)
        .build();

    loadObjects();

    
}

Magnet::AppBase::~AppBase()
{
    delete renderSystem;
}

void Magnet::AppBase::init()
{
    
    uboBuffers.resize(VKBase::SwapChain::MAX_FRAMES_IN_FLIGHT);
    for (int i = 0; i < uboBuffers.size(); i++) {
        uboBuffers[i] = std::make_unique<VKBase::Buffer>(
            device,
            sizeof(GlobalUbo),
            1,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
        uboBuffers[i]->map();
    }

    globalSetLayout = VKBase::DescriptorSetLayout::Builder(device)
        .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
        .build();

    globalDescriptorSets.resize(VKBase::SwapChain::MAX_FRAMES_IN_FLIGHT);
    for (int i = 0; i < globalDescriptorSets.size(); i++) {
        auto bufferInfo = uboBuffers[i]->descriptorInfo();
        VKBase::DescriptorWriter(*globalSetLayout, *globalPool)
            .writeBuffer(0, &bufferInfo)
            .build(globalDescriptorSets[i]);
    }

    currentTime = std::chrono::high_resolution_clock::now();

    renderSystem = new RenderSystem(device, renderer.getSwapChainRenderpass(), globalSetLayout->getDescriptorSetLayout());
    
}

void Magnet::AppBase::run() {

    glfwPollEvents();

    
    auto newTime = std::chrono::high_resolution_clock::now();
    float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
    currentTime = newTime;


    cameraController.moveInPlaneXZ(window.getGLFWWindow(), frameTime, viewerObject);
    camera.setViewXYZ(viewerObject.transform.location, viewerObject.transform.rotation);

    float aspect = renderer.getAspectRatio();
    //camera.setOrthographicProjection(-aspect, aspect, -1, 1, -1, 1);
    camera.setPerspectiveProjection(glm::radians(45.f), aspect, 0.1f, 10.f);

    if (auto commandBuffer = renderer.beginFrame()) {
        int frameIndex = renderer.getFrameIndex();

        VKBase::FrameInfo frameInfo{
                frameIndex,
                frameTime,
                commandBuffer,
                camera,
                globalDescriptorSets[frameIndex],
                objects
        };

        //update 
        GlobalUbo ubo;
        ubo.projection = camera.getProjection();
        ubo.view = camera.getView();
        uboBuffers[frameIndex]->writeToBuffer(&ubo);
        uboBuffers[frameIndex]->flush();

        //render
        renderer.beginSwapchainRenderpass(commandBuffer);
        renderSystem->renderObjects(frameInfo);
        renderer.endSwapchainRenderpass(commandBuffer);
        renderer.endFrame();
    }
    
    vkDeviceWaitIdle(device.device());
}

void Magnet::AppBase::waitIdle()
{
    vkDeviceWaitIdle(device.device());
    glfwDestroyWindow(window.getGLFWWindow());
    glfwTerminate();
}

bool Magnet::AppBase::shouldClose()
{
    return window.shouldClose();
}

void Magnet::AppBase::loadObjects()
{
    std::shared_ptr<Magnet::VKBase::Model> model_plane = Magnet::VKBase::Model::createModelFromFile(device, "assets/defaults/models/obj/Plane/Plane.obj");

    auto floor = EngineBase::Object::createObject();
    floor.model = model_plane;
    floor.transform.location = { 0.f, .5f, 0.f };
    floor.transform.scale = { 3.f, 1.f, 3.f };
    objects.emplace(floor.getId(), std::move(floor));



    std::shared_ptr<Magnet::VKBase::Model> model = Magnet::VKBase::Model::createModelFromFile(device, "assets/defaults/models/obj/Sphere_Smooth/Sphere_Smooth.obj");

    auto Sphere = Magnet::EngineBase::Object::createObject();
    Sphere.model = model;
    Sphere.transform.location = { .0f, .0f, 2.5f };
    Sphere.transform.scale = { .5f, .5f, .5f };
    objects.emplace(Sphere.getId(), std::move(Sphere));

    auto Sphere2 = Magnet::EngineBase::Object::createObject();
    Sphere2.model = model;
    Sphere2.transform.location = { .0f, .0f, 1.f };
    Sphere2.transform.scale = { .5f, .5f, .5f };
    objects.emplace(Sphere2.getId(), std::move(Sphere2));

}

