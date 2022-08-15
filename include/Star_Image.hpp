/*
* This class is a wrapper class for vulkan images. It includes all needed operations in order to create a vk::Image
*/
#pragma once 

#include "Star_Device.hpp"

#include <vulkan/vulkan.hpp>

namespace star::core {
	class StarImage {
	public: 
		struct ImageCreationDetails	{
			uint32_t width = 0;
			uint32_t height = 0;
			vk::Format format = {};
			vk::ImageTiling tiling = {};
			vk::ImageUsageFlags usage = {};
			vk::MemoryPropertyFlags memoryProperties = {};
		};
		/// <summary>
		/// Create vk::image and all required structures with provided arguments
		/// </summary>
		/// <param name="starDevice"></param>
		/// <param name="width"></param>
		/// <param name="height"></param>
		/// <param name="format"></param>
		/// <param name="tiling"></param>
		/// <param name="usage"></param>
		/// <param name="aspectFlags">Aspect flags to be used in creating the vk::ImageView</param>
		/// <param name="memoryProperties"></param>
		StarImage(StarDevice& starDevice, const uint32_t& width, const uint32_t& height, 
			const vk::Format& format, const vk::ImageTiling& tiling,
			const vk::ImageUsageFlags& usage, const vk::ImageAspectFlags& aspectFlags, const vk::MemoryPropertyFlagBits& memoryProperties = {});
		virtual ~StarImage(); 

		static vk::ImageView createImageView(StarDevice& starDevice, vk::Image& image, const vk::Format& format, const vk::ImageAspectFlags& aspectFlags);

		vk::ImageView& getImageView() { return imageView; }
	protected:
		StarDevice& starDevice; 
		vk::Image image;
		vk::ImageView imageView; 
		vk::DeviceMemory imageMemory;
		/// <summary>
		/// Only allow inhertied classes access to the basic constructor of this class.
		/// This requires that the inherited class initialize all of the necessary structures needed during rendering ops. 
		/// </summary>
		/// <param name="starDevice"></param>
		StarImage(StarDevice& starDevice) : starDevice(starDevice) { }
		/// <summary>
		/// Initialize and create image resources if they are not already created. This should only be used if the minimal
		/// constructor for the class was used
		/// </summary>
		/// <param name="width"></param>
		/// <param name="height"></param>
		/// <param name="format"></param>
		/// <param name="tiling"></param>
		/// <param name="usage"></param>
		/// <param name="aspectFlags"></param>
		/// <param name="memoryProperties"></param>
		virtual void init(const uint32_t& width, const uint32_t& height, const vk::Format& format,
			const vk::ImageTiling& tiling, const vk::ImageUsageFlags& usage,
			const vk::ImageAspectFlags& aspectFlags, const vk::MemoryPropertyFlagBits& memoryProperties);

		void createImage(const uint32_t& width, const uint32_t& height, const vk::Format& format, 
			const vk::ImageTiling& tiling, const vk::ImageUsageFlags& usage,
			const vk::MemoryPropertyFlagBits& properties);

	private:


	};
}