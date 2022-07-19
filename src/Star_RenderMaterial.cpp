#include "Star_RenderMaterial.hpp"

namespace star::core {
#pragma region Builder
	RenderMaterial::Builder& RenderMaterial::Builder::setMaterial(common::Handle material) {
		assert(this->material == nullptr && "A material has already been applied to this object");

		this->material = &materialManager.get(material); 
		if (this->material->texture.type != common::Handle_Type::null) {
			this->texture = &textureManager.get(this->material->texture); 
		}
		return *this; 
	}
	std::unique_ptr<RenderMaterial> RenderMaterial::Builder::build() {
		assert(this->material != nullptr && "A material object is required to create a RenderMaterial object"); 
		if (!this->texture) {
			return std::make_unique<RenderMaterial>(this->starDevice, *this->material);
		}

		return std::make_unique<RenderMaterial>(this->starDevice, *this->material, *this->texture);
	}
#pragma endregion
	RenderMaterial::RenderMaterial(StarDevice& starDevice, common::Material& material, common::Texture& texture) 
		: starDevice(starDevice), material(material) {

		//prepare vulkan image 
		this->texture = std::make_unique<StarTexture>(starDevice, texture); 
	}

	void RenderMaterial::initDescriptorLayouts(StarDescriptorSetLayout::Builder& constBuilder) {
		if (this->texture) {
			constBuilder.addBinding(1, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment); 
		}
	}

	void RenderMaterial::buildConstDescriptor(StarDescriptorWriter writer) {
		if (this->texture) {
			//buildTextureDescriptor(writer, 1, vk::ImageLayout::eShaderReadOnlyOptimal); 
			vk::DescriptorImageInfo descriptorInfo{
				this->texture->getSampler(),
				this->texture->getImageView(),
				vk::ImageLayout::eShaderReadOnlyOptimal
			};

			writer.writeImage(0, &descriptorInfo);
		}
		writer.build(this->descriptor); 
	}

	void RenderMaterial::bind(vk::CommandBuffer& commandBuffer, vk::PipelineLayout pipelineLayout, int swapChainImageIndex) {
		if (this->descriptor) {
			commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, 2, 1, &this->descriptor, 0, nullptr);
		}
	}

	void RenderMaterial::buildTextureDescriptor(StarDescriptorWriter& constDescriptorWriter, int binding, vk::ImageLayout imageLayout) {
		vk::DescriptorImageInfo descriptorInfo{
			this->texture->getSampler(),
			this->texture->getImageView(),
			imageLayout
		};

		constDescriptorWriter.writeImage(binding, &descriptorInfo);
	}
}