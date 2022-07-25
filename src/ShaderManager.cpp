#include "ShaderManager.h"

namespace star::core {
    ShaderManager::ShaderManager(const std::string& defaultVert, const std::string& defaultFrag)
    {
        this->defaultVertShader = this->addResource(defaultVert);
        this->defaultFragShader = this->addResource(defaultFrag);
    }

    ShaderManager::~ShaderManager() {

    }

    //common::Handle ShaderManager::add(const std::string& pathToFile) {
    //    //create shader object for new thing 
    //    common::Shader_File_Type fileType = common::FileHelpers::GetFileType(pathToFile);

    //    common::Shader_Stage stage = common::FileHelpers::GetStageOfShader(pathToFile);

    //    //check if shader was previously requested
    //    bool hasBeenLoaded = this->fileContainer.contains(pathToFile);
    //    if (!hasBeenLoaded && (fileType == common::Shader_File_Type::glsl)) {
    //        std::unique_ptr<common::Shader> newShader(std::move(GLSLShader::load(pathToFile)));
    //        std::cout << "Completed compilation of: " << pathToFile << std::endl;
    //        return this->addResource(pathToFile, std::move(newShader));
    //    }
    //    else if (hasBeenLoaded) {
    //        std::cout << "Shader has already been loaded. Returning shared object." << std::endl;
    //        return this->fileContainer.getHandle(pathToFile);
    //    }
    //    else {
    //        throw std::runtime_error("This file type is not yet supported");
    //    }
    //}

    common::Handle ShaderManager::createAppropriateHandle() {
        common::Handle newHandle; 
        newHandle.type = common::Handle_Type::shader; 
        return newHandle; 
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