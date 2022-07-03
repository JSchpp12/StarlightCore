#pragma once 

#include "SC/Handle.hpp"
#include "SC/GameObject.hpp"
#include "SC/Vertex.hpp"
#include "SC/Mesh.hpp"
#include "Star_Texture.hpp"
#include "Star_Device.hpp"

#include <vulkan/vulkan.hpp>

#include <memory>
#include <vector>
#include <string>

namespace star {
namespace core {
	class RenderObject {
	public:
		struct MeshRenderInfo {
			common::Mesh& mesh;
			std::unique_ptr<StarTexture> starTexture;
			vk::DescriptorSet staticDescriptorSet;			//descriptor set to contain descrptors for ubo and texture for draw
		};

		//eventually use for adding new buffers and shader data to the object which will be queried before draw time
		class Builder {
		public:
			Builder(StarDevice& starDevice, common::GameObject& gameObject) 
				: starDevice(starDevice), gameObject(gameObject) { }
			Builder& addMesh(common::Mesh& mesh, common::Texture& texture); 

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
			std::vector<std::pair<std::reference_wrapper<common::Mesh>, std::reference_wrapper<common::Texture>>> meshInfos; 
		};

		RenderObject(StarDevice& starDevice, common::GameObject& gameObject, std::vector<std::pair<std::reference_wrapper<common::Mesh>, std::reference_wrapper<common::Texture>>> meshInfos,
			size_t numImages = 0);
		//~RenderObject(); 

		void render(vk::CommandBuffer& commandBuffer); 
		
		common::Handle getHandle(); 
		common::GameObject& getGameObject() { return this->gameObject; }
		std::vector<vk::DescriptorSet>& getDefaultDescriptorSets(); 
		const std::vector<std::unique_ptr<MeshRenderInfo>>& getMeshRenderInfos() { return this->meshRenderInfos; }

	protected:


	private: 

		StarDevice& starDevice; 
		common::GameObject& gameObject; 
		//TODO: I would like to make the descriptor sets a unique_ptr
		common::Handle objectHandle;
		std::vector<vk::DescriptorSet> uboDescriptorSets;
		std::vector<std::unique_ptr<MeshRenderInfo>> meshRenderInfos; 

	};
}
}