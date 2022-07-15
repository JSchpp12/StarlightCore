#pragma once

#include "SC/Shader.h"
#include "SC/Enums.h"
#include "Compiler.h"

#include <string> 
#include <iostream> 

namespace star::core{
    class GLSLShader : public common::Shader{
        public:
            static GLSLShader* New(const std::string& pathToFile); 
        protected: 
                                
        private: 
            //create a GLSL shader and compile provided file to spirv
            GLSLShader(std::unique_ptr<std::vector<uint32_t>> compiledCode); 
    }; 
}