#include "Star_RenderMaterial.hpp"

namespace star::core {
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

	RenderMaterial::RenderMaterial(StarDevice& starDevice, common::Material& material, common::Texture& texture) 
		: starDevice(starDevice), material(material) {

		//prepare vulkan image 
		this->texture = std::make_unique<StarTexture>(starDevice, texture); 
	}

	void RenderMaterial::buildTextureDescriptor (StarDescriptorWriter descriptorWriter, int binding, vk::ImageLayout imageLayout) {
		vk::DescriptorImageInfo descriptorInfo{
			this->texture->getSampler(),
			this->texture->getImageView(),
			imageLayout
		};

		descriptorWriter.writeImage(binding, &descriptorInfo);
		descriptorWriter.build(this->descriptor);
	}
}