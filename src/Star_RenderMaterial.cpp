#include "Star_RenderMaterial.hpp"

namespace star::core {
#pragma region Builder
	RenderMaterial::Builder& RenderMaterial::Builder::setMaterial(common::Handle materialHandle) {
		assert(material == nullptr && "A material has already been applied to this object");

		material = &materialManager.get(materialHandle); 

		return *this; 
	}
	std::unique_ptr<RenderMaterial> RenderMaterial::Builder::build() {
		assert(material != nullptr && "A material object is required to create a RenderMaterial object"); 

		return std::make_unique<RenderMaterial>(starDevice, *material, 
			textureManager.getResource(material->texture), 
			material->bumpMap.type == common::Handle_Type::texture ? textureManager.getResource(material->bumpMap) : mapManager.getResource(material->bumpMap));
	}
#pragma endregion
	RenderMaterial::RenderMaterial(StarDevice& starDevice, common::Material& material, common::Texture& texture, common::Texture& bumpMap) 
		: starDevice(starDevice), material(material), 
		texture(std::make_unique<StarTexture>(starDevice, texture)), 
		bumpMap(std::make_unique<StarTexture>(starDevice, bumpMap)) {
	}

	void RenderMaterial::initDescriptorLayouts(StarDescriptorSetLayout::Builder& constBuilder) {
		constBuilder.addBinding(1, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment); 
		constBuilder.addBinding(2, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment);
	}

	void RenderMaterial::buildConstDescriptor(StarDescriptorWriter writer) {
		writer.writeImage(0, vk::DescriptorImageInfo{
			texture->getSampler(),
			texture->getImageView(),
			vk::ImageLayout::eShaderReadOnlyOptimal });

		writer.writeImage(1, vk::DescriptorImageInfo{
			bumpMap->getSampler(),
			bumpMap->getImageView(),
			vk::ImageLayout::eShaderReadOnlyOptimal});

		writer.build(descriptor); 
	}

	void RenderMaterial::bind(vk::CommandBuffer& commandBuffer, vk::PipelineLayout pipelineLayout, int swapChainImageIndex) {
		if (this->descriptor) {
			commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, 2, 1, &this->descriptor, 0, nullptr);
		}
	}
}