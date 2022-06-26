#pragma once

#include "SC/FileResourceManager.hpp"
#include "SC/Handle.hpp"
#include "SC/RenderObject.hpp"
#include "SC/GameObject.hpp"
#include "SC/Vertex.hpp"

#include <tiny_obj_loader.h>
#include <glm/glm.hpp>

#include <string>
#include <vector> 
#include <memory> 

namespace star{
    namespace core{
        //TODO: inherit from manager base
        class ObjectManager : public common::FileResourceManager<common::GameObject> {
        public: 
            class Builder {
            public:
                Builder(ObjectManager* manager) : manager(manager) {}; 
                Builder& setPosition(const glm::vec3 position);
                Builder& setPath(const std::string& path);
                Builder& setScale(const glm::vec3 scale); 
                Builder& setVertShader(const common::Handle& vertShader);
                Builder& setFragShader(const common::Handle& fragShader); 
                Builder& setTexture(const common::Handle& texture); 
                Builder& setVerticies(const std::vector<glm::vec3>& verticies); 
                Builder& setIndicies(const std::vector<uint32_t>& indicies); 
                common::Handle build(); 

            protected:

            private: 
                ObjectManager* manager; 
                glm::vec3 scale = glm::vec3{1.0f, 1.0f, 1.0f};
                glm::vec3 position = glm::vec3{ 0.0f, 0.0f, 0.0f }; 
                common::Handle vertShader = common::Handle{ 0 };
                common::Handle fragShader = common::Handle{ 1 };
                common::Handle texture = common::Handle{ 0 }; 
                std::string path; 
                std::unique_ptr<std::vector<uint32_t>> indicies; 
                std::unique_ptr<std::vector<common::Vertex>> verticies; 
            };

            ~ObjectManager(); 

            common::Handle Add(const std::string& pathToFile); 

            common::Handle Add(const std::string& pathToFile,
                glm::vec3 position, glm::vec3 scaleAmt,
                common::Handle texture, common::Handle vertShader, 
                common::Handle fragShader);

            common::Handle Add(std::unique_ptr<std::vector<common::Vertex>> verticies, std::unique_ptr<std::vector<uint32_t>> indicies,
                glm::vec3 position, glm::vec3 scaleAmt,
                common::Handle texture, common::Handle vertShader,
                common::Handle fragShader);

        protected: 
            //load the object from disk 
            void load(const std::string& pathToFile, std::vector<common::Vertex>* vertexList, std::vector<uint32_t>* indiciesList);

        private: 
            common::GameObject* create(const std::string& pathToFile, glm::vec3 position = glm::vec3{0.0f, 0.0f, 0.0f}, glm::vec3 scaleAmt = glm::vec3{1.0f, 1.0f, 1.0f}, common::Handle texture = common::Handle{0}, common::Handle vertShader = common::Handle{0}, common::Handle fragShader = common::Handle{1});

        };
    }
}