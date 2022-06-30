#pragma once
#include "SC/Light.hpp"
#include "SC/Enums.h"
#include "SC/Handle.hpp"

#include <glm/glm.hpp>

#include <vector>
#include <memory>

namespace star {
namespace core {
	class LightManager {
	public:
		//class Builder {
		//public:
		//	Builder& setPosition(glm::vec3 position); 
		//	Budiler& 
		//private: 

		//};
		
		common::Handle Add(common::Type::Light lightType, glm::vec3 position);

		common::Handle Add(common::Type::Light lightType, glm::vec3 position, glm::vec4 color); 
		
		common::Light* Get(common::Handle lightHandle); 

	private: 
		std::vector<std::unique_ptr<common::Light>> lightContainer; 

	};
}
}