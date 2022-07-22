#pragma once 
#include "SC/Texture.hpp"
#include "SC/MemoryManager.hpp"
#include "SC/Handle.hpp"

#include <memory>

namespace star::core {
	class MapManager : public common::MemoryManager<common::Texture> {
	public:
		MapManager(std::unique_ptr<common::Texture> defaultMap); 

	protected:
		virtual common::Handle createAppropriateHandle() override;

	private:


	};
}