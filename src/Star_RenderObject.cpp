#include "Star_RenderObject.hpp"

namespace star{
namespace core {
	RenderObject::Builder& RenderObject::Builder::setFromObject(common::Handle objectHandle, common::LogicalObject* object) {
		this->numVerticies = object->getVerticies()->size(); 
		this->numIndicies = object->getIndicies()->size(); 
		this->objectHandle = objectHandle; 
		return *this; 
	}

	RenderObject::Builder& RenderObject::Builder::setNumFrames(size_t numImages) {
		this->numSwapChainImages = numImages; 
		return *this; 
	}

	std::unique_ptr<RenderObject> RenderObject::Builder::build() {
		return std::make_unique<RenderObject>(this->objectHandle, this->numVerticies, this->numIndicies, this->numSwapChainImages); 
	}

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
}
}