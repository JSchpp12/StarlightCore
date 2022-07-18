#include "LightManager.hpp"

namespace star::core{
	common::Handle LightManager::createAppropriateHandle() {
		common::Handle newHandle; 
		newHandle.type = common::Handle_Type::light; 
		return newHandle; 
	}
}