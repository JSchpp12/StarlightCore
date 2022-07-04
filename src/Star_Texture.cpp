#include "Star_Texture.hpp"

namespace star {
	namespace core {
		StarTexture::StarTexture(StarDevice& starDevice, common::Texture& texture) : starDevice(starDevice) {
			createTextureImage(texture);
			createTextureImageView(); 
			createImageSampler();
		}

		StarTexture::~StarTexture() {
			this->starDevice.getDevice().destroySampler(this->textureSampler);
			this->starDevice.getDevice().destroyImageView(this->textureImageView);
			this->starDevice.getDevice().destroyImage(this->textureImage);

			this->starDevice.getDevice().freeMemory(this->imageMemory);
		}

		void StarTexture::createTextureImage(common::Texture& texture) {
			int texWidth;
			int texHeight;
			int texChannels;

			//stbi_uc* pixels = stbi_load(texture->path().c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha); 
			vk::DeviceSize imageSize = texture.width() * texture.height() * 4;

			StarBuffer stagingBuffer(
				this->starDevice,
				imageSize,
				1,
				vk::BufferUsageFlagBits::eTransferSrc,
				vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
			stagingBuffer.map();
			stagingBuffer.writeToBuffer(texture.data(), imageSize);

			//stbi_image_free(pixels);

			createImage(texture.width(), texture.height(), vk::Format::eR8G8B8A8Srgb, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled, vk::MemoryPropertyFlagBits::eDeviceLocal, textureImage, imageMemory);

			//copy staging buffer to texture image 
			transitionImageLayout(textureImage, vk::Format::eR8G8B8A8Srgb, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);

			this->starDevice.copyBufferToImage(stagingBuffer.getBuffer(), textureImage, static_cast<uint32_t>(texture.width()), static_cast<uint32_t>(texture.height()));

			//prepare final image for texture mapping in shaders 
			transitionImageLayout(textureImage, vk::Format::eR8G8B8A8Srgb, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);

		}

		void StarTexture::createImage(uint32_t width, uint32_t height, vk::Format format,
			vk::ImageTiling tiling, vk::ImageUsageFlags usage,
			vk::MemoryPropertyFlagBits properties, vk::Image& image, vk::DeviceMemory& imageMemory) {

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

			image = this->starDevice.getDevice().createImage(imageInfo);
			if (!image) {
				throw std::runtime_error("failed to create image");
			}

			/* Allocate the memory for the imag*/
			vk::MemoryRequirements memRequirements = this->starDevice.getDevice().getImageMemoryRequirements(image);

			vk::MemoryAllocateInfo allocInfo{};
			allocInfo.sType = vk::StructureType::eMemoryAllocateInfo;
			allocInfo.allocationSize = memRequirements.size;
			allocInfo.memoryTypeIndex = this->starDevice.findMemoryType(memRequirements.memoryTypeBits, properties);

			imageMemory = this->starDevice.getDevice().allocateMemory(allocInfo);
			if (!imageMemory) {
				throw std::runtime_error("failed to allocate image memory!");
			}

			this->starDevice.getDevice().bindImageMemory(image, imageMemory, 0);
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

		//TODO: allow for more formats 
		void StarTexture::createTextureImageView() {
			this->textureImageView = createImageView(textureImage, vk::Format::eR8G8B8A8Srgb, vk::ImageAspectFlagBits::eColor);
		}

		vk::ImageView StarTexture::createImageView(vk::Image image, vk::Format format, vk::ImageAspectFlagBits aspectFlags) {
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

			vk::ImageView imageView = this->starDevice.getDevice().createImageView(viewInfo);

			if (!imageView) {
				throw std::runtime_error("failed to create texture image view!");
			}

			return imageView;
		}
	}
}