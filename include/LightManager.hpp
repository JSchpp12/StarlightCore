#pragma once
#include "SC/Light.hpp"
#include "SC/Enums.h"
#include "SC/Handle.hpp"
#include "SC/MemoryManager.hpp"

#include <glm/glm.hpp>

#include <vector>
#include <memory>

namespace star::core{
	class LightManager : public common::MemoryManager<common::Light> {
	public:
		common::Handle add(std::unique_ptr<common::Light> newLight);

	protected:
		virtual common::Handle createAppropriateHandle() override;

	private: 
		std::vector<std::unique_ptr<common::Light>> lightContainer; 

	};
}