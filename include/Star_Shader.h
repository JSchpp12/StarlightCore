/*
* This class is a wrapper for shader operations needed to prepare common::Shader object for use. Such as compilation and cleaning.
*/
#pragma once

#include "SC/Shader.h"
#include "SC/Enums.h"
#include "Compiler.h"

#include <string> 
#include <iostream> 
#include <vector>

namespace star::core{
    class Shader : private common::Shader{
        public:
            Shader(const common::Shader& shader); 

            std::unique_ptr<std::vector<uint32_t>> compiledCode; 
        protected:

        private: 
            static std::unique_ptr<std::vector<uint32_t>> load(const std::string& pathToFile);

    }; 
}