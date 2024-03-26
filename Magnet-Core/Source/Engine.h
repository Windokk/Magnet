#pragma once
#include "Engine/Object.h"
#include "VK/Descriptors.h"
#include "Engine/Camera.h"
#include "Engine/Object.h"
#include "VK/Swapchain.h"
#include "Renderer.h"

namespace Magnet {

	struct GlobalUbo {
		glm::mat4 projection{ 1.f };
		glm::mat4 view{ 1.f };
		glm::vec4 ambientLightColor{ 1.f, 1.f, 1.f, .02f };  // w is intensity
		glm::vec3 lightPosition{ -1.f };
		alignas(16) glm::vec4 lightColor{ 1.f };  // w is light intensity
	};

	

	class Engine {
	public:
		static constexpr int WIDTH = 800;
		static constexpr int HEIGHT = 600;
		Engine();
		~Engine();

		Engine(const Engine&) = delete;
		Engine& operator=(const Engine&) = delete;

		void init();
		void run();
		void waitIdle();

		bool shouldClose();

		

	private:

		Window window{ WIDTH, HEIGHT, "Magnet" };
		Magnet::VKBase::Device device{window};

		Magnet::EngineBase::Camera camera{};

		Magnet::VKBase::SwapChain swapchain{device, window.getExtent()};

		Renderer renderer{ window, device };

		EngineBase::Object::Map objects;

	};
}
