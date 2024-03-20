#pragma once
#include "Object.h"
#include "Renderer.h"
#include "Descriptors.h"

namespace Magnet {

	struct GlobalUBO {
		alignas(16) glm::mat4 projectionView{ 1.f };
		alignas(16) glm::vec3 lightDirection = glm::normalize(glm::vec3{ 1.f, -3.f, -1.f });
	};

	class AppBase {
	public:
		static constexpr int WIDTH = 800;
		static constexpr int HEIGHT = 600;
		AppBase();
		~AppBase();

		AppBase(const AppBase&) = delete;
		AppBase& operator=(const AppBase&) = delete;

		void run();

	private:
		void loadObjects();

		Window window{ WIDTH, HEIGHT, "Magnet" };
		Magnet::VKBase::Device device{ window };
		Renderer renderer{window, device};

		// NOTE : Order of declaration matters ! 

		std::unique_ptr<VKBase::DescriptorPool> globalPool{};
		std::vector<Magnet::EngineBase::Object> objects;
	};
}
