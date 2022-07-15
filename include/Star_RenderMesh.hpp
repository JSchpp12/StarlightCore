#pragma once 

#include "SC/Mesh.hpp"
#include "SC/Material.hpp"
#include "Star_Descriptors.hpp"
#include "Star_RenderMaterial.hpp"
#include "Star_Pipeline.hpp"
#include "Star_Device.hpp"
#include "Star_RenderMesh.hpp"

#include <vulkan/vulkan.hpp>

#include <memory>

namespace star::core {
	class RenderMesh {
	public:
		class Builder {
		public:
			Builder(StarDevice& starDevice) : starDevice(starDevice) { }
			/// <summary>
			/// Set the common::Mesh object that the render object will be generated from 
			/// </summary>
			/// <param name="mesh">Target mesh object</param>
			/// <returns></returns>
			Builder& setMesh(common::Mesh& mesh); 
			/// <summary>
			/// Set properties relevant to rendering operations. 
			/// </summary>
			/// <param name="vertexBufferOffset">Offset within the vertex buffer that will be used when submitting draw commands</param>
			/// <returns></returns>
			Builder& setRenderSettings(uint32_t vertexBufferOffset); 
			Builder& setMaterial(std::unique_ptr<RenderMaterial> renderMaterial); 
			std::unique_ptr<RenderMesh> build(); 

		private:
			StarDevice& starDevice;
			std::unique_ptr<RenderMaterial> renderMaterial; 
			common::Mesh* mesh = nullptr; 
			uint32_t beginIndex = 0; 

		};

		RenderMesh(StarDevice& starDevice, common::Mesh& mesh, std::unique_ptr<RenderMaterial> material, uint32_t vbOffset = 0)
			: starDevice(starDevice), mesh(mesh), renderMaterial(std::move(material)) {
			this->startIndex = vbOffset;
		}
		/// <summary>
		/// Init object with needed structures such as descriptors
		/// </summary>
		/// <param name="staticDescriptorWriter">Writer to use when creating static descriptors (those updated once on init)</param>
		void init(StarDescriptorSetLayout& staticLayout, StarDescriptorPool& staticPool);
		void render(vk::CommandBuffer& commandBuffer, vk::PipelineLayout& pipelineLayout, int swapChainImageIndex); 

		void setVBOffset(uint32_t offset) { this->startIndex = offset; }
		RenderMaterial& getMaterial() { return *this->renderMaterial; }
		common::Mesh& getMesh() { return this->mesh; }
		vk::DescriptorSet& getDescriptor() { return this->renderMaterial->getDescriptor(); }

	private: 
		StarDevice& starDevice; 
		common::Mesh& mesh; 
		std::unique_ptr<RenderMaterial> renderMaterial; 
		uint32_t startIndex;								//index in vertex buffer where draw will begin             

	};
}