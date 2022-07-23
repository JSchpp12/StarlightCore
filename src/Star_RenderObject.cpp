#include "Star_RenderObject.hpp"

namespace star::core{
#pragma region Builder
	RenderObject::Builder::Builder(StarDevice& starDevice, common::GameObject& gameObject)
		: starDevice(starDevice), gameObject(gameObject) { }
	RenderObject::Builder& RenderObject::Builder::setNumFrames(size_t numImages) {
		this->numSwapChainImages = numImages; 
		return *this; 
	}
	RenderObject::Builder& RenderObject::Builder::addMesh(std::unique_ptr<RenderMesh> renderMesh) {
		this->meshes.push_back(std::move(renderMesh)); 
		return *this; 
	}
	std::unique_ptr<RenderObject> RenderObject::Builder::build() {
		assert(this->meshes.size() != 0 && "At least one mesh must be assigned to a render object");
		assert(this->numSwapChainImages > 0 && "Number of swap chain images must be set");

		return std::make_unique<RenderObject>(this->starDevice, this->gameObject, std::move(this->meshes), this->numSwapChainImages); 
	}
#pragma endregion 
	void RenderObject::initDescriptorLayouts(StarDescriptorSetLayout::Builder& constBuilder) {
		//create
		for (auto& mesh : this->meshes) {
			mesh->initDescriptorLayouts(constBuilder); 
		}
	}

	void RenderObject::initDescriptors(StarDescriptorSetLayout& constLayout, StarDescriptorPool& descriptorPool) {
		//add descriptors for each mesh -- right now this is the image descriptor
		for (auto& mesh : this->meshes) {

			mesh->initDescriptors(constLayout, descriptorPool); 
		}
	}

	void RenderObject::render(vk::CommandBuffer& commandBuffer, vk::PipelineLayout& pipelineLayout, int swapChainIndexNum) {
		for (auto& mesh : this->meshes) {
			mesh->render(commandBuffer, pipelineLayout, swapChainIndexNum);
		}
	}

	std::vector<vk::DescriptorSet>& RenderObject::getDefaultDescriptorSets() {
		return this->uboDescriptorSets;
	}
}