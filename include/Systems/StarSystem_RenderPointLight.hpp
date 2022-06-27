#pragma once 

#include "SC/Light.hpp"
#include "StarSystem_RenderObj.hpp"

#include <iostream>
#include <vector>

namespace star {
namespace core {
	class RenderSysPointLight : private RenderSysObj{
	public:
		struct UniformBufferObject{
			alignas(16) glm::mat4 modelMatrix;
			alignas(16) glm::mat4 normalMatrix;
			uint16_t numLights;
		};

		RenderSysPointLight(const RenderSysObj&) = delete;

		RenderSysPointLight(StarDevice* device, size_t numSwapChainImages, vk::DescriptorSetLayout globalSetLayout,
			vk::Extent2D swapChainExtent, vk::RenderPass renderPass) :
			RenderSysObj(device, numSwapChainImages, globalSetLayout, swapChainExtent, renderPass) { }

		virtual ~RenderSysPointLight();

		virtual void init(std::vector<vk::DescriptorSetLayout> globalSets) override { this->RenderSysObj::init(globalSets); }

		virtual void addLight(common::Light* newLight, common::GameObject* linkedObject, size_t numSwapChainImages);

		virtual void registerShader(vk::ShaderStageFlagBits stage, common::Shader* newShader, common::Handle newShaderHandle) override {
			this->RenderSysObj::registerShader(stage, newShader, newShaderHandle);
		};

		virtual void bind(vk::CommandBuffer& commandBuffer) { this->RenderSysObj::bind(commandBuffer); }

		virtual void updateBuffers(uint32_t currentImage) { this->RenderSysObj::updateBuffers(currentImage); }
		
		virtual void render(vk::CommandBuffer& commandBuffer, int swapChainImageIndex) { this->RenderSysObj::render(commandBuffer, swapChainImageIndex); }

	protected:

	private:
		std::vector<common::Light*> lightList; 


	};
}
}