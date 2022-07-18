#pragma once 
#include "SC/MemoryManager.hpp"
#include "SC/Material.hpp"
#include "SC/Enums.h"

#include <glm/glm.hpp>

#include <memory>

namespace star::core{
	class MaterialManager : public common::MemoryManager<common::Material>{
	public:
		MaterialManager(std::unique_ptr<common::Material> defaultMaterial) : common::MemoryManager<common::Material>() {
			this->common::MemoryManager<common::Material>::init(std::move(defaultMaterial)); 
		}

		common::Handle add(std::unique_ptr<common::Material> newMaterial); 

		common::Handle add(const glm::vec4& surfaceColor, const glm::vec4& highlightColor, const glm::vec4& ambient,
			const glm::vec4& diffuse, const glm::vec4& specular, const int& shinyCoefficient);

		common::Handle add(const glm::vec4& surfaceColor, const glm::vec4& highlightColor, const glm::vec4& ambient, 
			const glm::vec4& diffuse, const glm::vec4& specular, 
			const int& shinyCoefficient, common::Handle texture);

		common::Material& get(const common::Handle& handle); 

		size_t size() { return this->MemoryManager::size(); }

	protected:
		virtual common::Handle createAppropriateHandle() override;
		
	private:

	};
}