#pragma once 

#include "SC/Handle.hpp"
#include "SC/GameObject.hpp"
#include "SC/Vertex.hpp"
#include "SC/Mesh.hpp"
#include "Star_Texture.hpp"
#include "Star_Device.hpp"
#include "Star_RenderMesh.hpp"

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
			Builder(StarDevice& starDevice, common::GameObject& gameObject);
			Builder& addMesh(std::unique_ptr<RenderMesh> renderMesh); 

			/// <summary>
			/// Record the number of images in the swapchain. This will be used to resize the descriptor bindings. 
			/// </summary>
			/// <param name="numImages"></param>
			/// <returns></returns>
			Builder& setNumFrames(size_t numImages); 
			std::unique_ptr<RenderObject> build(); 

		protected:


		private: 
			StarDevice& starDevice; 
			common::GameObject& gameObject; 
			size_t numSwapChainImages; 
			std::vector<std::unique_ptr<RenderMesh>> meshes; 
		};

		RenderObject(StarDevice& starDevice, common::GameObject& gameObject, std::vector<std::unique_ptr<RenderMesh>> meshes,
			size_t numImages = 0) : starDevice(starDevice), objectHandle(objectHandle), meshes(std::move(meshes)),
			gameObject(gameObject), uboDescriptorSets(numImages) { }

		void render(vk::CommandBuffer& commandBuffer, vk::PipelineLayout& pipelineLayout, int swapChainIndexNum); 

		/// <summary>
		/// Build descriptors which will only be written to once during renderer init
		/// </summary>
		/// <param name="descriptorWriter">Descriptor writer to use when creating new descriptors</param>
		void buildConstantDescriptors(StarDescriptorWriter& descriptorWriter);

		//TODO: might want to create render function for each mesh as they get more complicated
		//void render(vk::CommandBuffer& commandBuffer); 
		
		common::Handle getHandle(); 
		common::GameObject& getGameObject() { return this->gameObject; }
		std::vector<vk::DescriptorSet>& getDefaultDescriptorSets(); 
		//TODO: remove this
		const std::vector<std::unique_ptr<RenderMesh>>& getMeshes() { return this->meshes; }
	protected:


	private: 
		StarDevice& starDevice; 
		common::GameObject& gameObject; 
		//TODO: I would like to make the descriptor sets a unique_ptr
		common::Handle objectHandle;
		std::vector<vk::DescriptorSet> uboDescriptorSets;
		std::vector<std::unique_ptr<RenderMesh>> meshes; 

	};
}
}