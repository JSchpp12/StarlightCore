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

	void RenderMaterial::init(StarDescriptorSetLayout& staticDescriptorSetLayout, StarDescriptorPool& staticDescriptorPool) {
		if (this->texture) {
			buildTextureDescriptor(staticDescriptorSetLayout, staticDescriptorPool, 0, vk::ImageLayout::eShaderReadOnlyOptimal); 
		}
	}

	void RenderMaterial::bind(vk::CommandBuffer& commandBuffer, vk::PipelineLayout pipelineLayout, int swapChainImageIndex) {
		if (this->descriptor) {
			commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, 2, 1, &this->descriptor, 0, nullptr);
		}
	}

	void RenderMaterial::buildTextureDescriptor(StarDescriptorSetLayout& staticLayout, StarDescriptorPool& staticPool, int binding, vk::ImageLayout imageLayout) {
		StarDescriptorWriter starDescriptorWriter(this->starDevice, staticLayout, staticPool); 
		vk::DescriptorImageInfo descriptorInfo{
			this->texture->getSampler(),
			this->texture->getImageView(),
			imageLayout
		};

		starDescriptorWriter.writeImage(binding, &descriptorInfo);
		starDescriptorWriter.build(this->descriptor);
	}
}