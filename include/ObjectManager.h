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
            ~ObjectManager(); 

            common::Handle Add(const std::string& pathToFile); 

            common::Handle Add(const std::string& pathToFile, common::Handle texture = common::Handle{ 0 }, common::Handle vertShader = common::Handle{ 0 }, common::Handle fragShader = common::Handle{ 1 });

        protected: 
            //load the object from disk 
            void load(const std::string& pathToFile, std::vector<common::Vertex>* vertexList, std::vector<uint32_t>* indiciesList);

        private: 
            common::Object* create(const std::string& pathToFile, common::Handle texture = common::Handle{ 0 }, common::Handle vertShader = common::Handle{ 0 }, common::Handle fragShader = common::Handle{ 1 });

        };
    }
}