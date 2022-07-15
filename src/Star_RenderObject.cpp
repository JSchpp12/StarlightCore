#include "Star_RenderObject.hpp"

namespace star::core{
#pragma region Builder
	RenderObject::Builder::Builder(StarDevice& starDevice, common::GameObject& gameObject)
		: starDevice(starDevice), gameObject(gameObject) {
	}
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
	void RenderObject::init(StarDescriptorSetLayout& staticLayout, StarDescriptorPool& staticPool) {
		for (auto& mesh : this->meshes) {
			mesh->init(staticLayout, staticPool); 
		}
	}

	void RenderObject::render(vk::CommandBuffer& commandBuffer, vk::PipelineLayout& pipelineLayout, int swapChainIndexNum) {
		for (auto& mesh : this->meshes) {
			mesh->render(commandBuffer, pipelineLayout, swapChainIndexNum);
		}
	}

	common::Handle RenderObject::getHandle() {
		return this->objectHandle;
	}

	std::vector<vk::DescriptorSet>& RenderObject::getDefaultDescriptorSets() {
		return this->uboDescriptorSets;
	}
}