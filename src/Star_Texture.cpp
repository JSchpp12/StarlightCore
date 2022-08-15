#include "Star_Texture.hpp"

namespace star::core{
	StarTexture::StarTexture(StarDevice& starDevice, common::Texture& texture) : StarImage(starDevice) {
		createTextureImage(texture);
		createImageSampler();
	}

	StarTexture::StarTexture(StarDevice& starDevice, const uint32_t& width, const uint32_t& height,
		const vk::Format& format, const vk::ImageTiling& tiling,
		const vk::ImageUsageFlags& usage, const vk::ImageAspectFlags& aspectFlags,
		const vk::MemoryPropertyFlagBits& memoryProperties = {})
		: StarImage(starDevice, width, height, format, tiling, usage, aspectFlags, memoryProperties) {
	}

	StarTexture::~StarTexture() {
		this->starDevice.getDevice().destroySampler(this->textureSampler);
	}

	void StarTexture::createTextureImage(common::Texture& texture) {
		vk::DeviceSize imageSize = texture.width * texture.height * 4;

		StarBuffer stagingBuffer(
			starDevice,
			imageSize,
			1,
			vk::BufferUsageFlagBits::eTransferSrc,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
		stagingBuffer.map();
		std::unique_ptr<unsigned char> textureData(texture.data());
		stagingBuffer.writeToBuffer(textureData.get(), imageSize);

		this->StarImage::init(texture.width, texture.height, vk::Format::eR8G8B8A8Srgb, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled, vk::ImageAspectFlagBits::eColor, vk::MemoryPropertyFlagBits::eDeviceLocal);

		//copy staging buffer to texture image 
		transitionImageLayout(this->image, vk::Format::eR8G8B8A8Srgb, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);

		starDevice.copyBufferToImage(stagingBuffer.getBuffer(), this->image, static_cast<uint32_t>(texture.width), static_cast<uint32_t>(texture.height));

		//prepare final image for texture mapping in shaders 
		transitionImageLayout(this->image, vk::Format::eR8G8B8A8Srgb, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);
	}

	void StarTexture::transitionImageLayout(vk::Image image, vk::Format format, vk::ImageLayout oldLayout,
		vk::ImageLayout newLayout) {
		vk::CommandBuffer commandBuffer = this->starDevice.beginSingleTimeCommands();

		//create a barrier to prevent pipeline from moving forward until image transition is complete
		vk::ImageMemoryBarrier barrier{};
		//barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;     //specific flag for image operations
		barrier.sType = vk::StructureType::eImageMemoryBarrier;     //specific flag for image operations
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;

		//if barrier is used for transferring ownership between queue families, this would be important -- set to ignore since we are not doing this
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

		barrier.image = image;
		//barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
		barrier.subresourceRange.baseMipLevel = 0;                          //image does not have any mipmap levels
		barrier.subresourceRange.levelCount = 1;                            //image is not an array
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		//the operations that need to be completed before and after the barrier, need to be defined
		barrier.srcAccessMask = {}; //TODO
		barrier.dstAccessMask = {}; //TODO

		vk::PipelineStageFlags sourceStage, destinationStage;

		if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal) {
			//undefined transition state, dont need to wait for this to complete
			barrier.srcAccessMask = {};
			//barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

			sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
			destinationStage = vk::PipelineStageFlagBits::eTransfer;
		}
		else if (oldLayout == vk::ImageLayout::eTransferDstOptimal && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
			//transfer destination shdaer reading, will need to wait for completion. Especially in the frag shader where reads will happen
			barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
			barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

			sourceStage = vk::PipelineStageFlagBits::eTransfer;
			destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
		}
		else {
			throw std::invalid_argument("unsupported layout transition!");
		}

		//transfer writes must occurr during the pipeline transfer stage
		commandBuffer.pipelineBarrier(
			sourceStage,                        //which pipeline stages should occurr before barrier 
			destinationStage,                   //pipeline stage in which operations iwll wait on the barrier 
			{},
			{},
			nullptr,
			barrier
		);

		this->starDevice.endSingleTimeCommands(commandBuffer);
	}

	void StarTexture::createImageSampler() {
		//get device properties for amount of anisotropy permitted
		vk::PhysicalDeviceProperties deviceProperties = this->starDevice.getPhysicalDevice().getProperties();

		vk::SamplerCreateInfo samplerInfo{};
		samplerInfo.sType = vk::StructureType::eSamplerCreateInfo;
		samplerInfo.magFilter = vk::Filter::eLinear;                       //how to sample textures that are magnified 
		samplerInfo.minFilter = vk::Filter::eLinear;                       //how to sample textures that are minified

		//repeat mode - repeat the texture when going beyond the image dimensions
		samplerInfo.addressModeU = vk::SamplerAddressMode::eRepeat;
		samplerInfo.addressModeV = vk::SamplerAddressMode::eRepeat;
		samplerInfo.addressModeW = vk::SamplerAddressMode::eRepeat;

		//should anisotropic filtering be used? Really only matters if performance is a concern
		samplerInfo.anisotropyEnable = VK_TRUE;
		//specifies the limit on the number of texel samples that can be used (lower = better performance)
		samplerInfo.maxAnisotropy = deviceProperties.limits.maxSamplerAnisotropy;;
		samplerInfo.borderColor = vk::BorderColor::eIntOpaqueBlack;
		//specifies coordinate system to use in addressing texels. 
			//VK_TRUE - use coordinates [0, texWidth) and [0, texHeight]
			//VK_FALSE - use [0, 1)
		samplerInfo.unnormalizedCoordinates = VK_FALSE;

		//if comparing, the texels will first compare to a value, the result of the comparison is used in filtering operations (percentage-closer filtering on shadow maps)
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = vk::CompareOp::eAlways;

		//following apply to mipmapping -- not using here
		samplerInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 0.0f;
		samplerInfo.anisotropyEnable = VK_FALSE;

		this->textureSampler = this->starDevice.getDevice().createSampler(samplerInfo);
		if (!this->textureSampler) {
			throw std::runtime_error("failed to create texture sampler!");
		}
	}
}