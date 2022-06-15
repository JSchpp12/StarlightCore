#include "ShaderManager.h"

star::core::ShaderManager::ShaderManager(const std::string& defaultVert, const std::string& defaultFrag)
{
    this->defaultVertShader = this->Add(defaultVert); 
    this->defaultFragShader = this->Add(defaultFrag); 
}

star::core::ShaderManager::~ShaderManager(){
    
}

star::common::Handle star::core::ShaderManager::Add(const std::string& pathToFile){
    //create shader object for new thing 
    common::Shader_File_Type fileType = common::FileHelpers::GetFileType(pathToFile); 

    common::Shader_Stage stage = common::FileHelpers::GetStageOfShader(pathToFile); 

    //check if shader was previously requested
    bool hasBeenLoaded = this->fileContainer.FileLoaded(pathToFile); 
    if(!hasBeenLoaded && (fileType == common::Shader_File_Type::glsl)){
        std::unique_ptr<common::Shader> newShader(GLSLShader::New(pathToFile));
        std::cout << "Completed compilation of: " << pathToFile << std::endl; 
        common::Handle handle = this->fileContainer.AddFileResource(pathToFile, newShader);  
        handle.shaderStage = stage; 
        return handle; 
    }
    else if (hasBeenLoaded) {
        std::cout << "Shader has already been loaded. Returning shared object." << std::endl;
        return this->fileContainer.GetResource(pathToFile); 
    }
    else {
        throw std::runtime_error("This file type is not yet supported");
    }
}

//star::common::Shader* star::core::ShaderManager::Get(const common::Handle& handle) {
//    if (handle.type != common::Handle_Type::defaultHandle) {
//        this->FileResourceManager::Get(handle); 
//    }
//    else {
//        //determine what type of shader to return 
//
//    }
//}