#include "MaterialManager.hpp"

namespace star::core{
	MaterialManager::MaterialManager(std::unique_ptr<common::Material> defaultMaterial) {
		std::unique_ptr<common::Material> defaultMaterial;
		defaultMaterial->highlightColor = highlightColor; 
		defaultMaterial->shinyCoefficient = shinyCoefficient; 
		defaultMaterial->ambient = ;
		defaultMaterial->diffuse = ; 
		common::Handle defaultHandle;
		defaultHandle.type = common::Handle_Type::material;

		this->addResource(std::move(defaultMaterial), defaultHandle);
		this->defaultResource = &this->getResource(defaultHandle); 
	}

	common::Handle MaterialManager::add(std::unique_ptr<common::Material> newMaterial) {
		common::Handle newHandle; 
		newHandle.type = common::Handle_Type::material; 

		this->addResource(std::move(newMaterial), newHandle); 
		return newHandle; 
	}

	common::Handle MaterialManager::add(const glm::vec4& surfaceColor, const glm::vec4& highlightColor, const int& shinyCoefficient) {
		common::Handle newHandle; 
		newHandle.type = common::Handle_Type::material; 

		std::unique_ptr<common::Material> newMaterial(new common::Material(surfaceColor, highlightColor, shinyCoefficient)); 
		this->addResource(std::move(newMaterial), newHandle); 
		return newHandle; 
	}

	common::Handle MaterialManager::add(const glm::vec4& surfaceColor, const glm::vec4& highlightColor, const int& shinyCoefficient, common::Handle texture)
	{
		common::Handle newHandle; 
		newHandle.type = common::Handle_Type::material; 

		this->addResource(std::make_unique<common::Material>(surfaceColor, highlightColor, shinyCoefficient, texture), newHandle); 
		return newHandle; 
	}

	common::Material& MaterialManager::get(const common::Handle& handle) {
		return this->getResource(handle); 
	}
}