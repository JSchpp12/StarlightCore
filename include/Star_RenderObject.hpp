#pragma once 

#include "SC/Handle.hpp"
#include "SC/GameObject.hpp"
#include "SC/Vertex.hpp"
#include "Star_Texture.hpp"

#include <vulkan/vulkan.hpp>

#include <memory>
#include <vector>
#include <string>

namespace star {
namespace core {
	class RenderObject {
	public:
		//eventually use for adding new buffers and shader data to the object which will be queried before draw time
		class Builder {
		public:
			Builder() = default; 
			Builder& setFromObject(common::Handle objectHandle, common::GameObject* object); 
			Builder& setTexture(common::Texture* texture); 
			/// <summary>
			/// Record the number of images in the swapchain. This will be used to resize the descriptor bindings. 
			/// </summary>
			/// <param name="numImages"></param>
			/// <returns></returns>
			Builder& setNumFrames(size_t numImages); 

			std::unique_ptr<RenderObject> build(); 

		protected:


		private: 
			common::Handle objectHandle; 
			common::GameObject* gameObject = nullptr; 
			common::Texture* texture = nullptr; 
			size_t numVerticies, numIndicies, numSwapChainImages; 
		};

		RenderObject(common::Handle objectHandle, common::GameObject* gameObject, size_t numVerticies, common::Texture* texture,
			size_t numIndicies, size_t numImages = 0) : 
			objectHandle(objectHandle),
			gameObject(gameObject),
			numVerticies(numVerticies), 
			numIndicies(numIndicies), 
			staticDescriptorSet(),
			uboDescriptorSets(numImages) {
			this->texture = texture;
		}
		//~RenderObject(); 

		//void render(vk::CommandBuffer& commandBuffer, vk::PipelineLayout layout, int swapChainImageIndex); 
		
		common::Handle getHandle(); 

		size_t getNumVerticies(); 

		size_t getNumIndicies();
		common::GameObject* getGameObject() { return this->gameObject; }
		std::vector<vk::DescriptorSet>* getDefaultDescriptorSets(); 
		vk::DescriptorSet& getStaticDescriptorSet();
		common::Texture* getTexture() { return this->texture; }

	protected:


	private: 
		common::Handle objectHandle;
		common::GameObject* gameObject = nullptr;
		common::Texture* texture = nullptr; 
		vk::DescriptorSet staticDescriptorSet; 
		std::vector<vk::DescriptorSet> uboDescriptorSets; 
		size_t numVerticies, numIndicies; 

	};
}
}