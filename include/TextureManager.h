#pragma once 

#include "SC/FileResourceManager.hpp"
#include "SC/Texture.hpp"
#include "SC/Handle.hpp"

#include <string> 
#include <memory> 

namespace star{
    namespace core{
        class TextureManager : public common::FileResourceManager<common::Texture> {
        public: 
            virtual common::Handle Add(const std::string& pathToFile); 
        protected: 

        private: 

        }; 
    }
}