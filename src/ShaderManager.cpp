#include "ShaderManager.h"

namespace star::core {
    ShaderManager::ShaderManager(const std::string& defaultVert, const std::string& defaultFrag) : 
        defaultVertShader(this->addResource(defaultVert, std::make_unique<common::Shader>(defaultVert))),
        defaultFragShader(this->addResource(defaultFrag, std::make_unique<common::Shader>(defaultFrag)))
    { }

    ShaderManager::~ShaderManager() { }

    common::Shader& ShaderManager::resource(const common::Handle& resourceHandle) {
        if (resourceHandle.type == common::Handle_Type::defaultHandle) {
            if (resourceHandle.shaderStage.has_value() && resourceHandle.shaderStage.value() == common::Shader_Stage::vertex) {
                return this->FileResourceManager<common::Shader>::resource(this->defaultVertShader);
            }
            else if (resourceHandle.shaderStage.has_value() && resourceHandle.shaderStage.value() == common::Shader_Stage::fragment) {
                return this->FileResourceManager<common::Shader>::resource(this->defaultFragShader);
            }
            else {
                throw std::runtime_error("Unexpected default shader requested");
            }
        }
        else {
            return this->FileResourceManager<common::Shader>::resource(resourceHandle);
        }
    }

    common::Handle ShaderManager::createAppropriateHandle() {
        common::Handle newHandle; 
        newHandle.type = common::Handle_Type::shader; 
        return newHandle; 
    }
}
