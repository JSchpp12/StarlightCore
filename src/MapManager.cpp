#include "MapManager.hpp"

namespace star::core {
	MapManager::MapManager(std::unique_ptr<common::Texture> defaultMap) : common::MemoryManager<common::Texture>() { 
		this->common::MemoryManager<common::Texture>::init(std::move(defaultMap));
	}

	common::Handle MapManager::createAppropriateHandle() {
		common::Handle newHandle; 
		newHandle.type = common::Handle_Type::map; 
		return newHandle; 
	}
}