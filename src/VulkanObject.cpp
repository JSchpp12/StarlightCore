#include "VulkanObject.h"


vk::Device star::core::VulkanObject::device; 
bool star::core::VulkanObject::deviceRegistered = false; 
size_t star::core::VulkanObject::numInFlightImages; 

star::core::VulkanObject::VulkanObject()
{ 
	this->perImageData.resize(numInFlightImages); 
}

void star::core::VulkanObject::registerVulkanRuntimeInfo(vk::Device newDevice, size_t numImages) {
	device = newDevice; 
	deviceRegistered = true; 
}

void star::core::VulkanObject::registerShader(vk::ShaderStageFlagBits stage, common::Handle newShaderHandle) {
	this->shaderContainer.insert(std::pair<vk::ShaderStageFlagBits, common::Handle>(stage, newShaderHandle)); 
}

void star::core::VulkanObject::addObject(common::Handle newObjectHandle, common::Object* newObject) {
	this->baseObjectHandles.push_back(newObjectHandle); 
	auto numIndicies = newObject->getIndicies()->size(); 
	auto numVerticies = newObject->getVerticies()->size(); 

	this->totalNumIndicies += numIndicies;
	this->numIndiciesPerObject.push_back(numIndicies); 
	this->totalNumVerticies += numVerticies; 
	this->numVerticiesPerObject.push_back(numVerticies);
}

star::common::Handle star::core::VulkanObject::getBaseShader(vk::ShaderStageFlagBits stage) {
	auto found = this->shaderContainer.find(stage); 
	assert(found != this->shaderContainer.end()); 

	return found->second; 
}

void star::core::VulkanObject::registerShaderModule(vk::ShaderStageFlagBits stage, vk::ShaderModule newShaderModule) {
	this->shaderModuleContainer.insert(std::pair<vk::ShaderStageFlagBits, vk::ShaderModule>(stage, newShaderModule));
}

vk::ShaderModule star::core::VulkanObject::getShaderModule(vk::ShaderStageFlagBits stage) {
	auto found = this->shaderModuleContainer.find(stage); 
	assert(found != this->shaderModuleContainer.end()); 

	return found->second; 
}

bool star::core::VulkanObject::hasShader(vk::ShaderStageFlagBits stage) {
	auto found = this->shaderContainer.find(stage); 
	if (found != this->shaderContainer.end()) {
		return true; 
	}

	return false; 
}

void star::core::VulkanObject::setPipelineLayout(vk::PipelineLayout newPipelineLayout) {
	this->pipelineLayout = newPipelineLayout; 
}

vk::PipelineLayout star::core::VulkanObject::getPipelineLayout() {
	return this->pipelineLayout;
}

void star::core::VulkanObject::addPipelines(std::vector<vk::Pipeline> newPipeline) {
	this->pipelines = newPipeline; 
}

size_t star::core::VulkanObject::getNumRenderObjects() {
	return this->baseObjectHandles.size(); 
}

star::common::Handle star::core::VulkanObject::getObjectHandleAt(const size_t& index) {
	return this->baseObjectHandles.at(index); 
}

std::vector<star::common::Handle> star::core::VulkanObject::getRenderObjectList() {
	return this->baseObjectHandles;
}

uint32_t star::core::VulkanObject::getNumIndiciesForRenderObjectAt(size_t index) {
	return this->numIndiciesPerObject.at(index); 
}
 

//
//void star::core::VulkanObject::createVertexBuffer(std::vector<common::Object*>& objects) {
//	uint64_t indexCount = 0;
//
//	vk::DeviceSize bufferSize; 
//	vk::Buffer stagingBuffer; 
//	vk::DeviceMemory stagingBufferMemory; 
//
//	for (auto& object : objects) {
//
//	}
//}