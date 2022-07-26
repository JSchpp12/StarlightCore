#include "TextureManager.hpp"

namespace star::core {
    common::Handle TextureManager::createAppropriateHandle() {
        common::Handle newHandle; 
        newHandle.type = common::Handle_Type::texture; 
        return newHandle; 
    }
}