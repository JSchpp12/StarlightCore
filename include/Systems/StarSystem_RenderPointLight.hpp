#pragma once 

#include "SC/Light.hpp"
#include "StarSystem_RenderObj.hpp"
#include "Star_RenderObject.hpp"
#include "Star_RenderObject.hpp"

#include <iostream>
#include <vector>
#include <memory>

namespace star::core{
	class RenderSysPointLight : private RenderSysObj{
	public:
		RenderSysPointLight(const RenderSysObj&) = delete;

		RenderSysPointLight(StarDevice& device, size_t numSwapChainImages, vk::DescriptorSetLayout globalSetLayout,
			vk::Extent2D swapChainExtent, vk::RenderPass renderPass) :
			RenderSysObj(device, numSwapChainImages, globalSetLayout, swapChainExtent, renderPass) { }

		virtual ~RenderSysPointLight();

		virtual void init(std::vector<vk::DescriptorSetLayout> globalSets) override { this->RenderSysObj::init(globalSets); }

		virtual void addLight(common::Light* newLight, std::unique_ptr<RenderObject> linkedRenderObject, size_t numSwapChainImages);

		virtual void registerShader(vk::ShaderStageFlagBits stage, common::Shader& newShader, common::Handle newShaderHandle) override {
			this->RenderSysObj::registerShader(stage, newShader, newShaderHandle);
		};

		virtual void bind(vk::CommandBuffer& commandBuffer) { this->RenderSysObj::bind(commandBuffer); }

		virtual void updateBuffers(uint32_t currentImage) override; 
		
		virtual void render(vk::CommandBuffer& commandBuffer, int swapChainImageIndex) { this->RenderSysObj::render(commandBuffer, swapChainImageIndex); }

		virtual void setPipelineLayout(vk::PipelineLayout newPipelineLayout) { this->RenderSysObj::setPipelineLayout(newPipelineLayout); }

		virtual bool hasShader(vk::ShaderStageFlagBits stage) { return this->RenderSysObj::hasShader(stage); }

		virtual common::Handle getBaseShader(vk::ShaderStageFlags stage) { return this->RenderSysObj::getBaseShader(stage); }
	protected:

	private:
		std::vector<common::Light*> lightList; 

		//might be able to change this to be a template method
		virtual void createRenderBuffers() override; 

		virtual void createDescriptors() override; 

	};
}