//wrapper class for textures 
#pragma once

#include "SC/Texture.hpp"

#include "Star_Device.hpp"
#include "Star_Buffer.hpp"

#include <vulkan/vulkan.hpp>

#include <memory>

namespace star {
	namespace core {
		class StarTexture {
		public:
			StarTexture(StarDevice& starDevice, common::Texture& texture);
			~StarTexture();

			vk::ImageView getImageView() { return this->textureImageView; }
			vk::Sampler getSampler() { return this->textureSampler; }
		private:
			StarDevice& starDevice;
			vk::DeviceSize storageBufferOffset;
			vk::Image textureImage;
			vk::ImageView textureImageView;
			vk::Sampler textureSampler;
			vk::DeviceMemory imageMemory;

			void createTextureImage(common::Texture& texture);
			/// <summary>
			/// Create Vulkan Image object with properties provided in function arguments. 
			/// </summary>
			/// <param name="width">Width of the image being created</param>
			/// <param name="height">Height of the image being created</param>
			/// <param name="format"></param>
			/// <param name="tiling"></param>
			/// <param name="usage"></param>
			/// <param name="properties"></param>
			/// <param name="image"></param>
			/// <param name="imageMemory"></param>
			void createImage(uint32_t width, uint32_t height, vk::Format format, vk::ImageTiling tiling,
				vk::ImageUsageFlags usage, vk::MemoryPropertyFlagBits properties,
				vk::Image& image, vk::DeviceMemory& imageMemory);

			void transitionImageLayout(vk::Image image, vk::Format format, vk::ImageLayout oldLayout,
				vk::ImageLayout newLayout);

			void createTextureImageView();

			vk::ImageView createImageView(vk::Image image, vk::Format format, vk::ImageAspectFlagBits aspectFlags);
		};
	}
}