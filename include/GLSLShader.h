#pragma once

#include "SC/Shader.h"
#include "SC/Enums.h"
#include "Compiler.h"

#include <string> 
#include <iostream> 

namespace star::core{
    class GLSLShader : public common::Shader{
        public:
            static std::unique_ptr<common::Shader> load(const std::string& pathToFile); 

        protected: 
                                
        private: 
    }; 
}