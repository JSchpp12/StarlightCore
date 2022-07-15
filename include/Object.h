#pragma once 

#include "SC/Vertex.hpp"
#include "SC/Handle.hpp"

#include <tiny_obj_loader.h>
#include <glm/glm.hpp>

#include <vector> 
#include <memory> 

namespace star::core{
    class Object {
    public: 
        // static Entity& New(const std::string& filePath, common::Handle vertShader, common::Handle fragShader); 
        // static Object& New(const std::string& filePath); 

        Object(std::unique_ptr<std::vector<common::Vertex>> vertexList, std::unique_ptr<std::vector<uint16_t>> indiciesList); 
    protected: 
        // virtual void Load(const std::string& filePath); 


    private: 
        glm::mat4 modelMatrix; 
        std::unique_ptr<std::vector<common::Vertex>> vertexList; 
        std::unique_ptr<std::vector<uint16_t>> indiciesList; 

    };
}