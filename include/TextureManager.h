#pragma once 
#include "Image.h"

#include "SC/FileResourceManager.hpp"
#include "SC/Texture.hpp"
#include "SC/Handle.hpp"

#include <string> 
#include <memory> 

namespace star{
    namespace core{
        class TextureManager : public common::FileResourceManager<common::Texture> {
        public: 
            ~TextureManager(); 
            
            common::Handle add(const std::string& pathToFile); 

        protected: 

        private: 
            unsigned char* loadImage(const std::string& path, int& texWidth, int& texHeight, int& texChannel); 
        }; 
    }
}