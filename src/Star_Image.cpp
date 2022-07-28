#include "Star_Image.hpp"

namespace star::core {
	StarImage::StarImage(StarDevice& starDevice, const uint32_t& width, const uint32_t& height,
		const vk::Format& format, const vk::ImageTiling& tiling,
		const vk::ImageUsageFlags& usage, const vk::ImageAspectFlags& aspectFlags, 
		const vk::MemoryPropertyFlagBits& memoryProperties) 
		: starDevice(starDevice) {
		init(width, height, format, tiling, usage, aspectFlags, memoryProperties);
	}

	StarImage::~StarImage() {
		starDevice.getDevice().destroyImageView(imageView);
		starDevice.getDevice().destroyImage(image); 
		starDevice.getDevice().freeMemory(imageMemory);
	}

	void StarImage::init(const uint32_t& width, const uint32_t& height, const vk::Format& format,
		const vk::ImageTiling& tiling, const vk::ImageUsageFlags& usage,
		const vk::ImageAspectFlags& aspectFlags, const vk::MemoryPropertyFlagBits& memoryProperties) {
		assert(!image && !imageView && "This image has already been created!");

		createImage(width, height, format, tiling, usage, memoryProperties); 
		imageView = createImageView(starDevice, image, format, aspectFlags);
	}

	vk::ImageView StarImage::createImageView(StarDevice& starDevice, vk::Image& image, const vk::Format& format, const vk::ImageAspectFlags& aspectFlags) {
		vk::ImageViewCreateInfo viewInfo{};
		viewInfo.sType = vk::StructureType::eImageViewCreateInfo;
		viewInfo.image = image;
		viewInfo.viewType = vk::ImageViewType::e2D;
		viewInfo.format = format;
		viewInfo.subresourceRange.aspectMask = aspectFlags;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		vk::ImageView imageView = starDevice.getDevice().createImageView(viewInfo);

		if (!imageView) {
			throw std::runtime_error("failed to create texture image view!");
		}

		return imageView;
	}

	void StarImage::createImage(const uint32_t& width, const uint32_t& height, const vk::Format& format,
		const vk::ImageTiling& tiling, const vk::ImageUsageFlags& usage,
		const vk::MemoryPropertyFlagBits& properties) {
		/* Create vulkan image */
		vk::ImageCreateInfo imageInfo{};

		imageInfo.sType = vk::StructureType::eImageCreateInfo;
		imageInfo.imageType = vk::ImageType::e2D;
		imageInfo.extent.width = width;
		imageInfo.extent.height = height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.format = format;
		imageInfo.tiling = tiling;
		imageInfo.initialLayout = vk::ImageLayout::eUndefined;
		imageInfo.usage = usage;
		imageInfo.samples = vk::SampleCountFlagBits::e1;
		imageInfo.sharingMode = vk::SharingMode::eExclusive;

		image = starDevice.getDevice().createImage(imageInfo);
		if (!image) {
			throw std::runtime_error("failed to create image");
		}

		/* Allocate the memory for the imag*/
		vk::MemoryRequirements memRequirements = starDevice.getDevice().getImageMemoryRequirements(image);

		vk::MemoryAllocateInfo allocInfo{};
		allocInfo.sType = vk::StructureType::eMemoryAllocateInfo;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = starDevice.findMemoryType(memRequirements.memoryTypeBits, properties);

		imageMemory = starDevice.getDevice().allocateMemory(allocInfo);
		if (!imageMemory) {
			throw std::runtime_error("failed to allocate image memory!");
		}

		starDevice.getDevice().bindImageMemory(image, imageMemory, 0);
	}
}