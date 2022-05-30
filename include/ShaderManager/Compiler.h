#pragma once

#include "shaderc/shaderc.hpp"
#include "SC/FileHelpers.h"

#include <vector> 
#include <string>
#include <iostream>
#include <exception>

namespace star{
    namespace shadermanager{
        class Compiler{
        public:   
            //compile provided shader to spirv
            static std::vector<uint32_t> Compile(const std::string& pathToFile, bool optimize); 

        private:
            //get the shaderc stage flag for the shader stage
            static shaderc_shader_kind GetShaderCStageFlag(const std::string& pathToFile); 

        }; 
    }
}