#include "LightManager.hpp"

namespace star {
namespace core {

	common::Handle LightManager::Add(common::Type::Light lightType, glm::vec3 position) {
		this->lightContainer.push_back(std::make_unique<common::Light>(lightType, position)); 
		return common::Handle{ this->lightContainer.size() - 1};
	}

	common::Handle LightManager::Add(common::Type::Light lightType, glm::vec3 position, glm::vec4 color) {
		this->lightContainer.push_back(std::make_unique<common::Light>(lightType, position, color));
		return common::Handle{ this->lightContainer.size() - 1 };
	}

	common::Light* LightManager::Get(common::Handle lightHandle) {
		return this->lightContainer.at(lightHandle.containerIndex).get();
	}
}
}