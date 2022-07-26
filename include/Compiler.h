/*
* This class contains the implementation of a online shader compiler. 
*/

#pragma once

#include "shaderc/shaderc.hpp"
#include "SC/FileHelpers.h"

#include <memory>
#include <vector> 
#include <string>
#include <iostream>
#include <exception>

namespace star::core{
    class Compiler{
    public:   
        //compile provided shader to spirv
        static std::unique_ptr<std::vector<uint32_t>> compile(const std::string& pathToFile, bool optimize); 

    private:
        //get the shaderc stage flag for the shader stage
        static shaderc_shader_kind getShaderCStageFlag(const std::string& pathToFile); 

        //preprocess shader code before compilation 
        static std::string preprocessShader(const std::string& sourceName, shaderc_shader_kind stage, const std::string& source); 
    }; 
}