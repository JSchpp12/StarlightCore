#pragma once

#include "SC/FileResourceManager.hpp"
#include "SC/Handle.hpp"
#include "SC/GameObject.hpp"
#include "SC/Vertex.hpp"
#include "SC/Material.hpp"

#include <tiny_obj_loader.h>
#include <glm/glm.hpp>

#include <string>
#include <vector> 
#include <memory> 

namespace star::core{
    //TODO: inherit from manager base
    class ObjectManager : public common::FileResourceManager<common::GameObject> {
    public:  
        virtual common::Handle add(const std::string& pathToFile); 

    protected: 
        //load the object from disk 
        void load(const std::string& pathToFile, std::vector<common::Vertex>* vertexList, std::vector<uint32_t>* indiciesList);

        common::Handle createAppropriateHandle() override; 

    private: 
        //TODO: replace this with the scene builder implementation
        //common::GameObject* create(const std::string& pathToFile, common::Material* material, glm::vec3 position = glm::vec3{ 0.0f, 0.0f, 0.0f }, glm::vec3 scaleAmt = glm::vec3{ 1.0f, 1.0f, 1.0f }, common::Handle texture = common::Handle{ 0 }, common::Handle vertShader = common::Handle{ 0 }, common::Handle fragShader = common::Handle{ 1 });

    };
}
