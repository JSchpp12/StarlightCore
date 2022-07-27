#include "Star_Shader.h"

namespace star::core {
    Shader::Shader(const common::Shader& shader) : compiledCode(load(shader.path)), common::Shader(shader) { 
        //generateDescriptorList(); 
    }

    //void Shader::generateDescriptorList() {
    //    SpvReflectShaderModule shaderModule; 
    //    SpvReflectResult result = spvReflectCreateShaderModule(4 * compiledCode->size(), compiledCode->data(), &shaderModule); 

    //    assert(result == SPV_REFLECT_RESULT_SUCCESS); 
    //}

    std::unique_ptr<std::vector<uint32_t>> Shader::load(const std::string& pathToFile) {
        return Compiler::compile(pathToFile, true);
    }
}