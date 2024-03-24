#pragma once
#include "Engine/Object.h"
#include "Renderer.h"
#include "VK/Descriptors.h"
#include "Systems/Render_System.h"
#include "Engine/KeyboardMovementController.h"
#include "VK/Model.h"

namespace Magnet {

	struct GlobalUbo {
		glm::mat4 projection{ 1.f };
		glm::mat4 view{ 1.f };
		glm::vec4 ambientLightColor{ 1.f, 1.f, 1.f, .02f };  // w is intensity
		glm::vec3 lightPosition{ -1.f };
		alignas(16) glm::vec4 lightColor{ 1.f };  // w is light intensity
	};

	

	class AppBase {
	public:
		static constexpr int WIDTH = 800;
		static constexpr int HEIGHT = 600;
		AppBase();
		~AppBase();

		AppBase(const AppBase&) = delete;
		AppBase& operator=(const AppBase&) = delete;

		void init();
		void run();
		void waitIdle();

		bool shouldClose();

		

	private:
		void loadObjects();

		Window window{ WIDTH, HEIGHT, "Magnet" };
		Magnet::VKBase::Device device{window};
		Renderer renderer{ window, device };


		std::unique_ptr<VKBase::DescriptorSetLayout> globalSetLayout;
		RenderSystem* renderSystem;
		std::vector<std::unique_ptr<VKBase::Buffer>> uboBuffers;
		Magnet::EngineBase::Camera camera{};
		Magnet::EngineBase::KeyboardMovementController cameraController{};
		Magnet::EngineBase::Object viewerObject = EngineBase::Object::createObject();
		std::chrono::steady_clock::time_point currentTime;
		std::vector<VkDescriptorSet> globalDescriptorSets;

		// NOTE : Order of declaration matters ! 
		std::unique_ptr<VKBase::DescriptorPool> globalPool{};
		EngineBase::Object::Map objects;

	};
}
