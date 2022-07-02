#include "Star_RenderObject.hpp"

namespace star{
namespace core {
	RenderObject::Builder& RenderObject::Builder::setFromObject(common::Handle objectHandle, common::GameObject* object) {
		this->numVerticies = object->getVerticies()->size(); 
		this->numIndicies = object->getIndicies()->size(); 
		this->objectHandle = objectHandle; 
		this->gameObject = object; 
		return *this; 
	}

	RenderObject::Builder& RenderObject::Builder::setNumFrames(size_t numImages) {
		this->numSwapChainImages = numImages; 
		return *this; 
	}

	RenderObject::Builder& RenderObject::Builder::setTexture(common::Texture* texture) {
		this->texture = texture; 
		return *this; 
	}

	std::unique_ptr<RenderObject> RenderObject::Builder::build() {
		return std::make_unique<RenderObject>(this->starDevice, this->objectHandle, this->gameObject, this->texture, this->numVerticies, this->numIndicies, this->numSwapChainImages); 
	}


	RenderObject::RenderObject(StarDevice& starDevice, common::Handle objectHandle, common::GameObject* gameObject,
		common::Texture* texture, size_t numVerticies, 
		size_t numIndicies, size_t numImages) :
		starDevice(starDevice), objectHandle(objectHandle),
		gameObject(gameObject), numVerticies(numVerticies),
		numIndicies(numIndicies), staticDescriptorSet(),
		uboDescriptorSets(numImages) {
		//create texture as needed
		if (texture) {
			this->starTexture = std::make_unique<StarTexture>(this->starDevice, *texture);
		}
	}

	//void RenderObject::render(vk::CommandBuffer& commandBuffer, vk::PipelineLayout layout, int swapChainImageIndex) {
	//	commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, layout, 1, 1, &this->uboDescriptorSets.at(swapChainImageIndex), 0, nullpt)
	//}

	common::Handle RenderObject::getHandle() {
		return this->objectHandle;
	}

	size_t RenderObject::getNumVerticies() {
		return this->numVerticies; 
	}

	size_t RenderObject::getNumIndicies() {
		return this->numIndicies; 
	}

	std::vector<vk::DescriptorSet>* RenderObject::getDefaultDescriptorSets() {
		return &this->uboDescriptorSets;
	}

	vk::DescriptorSet& RenderObject::getStaticDescriptorSet() {
		return this->staticDescriptorSet; 
	}
}
}