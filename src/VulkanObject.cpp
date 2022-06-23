#include "VulkanObject.h"

namespace star {
namespace core {

VulkanObject::~VulkanObject() {
    this->starDevice->getDevice().destroyPipelineLayout(this->pipelineLayout);
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

void star::core::VulkanObject::render(vk::CommandBuffer& commandBuffer) {
    this->starPipeline->bind(commandBuffer); 

    vk::DeviceSize offsets{}; 
    commandBuffer.bindVertexBuffers(0, this->vertexBuffer->getBuffer(), offsets);

    commandBuffer.bindIndexBuffer(this->indexBuffer->getBuffer(), 0, vk::IndexType::eUint32);
}

void star::core::VulkanObject::init() {
	//create needed buffers 
    createVertexBuffer(); 
    createIndexBuffer();
	//object buffer 

}

void VulkanObject::createVertexBuffer() {
    vk::DeviceSize bufferSize;

    //TODO: ensure that more objects can be drawn 
    common::GameObject* currObject = nullptr;
    std::vector<common::Vertex>* currObjectVerticies = nullptr; 
    std::vector<common::Vertex> vertexList(this->totalNumVerticies);
    size_t vertexCounter = 0;

    for (auto& object : this->renderObjects) {
        //go through all objects in object list and generate the vertex indicies -- only works with 1 object at the moment 
        currObjectVerticies = object->getGameObject()->getVerticies();

        //copy verticies from the render object into the total vertex list for the vulkan object
        for (size_t j = 0; j < currObjectVerticies->size(); j++) {
            vertexList.at(vertexCounter) = currObjectVerticies->at(j);
            vertexCounter++;
        }
    }

    bufferSize = sizeof(vertexList.at(0)) * vertexList.size();
	uint32_t vertexSize = sizeof(vertexList[0]);
	uint32_t vertexCount = vertexList.size(); 

    StarBuffer stagingBuffer{
        this->starDevice,
		vertexSize,
        vertexCount, 
        vk::BufferUsageFlagBits::eTransferSrc,
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
    }; 
    stagingBuffer.map();
    stagingBuffer.writeToBuffer(vertexList.data()); 

    this->vertexBuffer = std::make_unique<StarBuffer>(
        this->starDevice,
		vertexSize,
        vertexCount,
        vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst, 
		vk::MemoryPropertyFlagBits::eDeviceLocal);

	this->starDevice->copyBuffer(stagingBuffer.getBuffer(), this->vertexBuffer->getBuffer(), bufferSize); 
}

void VulkanObject::createIndexBuffer() {
    vk::DeviceSize bufferSize;

    //TODO: will only support one object at the moment
    std::vector<uint32_t> indiciesList(this->totalNumIndicies);
    std::vector<uint32_t>* currRenderObjectIndicies = nullptr;
    RenderObject* currRenderObject = nullptr; 
    common::GameObject* currObject = nullptr;
    size_t indexCounter = 0; //used to keep track of index offsets 

    for (size_t i = 0; i < this->renderObjects.size(); i++) {
        currRenderObject = this->renderObjects.at(i).get();
        currRenderObjectIndicies = currRenderObject->getGameObject()->getIndicies();

        for (size_t j = 0; j < currRenderObjectIndicies->size(); j++) {
            if (i > 0) {
                //not the first object 
                //displace the index counter by the number of indicies previously used
                indiciesList.at(indexCounter) = indexCounter + currRenderObjectIndicies->at(j);
            }
            else {
                indiciesList.at(indexCounter) = indexCounter;
            }

            indexCounter++;
        }
    }

    bufferSize = sizeof(indiciesList.at(0)) * indiciesList.size();
    uint32_t indexSize = sizeof(indiciesList[0]);

    StarBuffer stagingBuffer{
        this->starDevice, 
        indexSize, 
        this->totalNumIndicies, 
        vk::BufferUsageFlagBits::eTransferSrc, 
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
    };
    stagingBuffer.map(); 
    stagingBuffer.writeToBuffer(indiciesList.data()); 

    this->indexBuffer = std::make_unique<StarBuffer>(
        this->starDevice, 
        indexSize, 
        this->totalNumIndicies, 
        vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst,
        vk::MemoryPropertyFlagBits::eDeviceLocal );
    this->starDevice->copyBuffer(stagingBuffer.getBuffer(), this->indexBuffer->getBuffer(), bufferSize);
}

}
}