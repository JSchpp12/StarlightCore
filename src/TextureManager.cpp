#include "TextureManager.h"

 #define STB_IMAGE_IMPLEMENTATION
 #include <stb_image.h>

namespace star::core {
    TextureManager::TextureManager(const std::string& path) {
        int texWidth = 0; 
        int texHeight = 0; 
        int texChannels = 0; 

        std::unique_ptr<unsigned char> imageData(loadImage(path, texWidth, texHeight, texChannels)); 

        std::unique_ptr<common::Texture> newImage(new common::Texture(path, std::move(imageData), texWidth, texHeight, texChannels)); 
        common::Handle newHandle = this->addResource(path, std::move(newImage)); 
        this->defaultResource = &this->getResource(newHandle); 
    }

    TextureManager::~TextureManager() {
        //TODO: might have memory leak here...
        //for (size_t i = 0; i < this->fileContainer.container.size(); i++) {
        //    stbi_image_free(this->fileContainer.container.at(i)->data()); 
        //}
    }

    common::Handle star::core::TextureManager::add(const std::string& pathToFile) {
        //TODO -- READ DATA FROM FILE: manual entry for now 
        int texWidth;
        int texHeight;
        int texChannels;

        bool fileLoaded = this->fileContainer.fileLoaded(pathToFile);

        if (!fileLoaded) {
            std::unique_ptr<unsigned char> imageData(loadImage(pathToFile, texWidth, texHeight, texChannels));

            std::unique_ptr<common::Texture> newImage(new common::Texture(pathToFile, std::move(imageData), texWidth, texHeight, texChannels));

            common::Handle newHandle = this->addResource(pathToFile, std::move(newImage));

            return newHandle;
        }
        else {
            throw std::runtime_error("Previously loaded files are not yet permitted.");
        }
    }

    common::Handle TextureManager::createAppropriateHandle() {
        common::Handle newHandle; 
        newHandle.type = common::Handle_Type::texture; 
        return newHandle; 
    }

    unsigned char* star::core::TextureManager::loadImage(const std::string& path, int& texWidth, int& texHeight, int& texChannel) {
        stbi_uc* pixels = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannel, STBI_rgb_alpha);
        if (!pixels) {
            throw std::runtime_error("Unable to load image");
        }


        return pixels;
    }

}