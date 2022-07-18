/*
* This class will contain all information needed to render a material. The common::Material class only contains handles
* to required members (i.e. textures). These required members will be collected from managers into this class for rendering
* operations.
*/
#pragma once 
#include "SC/Enums.h"
#include "SC/Material.hpp"
#include "SC/Texture.hpp"
#include "Star_Device.hpp"
#include "Star_Texture.hpp"
#include "Star_Descriptors.hpp"
#include "MaterialManager.hpp"
#include "TextureManager.h"

#include <vulkan/vulkan.hpp>

#include <memory>

namespace star::core {
	class RenderMaterial {
	public:
		class Builder {
		public:
			Builder(StarDevice& starDevice, MaterialManager& materialManager, TextureManager& textureManager): starDevice(starDevice), 
				materialManager(materialManager), textureManager(textureManager){ }
			Builder& setMaterial(common::Handle material);
			std::unique_ptr<RenderMaterial> build();

		private:
			StarDevice& starDevice;
			MaterialManager& materialManager;
			TextureManager& textureManager;
			common::Material* material = nullptr;
			common::Texture* texture = nullptr;
		};
		/// <summary>
		/// Initalize the const descriptor set layouts with needed descriptor slots
		/// </summary>
		/// <param name="constBuilder"></param>
		void initDescriptorLayouts(StarDescriptorSetLayout::Builder& constBuilder); 
		/// <summary>
		/// Init Render Material with proper descriptors
		/// </summary>
		/// <param name="staticDescriptorSetLayout">DescriptorSetLayout to be used when creating object descriptors which are updated once (during init)</param>
		void buildConstDescriptor(StarDescriptorWriter writer);

		//need to gather refs to base materials
		std::unique_ptr<StarTexture> texture;

		RenderMaterial(StarDevice& starDevice, common::Material& material) : starDevice(starDevice), material(material) { }

		RenderMaterial(StarDevice& starDevice, common::Material& material, common::Texture& texture);

		void bind(vk::CommandBuffer& commandBuffer, vk::PipelineLayout pipelineLayout, int swapChainImageIndex); 

		//todo: might want to move all descriptor creation to individual classes
		//void addRequiredBindings();

		/// <summary>
		/// Build texture descriptors for this material. This should be added onto the other descriptors which will only be updated once
		/// </summary>
		/// <param name="writer"></param>
		/// <param name="binding"></param>
		/// <param name="layout"></param>
		void buildTextureDescriptor(StarDescriptorWriter& constDescriptorWriter, int binding, vk::ImageLayout imageLayout);

		//get 
		common::Material& getMaterial() { return this->material; }
		StarTexture& getTexture() { return *this->texture; }
		vk::DescriptorSet& getDescriptor() { return this->descriptor; }

	private:
		StarDevice& starDevice; 
		common::Material& material; 
		vk::DescriptorSet descriptor; 

	};
}