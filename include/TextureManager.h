#pragma once 

#include "SC/FileResourceManager.hpp"
#include "SC/Texture.hpp"
#include "SC/Handle.hpp"

#include <string> 
#include <memory> 

namespace star{
    namespace core{
        class TextureManager : common::FileResourceManager<common::Texture>{
        public: 
            common::Handle Add(const std::string& pathToFile, int numChannels); 
        protected: 

        private: 

        }; 
    }
}