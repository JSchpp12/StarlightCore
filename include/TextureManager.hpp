#pragma once 
#include "SC/FileResourceManager.hpp"
#include "SC/Texture.hpp"
#include "SC/Handle.hpp"

#include <string> 
#include <memory> 

namespace star::core{
    class TextureManager : private common::FileResourceManager<common::Texture> {
    public: 
        TextureManager(const std::string& path) { this->addResource(path); }

        virtual ~TextureManager() { };

        virtual common::Handle addResource(const std::string& path) { return this->FileResourceManager<common::Texture>::addResource(path, std::make_unique<common::Texture>(path)); }
        virtual common::Texture& resource(const common::Handle& resourceHandle) override { return this->FileResourceManager<common::Texture>::resource(resourceHandle); }

    protected: 
        virtual common::Handle createAppropriateHandle() override; 
        virtual common::Handle_Type handleType() override { return common::Handle_Type::texture; }

    private: 

    }; 
}