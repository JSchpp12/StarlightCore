#pragma once

#include "SC/FileResourceManager.hpp"
#include "SC/Handle.hpp"
#include "SC/RenderObject.hpp"
#include "SC/Object.hpp"
#include "SC/Vertex.hpp"

#include <tiny_obj_loader.h>

#include <vector> 
#include <memory> 

namespace star{
    namespace core{
        //TODO: inherit from manager base
        class ObjectManager : public common::FileResourceManager<common::Object> {
        public: 
            common::Handle Add(const std::string& pathToFile); 

        protected: 
            
        private: 

        };
    }
}