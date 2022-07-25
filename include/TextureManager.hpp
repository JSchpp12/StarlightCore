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

        virtual common::Handle addResource(const std::string& path) { return this->common::FileResourceManager<common::Texture>::addResource(path, std::make_unique<common::Texture>(path)); }

        std::string getPath(const common::Handle& texture) { return this->fileContainer.getPath(texture); };

        std::unique_ptr<unsigned char> load(const std::string& path) {
            int texWidth = 0, texHeight = 0, texChannels = 0;

            //load from disk
            std::unique_ptr<unsigned char> pixelData(stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha));
            if (!pixelData) {
                throw std::runtime_error("Unable to load image");
            }

            return std::move(pixelData);
        }

    protected: 
        virtual common::Handle createAppropriateHandle() override; 
        virtual common::Handle_Type handleType() override { return common::Handle_Type::texture; }
        
    private: 

    }; 
}