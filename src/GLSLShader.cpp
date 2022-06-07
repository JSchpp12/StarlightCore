#include "GLSLShader.h"

star::core::GLSLShader* star::core::GLSLShader::New(star::common::Pipe_Stage stage, const std::string& pathToFile){
    GLSLShader* newShader; 

    auto compilationResult = std::make_unique<std::vector<uint32_t>>(Compiler::Compile(pathToFile, true));

    newShader = new GLSLShader(std::move(compilationResult)); 

    return newShader; 
}

star::core::GLSLShader::GLSLShader(std::unique_ptr<std::vector<uint32_t>> compiledCode) : Shader(std::move(compiledCode)){}
