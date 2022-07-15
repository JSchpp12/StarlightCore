#include "StarSystem_RenderPointLight.hpp"

namespace star::core {
	RenderSysPointLight::~RenderSysPointLight() {}

	void RenderSysPointLight::addLight(common::Light* newLight, std::unique_ptr<RenderObject> linkedRenderObject, size_t numSwapChainImages) {
		this->lightList.push_back(newLight);

		this->RenderSysObj::addObject(std::move(linkedRenderObject));
	}

	void RenderSysPointLight::updateBuffers(uint32_t currentImage) {
		UniformBufferObject newBufferObject{};
		std::vector<UniformBufferObject> bufferObjects(this->renderObjects.size());
		auto deviceProperties = this->starDevice->getPhysicalDevice().getProperties();

		auto minAlignmentOfUBOElements = StarBuffer::getAlignment(sizeof(UniformBufferObject), deviceProperties.limits.minUniformBufferOffsetAlignment);

		for (size_t i = 0; i < this->renderObjects.size(); i++) {
			newBufferObject.modelMatrix = this->renderObjects.at(i)->getGameObject().getDisplayMatrix();
			newBufferObject.normalMatrix = this->renderObjects.at(i)->getGameObject().getNormalMatrix();
			newBufferObject.color = this->lightList.at(i)->getColor();

			this->uniformBuffers[currentImage]->writeToBuffer(&newBufferObject, sizeof(UniformBufferObject), minAlignmentOfUBOElements * i);
		}
	}

	void RenderSysPointLight::createRenderBuffers() {
		this->uniformBuffers.resize(this->numSwapChainImages);
		auto test = sizeof(UniformBufferObject);
		auto minUniformSize = this->starDevice->getPhysicalDevice().getProperties().limits.minUniformBufferOffsetAlignment;
		for (size_t i = 0; i < numSwapChainImages; i++) {
			this->uniformBuffers[i] = std::make_unique<StarBuffer>(*this->starDevice, this->renderObjects.size(), sizeof(UniformBufferObject),
				vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, minUniformSize);
			this->uniformBuffers[i]->map();
		}
	}

	void RenderSysPointLight::createDescriptors() {
		this->descriptorSetLayout = StarDescriptorSetLayout::Builder(*this->starDevice)
			.addBinding(0, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex)
			.build();
			
		auto deviceProperties = this->starDevice->getPhysicalDevice().getProperties();
		auto minAlignmentOfUBOElements = StarBuffer::getAlignment(sizeof(UniformBufferObject), deviceProperties.limits.minUniformBufferOffsetAlignment);

		//create descritptor sets 
		vk::DescriptorBufferInfo bufferInfo{};
		for (int i = 0; i < this->numSwapChainImages; i++) {
			for (int j = 0; j < this->renderObjects.size(); j++) {

				bufferInfo = vk::DescriptorBufferInfo{
					this->uniformBuffers[i]->getBuffer(),
					minAlignmentOfUBOElements * j,
					sizeof(UniformBufferObject)
				};

				StarDescriptorWriter(*this->starDevice, *this->descriptorSetLayout, *this->descriptorPool)
					.writeBuffer(0, &bufferInfo)
					.build(this->renderObjects.at(j)->getDefaultDescriptorSets().at(i));
			}
		}
	}
}