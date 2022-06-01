#pragma once

#include "SC/FileResourceManager.hpp"
#include "SC/Handle.hpp"
#include "SC/RenderObject.hpp"

#include "Vertex.hpp"
#include "Object.h"

#include <vector> 
#include <memory> 

namespace star{
    namespace core{
        //TODO: inherit from manager base
        class ObjectManager : public common::FileResourceManager<Object> {
        public: 
            common::Handle Add(const std::string& pathToFile); 

        protected: 
            
        private: 

        };
    }
}