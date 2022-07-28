//wrapper class for textures 
#pragma once

#include "SC/Texture.hpp"

#include "Star_Image.hpp"
#include "Star_Device.hpp"
#include "Star_Buffer.hpp"
#include "Star_Descriptors.hpp"

#include <vulkan/vulkan.hpp>

#include <memory>

namespace star::core{
	class StarTexture : public StarImage {
	public:
		StarTexture(StarDevice& starDevice, common::Texture& texture);
		virtual ~StarTexture() override;

		vk::Sampler getSampler() { return this->textureSampler; }

	private:
		vk::Sampler textureSampler;					//using sampler to apply filtering or other improvements over raw texel access
		vk::DescriptorSet descriptorSet; 

		void createTextureImage(common::Texture& texture);

		void transitionImageLayout(vk::Image image, vk::Format format, vk::ImageLayout oldLayout,
			vk::ImageLayout newLayout);

		void createImageSampler();

		vk::ImageView createImageView(vk::Image image, vk::Format format, vk::ImageAspectFlagBits aspectFlags);
	};
}