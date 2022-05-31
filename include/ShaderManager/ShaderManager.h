#pragma once

#include "SC/FileHelpers.h"
#include "SC/Enums.h"
#include "SC/Manager.h"
#include "SC/ConfigFile.h"

#include "GLSLShader.h"
#include "Compiler.h"

#include <map> 
#include <string> 
#include <memory>

namespace star{
    namespace shadermanager{
        class ShaderManager : public common::Manager {
        public: 
            ShaderManager(std::shared_ptr<common::ConfigFile> configFile); 

            ///add shader to manager, will return handle to compiled resource
            uint8_t AddShader(const std::string& pathToFile); 
        protected: 

        private:
            //common::ResourceHolder<common::Shader, std::string> holder; 
            std::vector<std::unique_ptr<common::Shader>> shaderContainer; 
        };
    }
}
