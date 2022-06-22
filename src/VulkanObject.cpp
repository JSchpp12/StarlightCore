#include "VulkanObject.h"

namespace star {
namespace core {

void VulkanObject::cleanup() {
}

void VulkanObject::registerShader(vk::ShaderStageFlagBits stage, common::Shader* newShader, common::Handle newShaderHandle) {
	if (stage & vk::ShaderStageFlagBits::eVertex) {
		this->vertShader = newShader; 
		this->vertShaderHandle = newShaderHandle; 
	}
	else {
		this->fragShader = newShader; 
		this->fragShaderHandle = newShaderHandle; 
	}
}

void VulkanObject::addObject(common::Handle newObjectHandle, common::GameObject* newObject, size_t numSwapChainImages) {
	auto numIndicies = newObject->getIndicies()->size();
	auto numVerticies = newObject->getVerticies()->size();

	this->totalNumIndicies += numIndicies;
	this->totalNumVerticies += numVerticies;

	this->renderObjects.push_back(std::move(RenderObject::Builder().setFromObject(newObjectHandle, newObject).setNumFrames(numSwapChainImages).build())); 
}

bool VulkanObject::hasShader(vk::ShaderStageFlagBits stage) {
	if (stage & vk::ShaderStageFlagBits::eVertex && this->vertShader != nullptr) {
		return true; 
	}
	else if (stage & vk::ShaderStageFlagBits::eFragment && this->fragShader != nullptr) {
		return true; 
	}
	return false; 
}

common::Handle VulkanObject::getBaseShader(vk::ShaderStageFlags stage) {
	if (stage & vk::ShaderStageFlagBits::eVertex) {
		return this->vertShaderHandle; 
	}
	else {
		return this->fragShaderHandle; 
	}
}

void VulkanObject::setPipelineLayout(vk::PipelineLayout newPipelineLayout) {
	this->pipelineLayout = newPipelineLayout;
}

vk::PipelineLayout VulkanObject::getPipelineLayout() {
	return this->pipelineLayout;
}

void VulkanObject::addPipeline(std::unique_ptr<StarPipeline> newPipeline) {
	this->starPipeline = std::move(newPipeline);
}

size_t VulkanObject::getNumRenderObjects()
{
	return this->renderObjects.size(); 
}

RenderObject* star::core::VulkanObject::getRenderObjectAt(size_t index) {
	return this->renderObjects.at(index).get();
}

void star::core::VulkanObject::createPipeline(PipelineConfigSettings& configs) {
	this->starPipeline = std::make_unique<StarPipeline>(this->starDevice, this->vertShader, this->fragShader, configs); 
}

}
}