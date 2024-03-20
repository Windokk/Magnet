#include "App.h"
#include "Render_System.h"
#include "KeyboardMovementController.h"
#include "Model.h"

Magnet::AppBase::AppBase()
{
    loadObjects();
}

Magnet::AppBase::~AppBase()
{
}

void Magnet::AppBase::run() {

    VKBase::Buffer globalUBOBuffer{
        device,
        sizeof(GlobalUBO),
        VKBase::SwapChain::MAX_FRAMES_IN_FLIGHT,
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
        device.properties.limits.minUniformBufferOffsetAlignment
    };
    globalUBOBuffer.map();

    RenderSystem renderSystem{ device, renderer.getSwapChainRenderpass() };
    Magnet::EngineBase::Camera camera{};


    auto viewerObject = EngineBase::Object::createObject();
    EngineBase::KeyboardMovementController cameraController{};

    auto currentTime = std::chrono::high_resolution_clock::now();

    while (!window.shouldClose()) {
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
                camera
            };

            //update 
            GlobalUBO ubo;
            ubo.projectionView = camera.getProjection() * camera.getView();
            globalUBOBuffer.writeToIndex(&ubo, frameIndex);
            globalUBOBuffer.flushIndex(frameIndex);

            //render
            renderer.beginSwapchainRenderpass(commandBuffer);
            renderSystem.renderObjects(frameInfo, objects);
            renderer.endSwapchainRenderpass(commandBuffer);
            renderer.endFrame();
        }

    }
    vkDeviceWaitIdle(device.device());
}

void Magnet::AppBase::loadObjects()
{
    std::shared_ptr<Magnet::VKBase::Model> model = Magnet::VKBase::Model::createModelFromFile(device, "assets/defaults/models/obj/Sphere_Smooth/Sphere_Smooth.obj");

    auto Obj = Magnet::EngineBase::Object::createObject();
    Obj.model = model;
    Obj.transform.location = { .0f, .0f, 2.5f };
    Obj.transform.scale = { .5f, .5f, .5f };
    objects.push_back(std::move(Obj));
}


