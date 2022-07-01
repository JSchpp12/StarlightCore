#pragma once 
#include "SC/MemoryManager.hpp"
#include "SC/Material.hpp"
#include "SC/Enums.h"

#include <glm/glm.hpp>

namespace star {
namespace core {
	class MaterialManager : public common::MemoryManager<common::Material>{
	public:
		MaterialManager(const glm::vec4& surfaceColor, const glm::vec4& highlightColor, const int& shinyCoefficient); 

		common::Handle add(const glm::vec4& surfaceColor, const glm::vec4& highlightColor, const int& shinyCoefficient);

		common::Material* get(const common::Handle& handle); 

		common::Material* getDefault() { 
			if (this->defaultMaterial != nullptr)
				return this->defaultMaterial; 
			throw std::runtime_error("Default material is null");
		}

	protected:
		common::Material* defaultMaterial = nullptr; 

	private:


	};
}
}
