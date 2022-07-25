#include "MaterialManager.hpp"

namespace star::core{
	common::Handle MaterialManager::add(std::unique_ptr<common::Material> newMaterial) {
		common::Handle newHandle; 
		newHandle.type = common::Handle_Type::material; 

		this->addResource(std::move(newMaterial)); 
		return newHandle; 
	}

	common::Handle MaterialManager::add(const glm::vec4& surfaceColor, const glm::vec4& highlightColor, const glm::vec4& ambient,
		const glm::vec4& diffuse, const glm::vec4& specular, const int& shinyCoefficient) {
		common::Handle newHandle; 
		newHandle.type = common::Handle_Type::material; 

		std::unique_ptr<common::Material> newMaterial(new common::Material(surfaceColor, highlightColor, ambient, diffuse, specular, shinyCoefficient)); 
		this->addResource(std::move(newMaterial)); 
		return newHandle; 
	}
	common::Handle MaterialManager::add(const glm::vec4& surfaceColor, const glm::vec4& highlightColor, const glm::vec4& ambient, 
		const glm::vec4& diffuse, const glm::vec4& specular, 
		const int& shinyCoefficient, common::Handle texture, common::Handle bumpMap)
	{
		return this->addResource(std::make_unique<common::Material>(surfaceColor, highlightColor, ambient, diffuse, specular, shinyCoefficient, texture, bumpMap));
	}

	common::Handle MaterialManager::createAppropriateHandle() {
		common::Handle newHandle; 
		newHandle.type = common::Handle_Type::material; 
		return newHandle; 
	}
}