#include "VulkanObject.h"
#include "VulkanObject.h"

namespace star {
namespace core {

void VulkanObject::cleanup() {
	//for (size_t i = 0; i < this->renderObjects.size(); i++) {
	//	this->renderObjects.at(i).release(); 
	//}
}

void VulkanObject::registerShader(vk::ShaderStageFlagBits stage, common::Handle newShaderHandle) {
	this->shaderContainer.insert(std::pair<vk::ShaderStageFlagBits, common::Handle>(stage, newShaderHandle));
}

void VulkanObject::addObject(common::Handle newObjectHandle, common::LogicalObject* newObject, size_t numSwapChainImages) {
	auto numIndicies = newObject->getIndicies()->size();
	auto numVerticies = newObject->getVerticies()->size();

	this->totalNumIndicies += numIndicies;
	this->totalNumVerticies += numVerticies;

	this->renderObjects.push_back(std::move(RenderObject::Builder().setFromObject(newObjectHandle, newObject).setNumFrames(numSwapChainImages).build())); 
}

common::Handle VulkanObject::getBaseShader(vk::ShaderStageFlagBits stage) {
	auto found = this->shaderContainer.find(stage);
	assert(found != this->shaderContainer.end());

	return found->second;
}

void VulkanObject::registerShaderModule(vk::ShaderStageFlagBits stage, vk::ShaderModule newShaderModule) {
	this->shaderModuleContainer.insert(std::pair<vk::ShaderStageFlagBits, vk::ShaderModule>(stage, newShaderModule));
}

vk::ShaderModule VulkanObject::getShaderModule(vk::ShaderStageFlagBits stage) {
	auto found = this->shaderModuleContainer.find(stage);
	assert(found != this->shaderModuleContainer.end());

	return found->second;
}

bool VulkanObject::hasShader(vk::ShaderStageFlagBits stage) {
	auto found = this->shaderContainer.find(stage);
	if (found != this->shaderContainer.end()) {
		return true;
	}

	return false;
}

void VulkanObject::setPipelineLayout(vk::PipelineLayout newPipelineLayout) {
	this->pipelineLayout = newPipelineLayout;
}

vk::PipelineLayout VulkanObject::getPipelineLayout() {
	return this->pipelineLayout;
}

void VulkanObject::addPipelines(std::vector<vk::Pipeline> newPipeline) {
	this->pipelines = newPipeline;
}

size_t VulkanObject::getNumRenderObjects()
{
	return this->renderObjects.size(); 
}

RenderObject* star::core::VulkanObject::getRenderObjectAt(size_t index) {
	return this->renderObjects.at(index).get();
}

}
}