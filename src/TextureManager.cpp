#include "TextureManager.h"

 #define STB_IMAGE_IMPLEMENTATION
 #include <stb_image.h>

star::core::TextureManager::~TextureManager(){
    //TODO: might have memory leak here...
    //for (size_t i = 0; i < this->fileContainer.container.size(); i++) {
    //    stbi_image_free(this->fileContainer.container.at(i)->data()); 
    //}
}

star::common::Handle star::core::TextureManager::add(const std::string& pathToFile){
    //TODO -- READ DATA FROM FILE: manual entry for now 
    int texWidth; 
    int texHeight; 
    int texChannels; 
    
    bool fileLoaded = this->fileContainer.fileLoaded(pathToFile); 

    if (!fileLoaded){
        std::unique_ptr<unsigned char> imageData(loadImage(pathToFile, texWidth, texHeight, texChannels)); 

        std::unique_ptr<common::Texture> newImage(new common::Texture(pathToFile, std::move(imageData), texWidth, texHeight, texChannels));
        
        common::Handle newHandle; 
        newHandle.type = common::Handle_Type::texture; 

        this->addResource(pathToFile, std::move(newImage), newHandle);
        return newHandle; 
    }else{
        throw std::runtime_error("Previously loaded files are not yet permitted."); 
    }
}

unsigned char* star::core::TextureManager::loadImage(const std::string& path, int& texWidth, int& texHeight, int& texChannel) {
    stbi_uc* pixels = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannel, STBI_rgb_alpha);
    if (!pixels) {
        throw std::runtime_error("Unable to load image");
    }


    return pixels;
}
