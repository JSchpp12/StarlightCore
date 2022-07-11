#include "ShaderManager.h"

namespace star::core {
    ShaderManager::ShaderManager(const std::string& defaultVert, const std::string& defaultFrag)
    {
        this->defaultVertShader = this->add(defaultVert);
        this->defaultFragShader = this->add(defaultFrag);
    }

    ShaderManager::~ShaderManager() {

    }

    common::Handle ShaderManager::add(const std::string& pathToFile) {
        //create shader object for new thing 
        common::Shader_File_Type fileType = common::FileHelpers::GetFileType(pathToFile);

        common::Shader_Stage stage = common::FileHelpers::GetStageOfShader(pathToFile);

        //check if shader was previously requested
        bool hasBeenLoaded = this->fileContainer.fileLoaded(pathToFile);
        if (!hasBeenLoaded && (fileType == common::Shader_File_Type::glsl)) {
            std::unique_ptr<common::Shader> newShader(GLSLShader::New(pathToFile));
            std::cout << "Completed compilation of: " << pathToFile << std::endl;
            common::Handle newHandle;
            newHandle.type = common::Handle_Type::shader;
            newHandle.shaderStage = stage;
            this->addResource(pathToFile, std::move(newShader), newHandle);
            return newHandle;
        }
        else if (hasBeenLoaded) {
            std::cout << "Shader has already been loaded. Returning shared object." << std::endl;
            return this->fileContainer.get(pathToFile);
        }
        else {
            throw std::runtime_error("This file type is not yet supported");
        }
    }

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