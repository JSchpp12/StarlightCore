#pragma once

#include "SC/FileHelpers.h"
#include "SC/Enums.h"
#include "SC/ResourceContainer.hpp"
#include "SC/ConfigFile.hpp"
#include "SC/FileResourceManager.hpp"
#include "SC/Shader.h"

#include "Compiler.h"

#include <map> 
#include <string> 
#include <memory>

namespace star::core{
    class ShaderManager : public common::FileResourceManager<common::Shader> {
    public: 
        /// <summary>
        /// Create shader manager with default shaders to be used if objects are not explicitly provided shaders
        /// </summary>
        /// <param name="defaultVert"></param>
        /// <param name="defaultFrag"></param>
        ShaderManager(const std::string& defaultVert, const std::string& defaultFrag);
         
        ~ShaderManager(); 

        common::Shader& resource(const common::Handle& resourceHandle) override; 
    protected: 

        common::Handle createAppropriateHandle() override;
        common::Handle_Type handleType() override { return common::Handle_Type::shader; }


    private: 
        common::Handle defaultVertShader, defaultFragShader; 

    };
}