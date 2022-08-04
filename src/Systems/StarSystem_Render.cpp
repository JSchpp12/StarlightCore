#include "StarSystem_Render.hpp"

namespace star::core {
	RenderSystem::RenderSystem(StarDevice& starDevice) : starDevice(starDevice) { }

	void RenderSystem::init(std::vector<vk::DescriptorSetLayout> globalDescriptorSets) { 
		createVertexBuffer();
		createIndexBuffer();
		createRenderBuffers();
		createDescriptorPool();
		createDescriptorLayouts();
		createDescriptors();
		if (!this->pipelineLayout)
			createPipelineLayout(globalDescriptorSets);
		createPipeline();
		checkRenderRequirenments();
	}

	void RenderSystem::bind(vk::CommandBuffer& commandBuffer) {
		this->starPipeline->bind(commandBuffer);
	}

	void RenderSystem::registerShader(vk::ShaderStageFlagBits stage, common::Shader& newShader, common::Handle newShaderHandle) {
		if (stage & vk::ShaderStageFlagBits::eVertex) {
			this->vertShader = &newShader;
			this->vertShaderHandle = newShaderHandle;
		}
		else {
			this->fragShader = &newShader;
			this->fragShaderHandle = newShaderHandle;
		}
	}

	void RenderSystem::addObject(std::unique_ptr<RenderObject> newRenderObject) {
		this->numMeshes += newRenderObject->getMeshes().size();
		for (auto& mesh : newRenderObject->getGameObject().getMeshes()) {
			this->totalNumVerticies += mesh->getTriangles()->size() * 3;
		}
		this->renderObjects.push_back(std::move(newRenderObject));
	}

	bool RenderSystem::hasShader(vk::ShaderStageFlagBits stage) {
		if (stage & vk::ShaderStageFlagBits::eVertex && this->vertShader != nullptr) {
			return true;
		}
		else if (stage & vk::ShaderStageFlagBits::eFragment && this->fragShader != nullptr) {
			return true;
		}
		return false;
	}

	common::Handle RenderSystem::getBaseShader(vk::ShaderStageFlags stage) {
		if (stage & vk::ShaderStageFlagBits::eVertex) {
			return this->vertShaderHandle;
		}
		else {
			return this->fragShaderHandle;
		}
	}

	void RenderSystem::createVertexBuffer() {
		vk::DeviceSize bufferSize;

		//TODO: ensure that more objects can be drawn 
		common::GameObject* currObject = nullptr;
		std::vector<common::Vertex> vertexList(this->totalNumVerticies);
		size_t vertexCounter = 0;

		for (auto& object : this->renderObjects) {
			const std::vector<std::unique_ptr<common::Mesh>>& currObjectMeshes = object->getGameObject().getMeshes();

			//copy verticies from the render object into the total vertex list for the vulkan object
			for (size_t i = 0; i < currObjectMeshes.size(); i++) {
				auto& triangles = currObjectMeshes.at(i)->getTriangles();

				for (size_t j = 0; j < triangles->size(); j++) {
					for (int k = 0; k < 3; k++) {
						vertexList.at(vertexCounter) = triangles->at(j).verticies[k];
						vertexCounter++;
					}
				}
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

		this->starDevice.copyBuffer(stagingBuffer.getBuffer(), this->vertexBuffer->getBuffer(), bufferSize);
	}

	void RenderSystem::createIndexBuffer() {
		std::vector<uint32_t> indiciesList(this->totalNumVerticies);
		RenderObject* currRenderObject = nullptr;
		common::GameObject* currObject = nullptr;
		size_t indexCounter = 0; //used to keep track of index offsets 

		for (auto& object : this->renderObjects) {
			for (size_t j = 0; j < object->getGameObject().getMeshes().size(); j++) {
				auto& mesh = object->getGameObject().getMeshes().at(j);
				for (size_t k = 0; k < mesh->getTriangles()->size(); k++) {
					indiciesList.at(indexCounter + 0) = indexCounter + 0;
					indiciesList.at(indexCounter + 1) = indexCounter + 1;
					indiciesList.at(indexCounter + 2) = indexCounter + 2;
					indexCounter += 3;
				}
			}
		}

		vk::DeviceSize bufferSize = sizeof(indiciesList.at(0)) * indiciesList.size();
		uint32_t indexSize = sizeof(indiciesList[0]);
		uint32_t indexCount = indiciesList.size();

		StarBuffer stagingBuffer{
			this->starDevice,
			indexSize,
			indexCount,
			vk::BufferUsageFlagBits::eTransferSrc,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
		};
		stagingBuffer.map();
		stagingBuffer.writeToBuffer(indiciesList.data());

		this->indexBuffer = std::make_unique<StarBuffer>(
			this->starDevice,
			indexSize,
			indexCount,
			vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst,
			vk::MemoryPropertyFlagBits::eDeviceLocal);
		this->starDevice.copyBuffer(stagingBuffer.getBuffer(), this->indexBuffer->getBuffer(), bufferSize);
	}

	void RenderSystem::checkRenderRequirenments() {
		assert(starPipeline && "The pipeline to be used by this rendering system was never created");
	}
}