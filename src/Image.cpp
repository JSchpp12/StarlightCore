//#include "Image.h"
//
//#define STB_IMAGE_IMPLEMENTATION   // use of stb functions once and for all
//#include "stb_image.h"
//
//star::core::Image::Image(const std::string& pathToImage, int texWidth, int texHeight, int texChannels) : 
//	common::Texture(pathToImage, texWidth, texHeight, texChannels)
//{
//
//}
//
//unsigned char* star::core::Image::load() {
//	stbi_uc* pixels = stbi_load(this->path.c_str(), &this->texWidth, &this->texHeight, &this->texChannels, STBI_rgb_alpha);
//	if (!pixels) {
//		throw std::runtime_error("Unable to load image");
//	}
//	return pixels; 
//}