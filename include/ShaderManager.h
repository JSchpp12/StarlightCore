#pragma once

#include "SC/FileHelpers.h"
#include "SC/Enums.h"
#include "SC/ResourceContainer.hpp"
#include "SC/ConfigFile.hpp"
#include "SC/FileResourceManager.hpp"

#include "GLSLShader.h"
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

        ///add shader to manager, will return handle to compiled resource
        virtual common::Handle add(const std::string& pathToFile); 

        //common::Shader* Get(const common::Handle& handle) override; 
    protected: 
        common::Handle createAppropriateHandle(); 

    private: 
        common::Handle defaultVertShader, defaultFragShader; 

    };
}