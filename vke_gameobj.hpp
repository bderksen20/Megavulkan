#pragma once

#include "vke_model.hpp"

// std
#include <memory>

// Class for "game objects", anything in a game with properties or etc (example a gooooomba)
// - will include any property that could be used (not good for big complex game)
// - take a look at ECS???
namespace vke {

	struct Transform2dComponent {
		glm::vec2 translation;	// position offset
		glm::vec2 scale{1.f, 1.f};
		float rotation;

		glm::mat2 mat2() {
			const float s = glm::sin(rotation);
			const float c = glm::cos(rotation);
			glm::mat2 rotMat{ {c, s}, {-s, c } };
			glm::mat2 scaleMat{ {scale.x, .0f}, {0.f, scale.y } };
			//return rotMat * scaleMat;
			return rotMat;
		}
	};

	class VkeGameObject {

		public:
			using id_t = unsigned int;	
	
			static VkeGameObject createGameObject() {	// every obj gets a unique ID
				static id_t currentId = 0;
				return VkeGameObject{ currentId++ };
			}

			// Delete copy and assignment operator (dont want to have dupe objs)
			// - but set move and assignment to use default
			VkeGameObject(const VkeGameObject&) = delete;
			VkeGameObject& operator=(const VkeGameObject&) = delete;
			VkeGameObject(VkeGameObject&&) = default;
			VkeGameObject& operator=(VkeGameObject&&) = default;

			id_t const getId() { return id; }

			std::shared_ptr<VkeModel> model{};
			glm::vec3 color{};
			Transform2dComponent transform2d{};

		private:
			VkeGameObject(id_t objId) : id{ objId } {}

			id_t id;
	};
}