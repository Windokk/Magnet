#pragma once
#include "Commons.h"
#include "Model.h"

namespace Magnet {

	namespace EngineBase {

		struct TransformComponent {
			glm::vec3 location;
			glm::vec3 scale{ 1.f, 1.f , 1.f};
			glm::vec3 rotation{};

			// Matrix corrsponds to Translate * Ry * Rx * Rz * Scale
			// Rotations correspond to Tait-bryan angles of Y(1), X(2), Z(3)
			glm::mat4 mat4();
			glm::mat3 normalMatrix();

		};

		class Object {
			public:
				using id_t = unsigned int;

				static Object createObject() {
					static id_t currentId = 0;
					return Object(currentId++);
				}


				Object(const Object&) = delete;
				Object& operator=(const Object&) = delete;
				Object(Object&&) = default;
				Object& operator=(Object&&) = default;

				id_t getId() { return id; };

				const id_t id;
				std::shared_ptr<Magnet::VKBase::Model> model{};
				glm::vec3 color{};
				TransformComponent transform{};

			private:

				Object(id_t objId) : id{ objId } {}

			};
	}
}