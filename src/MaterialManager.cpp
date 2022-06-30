#include "MaterialManager.hpp"

namespace star {
namespace core {
	MaterialManager::MaterialManager(const glm::vec4& surfaceColor, const glm::vec4& highlightColor, const int& shinyCoefficient) {
		std::unique_ptr<common::Material> defaultMaterial(new common::Material(surfaceColor, highlightColor, shinyCoefficient)); 
		common::Handle defaultHandle;
		defaultHandle.type = common::Handle_Type::material;

		this->addResource(std::move(defaultMaterial), defaultHandle);
		this->defaultMaterial = this->getResource(defaultHandle); 
	}

	common::Handle MaterialManager::add(const glm::vec4& surfaceColor, const glm::vec4& highlightColor, const int& shinyCoefficient) {
		common::Handle newHandle; 
		newHandle.type = common::Handle_Type::material; 

		std::unique_ptr<common::Material> newMaterial(new common::Material(surfaceColor, highlightColor, shinyCoefficient)); 
		this->addResource(std::move(newMaterial), newHandle); 
		return newHandle; 
	}

	common::Material* MaterialManager::get(const common::Handle& handle) {
		return this->getResource(handle); 
	}
}
}