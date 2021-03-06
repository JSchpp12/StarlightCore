#pragma once 

#include "SC/Shader.h"
#include "Star_Device.hpp"
#include "VulkanVertex.hpp"
#include "Star_Shader.h"
#include "Compiler.h"

#include <vulkan/vulkan.hpp>

#include <iostream> 

namespace star::core{
	struct PipelineConfigSettings {
		PipelineConfigSettings() = default; 
		//no copy 
		PipelineConfigSettings(const PipelineConfigSettings&) = delete;
		PipelineConfigSettings& operator=(const PipelineConfigSettings&) = delete;

		vk::PipelineViewportStateCreateInfo viewportInfo;
		vk::PipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
		vk::PipelineRasterizationStateCreateInfo rasterizationInfo;
		vk::PipelineMultisampleStateCreateInfo multisampleInfo;
		vk::PipelineColorBlendAttachmentState colorBlendAttachment;
		vk::PipelineColorBlendStateCreateInfo colorBlendInfo;
		vk::PipelineDepthStencilStateCreateInfo depthStencilInfo;
		std::vector<VkDynamicState> dynamicStateEnables;
		vk::PipelineDynamicStateCreateInfo dynamicStateInfo;
		vk::PipelineLayout pipelineLayout = nullptr;
		vk::RenderPass renderPass = nullptr;
		uint32_t subpass = 0;
	};

	class StarPipeline {
	public:
		StarPipeline(StarDevice* device, common::Shader& inVertShader, common::Shader& inFragShader, PipelineConfigSettings& configSettings);
		~StarPipeline(); 

		//no copy
		StarPipeline(const StarPipeline&) = delete; 
		StarPipeline& operator=(const StarPipeline&) = delete;

		void bind(vk::CommandBuffer commandBuffer); 

		static void defaultPipelineConfigInfo(PipelineConfigSettings& configInfo, vk::Extent2D swapChainExtent);
	protected:


	private: 
		StarDevice* starDevice; 
		vk::Pipeline graphicsPipeline; 
		vk::ShaderModule vertShaderModule, fragShaderModule; 

		void createGraphicsPipeline(const common::Shader& inVertShader, const common::Shader& inFragShader, PipelineConfigSettings& configSettings); 

		vk::ShaderModule createShaderModule(const std::vector<uint32_t>& sourceCode); 
	};
}