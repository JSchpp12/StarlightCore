#include "TextureManager.h"

star::common::Handle star::core::TextureManager::Add(const std::string& pathToFile, int numChannels){

    
    bool fileLoaded = this->fileContainer.FileLoaded(pathToFile); 

    if (!fileLoaded){

        //TODO: would like to load file here 
        // int width, int height, int nrChannels; 
        std::unique_ptr<common::Texture> newTexture(new common::Texture(pathToFile)); 
        
        return this->fileContainer.AddFileResource(pathToFile, newTexture); 
    }else{
        throw std::runtime_error("Previously loaded files are not yet permitted."); 
    }
}