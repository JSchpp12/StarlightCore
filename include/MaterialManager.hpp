#pragma once 
#include "SC/MemoryManager.hpp"
#include "SC/Material.hpp"
#include "SC/Enums.h"

#include <glm/glm.hpp>

#include <memory>

namespace star::core{
	class MaterialManager : public common::MemoryManager<common::Material>{
	public:
		MaterialManager(const glm::vec4& surfaceColor, const glm::vec4& highlightColor, const int& shinyCoefficient); 

		common::Handle add(std::unique_ptr<common::Material> newMaterial); 

		common::Handle add(const glm::vec4& surfaceColor, const glm::vec4& highlightColor, const int& shinyCoefficient);

		common::Handle add(const glm::vec4& surfaceColor, const glm::vec4& highlightColor, const int& shinyCoefficient, common::Handle texture);

		common::Material& get(const common::Handle& handle); 

		size_t size() { return this->MemoryManager::size(); }

	protected:

	private:

	};
}