/*
* The StarTexture class inherites from the StarImage class. A StarTexture is used when a textureSampler is needed when binding the image to a shader.
*/
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
		/// <summary>
		/// Create a texture with manually defined attributes
		/// </summary>
		StarTexture(StarDevice& starDevice, const uint32_t& width, const uint32_t& height,
			const vk::Format& format, const vk::ImageTiling& tiling,
			const vk::ImageUsageFlags& usage, const vk::ImageAspectFlags& aspectFlags, const vk::MemoryPropertyFlagBits& memoryProperties = {});
		virtual ~StarTexture() override;

		vk::Sampler getSampler() { return this->textureSampler; }

	private:
		vk::Sampler textureSampler;					//using sampler to apply filtering or other improvements over raw texel access
		vk::DescriptorSet descriptorSet; 
		/// <summary>
		/// Create the rendering structures needed for the provided image.
		/// </summary>
		/// <param name="texture">Texture to create the resources for</param>
		void createTextureImage(common::Texture& texture);

		void transitionImageLayout(vk::Image image, vk::Format format, vk::ImageLayout oldLayout,
			vk::ImageLayout newLayout);

		void createImageSampler();

		vk::ImageView createImageView(vk::Image image, vk::Format format, vk::ImageAspectFlagBits aspectFlags);
	};
}