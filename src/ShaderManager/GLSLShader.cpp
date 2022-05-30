#include "GLSLShader.h"

star::shadermanager::GLSLShader* star::shadermanager::GLSLShader::New(star::common::Pipe_Stage stage, const std::string& pathToFile){
    GLSLShader* newShader; 

    auto compilationResult = std::make_unique<std::vector<uint32_t>>(Compiler::Compile(pathToFile, true));

    newShader = new GLSLShader(std::move(compilationResult)); 

    return newShader; 
}

star::shadermanager::GLSLShader::GLSLShader(std::unique_ptr<std::vector<uint32_t>> compiledCode) : Shader(std::move(compiledCode)){}

