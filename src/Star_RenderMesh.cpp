#include "Star_RenderMesh.hpp"

namespace star::core {
#pragma region builder 
	RenderMesh::Builder& RenderMesh::Builder::setMesh(common::Mesh& mesh) {
		this->mesh = &mesh; 
		return *this; 
	}
	RenderMesh::Builder& RenderMesh::Builder::setRenderSettings(uint32_t vertexBufferOffset) {
		this->beginIndex = vertexBufferOffset;
		return *this; 
	}
	RenderMesh::Builder& RenderMesh::Builder::setMaterial(std::unique_ptr<RenderMaterial> renderMaterial) {
		this->renderMaterial = std::move(renderMaterial); 
		return *this; 
	}
	std::unique_ptr<RenderMesh> RenderMesh::Builder::build() {
		assert(this->mesh != nullptr  && "A mesh is required to create a RenderMesh object"); 
		if (this->renderMaterial) {
			return std::make_unique<RenderMesh>(this->starDevice, *this->mesh, std::move(this->renderMaterial), this->beginIndex);
		}
		else {
			return std::make_unique<RenderMesh>(starDevice, *mesh, beginIndex);
		}
		
	}
#pragma endregion 
	void RenderMesh::initDescriptorLayouts(StarDescriptorSetLayout::Builder& constBuilder) {
		this->renderMaterial->initDescriptorLayouts(constBuilder); 
	}
	void RenderMesh::initDescriptors(StarDescriptorSetLayout& constLayout, StarDescriptorPool& descriptorPool) {
		auto writer = StarDescriptorWriter(this->starDevice, constLayout, descriptorPool); 

		//add per object const descriptors

		//build per mesh descriptors
		if (this->renderMaterial)
			this->renderMaterial->buildConstDescriptor(writer); 
	}
	void RenderMesh::render(vk::CommandBuffer& commandBuffer, vk::PipelineLayout& pipelineLayout, int swapChainImageIndex) {
		if (this->renderMaterial)
			this->renderMaterial->bind(commandBuffer, pipelineLayout, swapChainImageIndex);

		auto testTriangle = this->mesh.getTriangles()->size() * 3; 
		commandBuffer.drawIndexed(testTriangle, 1, 0, this->startIndex, 0);
	}
}