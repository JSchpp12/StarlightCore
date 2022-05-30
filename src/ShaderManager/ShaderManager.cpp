#include "ShaderManager.h"

star::shadermanager::ShaderManager::ShaderManager(std::shared_ptr<common::ConfigFile> configFile) : common::Manager(configFile){

}

int star::shadermanager::ShaderManager::AddShader(const std::string& pathToFile){
    //create shader object for new thing 
    common::Shader_File_Type fileType = common::FileHelpers::GetFileType(pathToFile); 

    common::Pipe_Stage stage = common::FileHelpers::GetStageOfShader(pathToFile); 

    //check for type of provided file 
    if (fileType == common::Shader_File_Type::glsl){
        std::unique_ptr<GLSLShader> newShader(GLSLShader::New(stage, pathToFile));
        this->shaderContainer.push_back(std::move(newShader)); 
    }else{
        // common::Shader newShader = common::Shader()
        //TODO: support precompiled shaders
        throw std::runtime_error("Precompiled shaders are not yet supported"); 
    }

    //return index of shader
    return this->shaderContainer.size(); 
}