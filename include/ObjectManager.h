#pragma once

#include "SC/FileResourceManager.hpp"
#include "SC/Handle.hpp"
#include "SC/RenderObject.hpp"
#include "SC/LogicalObject.hpp"
#include "SC/Vertex.hpp"

#include <tiny_obj_loader.h>
#include <glm/glm.hpp>

#include <vector> 
#include <memory> 

namespace star{
    namespace core{
        //TODO: inherit from manager base
        class ObjectManager : public common::FileResourceManager<common::LogicalObject> {
        public: 
            ~ObjectManager(); 

            common::Handle Add(const std::string& pathToFile); 

            common::Handle Add(const std::string& pathToFile,
                common::Handle texture = common::Handle{0}, common::Handle vertShader = common::Handle{0}, 
                common::Handle fragShader = common::Handle{1},
                glm::vec3 scaleAmt = glm::vec3{ 1.0f, 1.0f, 1.0f });

        protected: 
            //load the object from disk 
            void load(const std::string& pathToFile, std::vector<common::Vertex>* vertexList, std::vector<uint32_t>* indiciesList);

        private: 
            common::LogicalObject* create(const std::string& pathToFile, glm::vec3 scaleAmt = glm::vec3{1.0f, 1.0f, 1.0f}, common::Handle texture = common::Handle{0}, common::Handle vertShader = common::Handle{0}, common::Handle fragShader = common::Handle{1});

        };
    }
}