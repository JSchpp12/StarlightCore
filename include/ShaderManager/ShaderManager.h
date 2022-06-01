#pragma once

#include "SC/FileHelpers.h"
#include "SC/Enums.h"
#include "SC/ResourceContainer.hpp"
#include "SC/ConfigFile.h"
#include "SC/FileResourceManager.hpp"

#include "GLSLShader.h"
#include "Compiler.h"

#include <map> 
#include <string> 
#include <memory>

namespace star{
    namespace core{
        class ShaderManager : public common::FileResourceManager<common::Shader> {
        public: 
            ShaderManager(); 

            ///add shader to manager, will return handle to compiled resource
            virtual common::Handle Add(const std::string& pathToFile); 
        protected: 

        private: 
            
        };
    }
}
