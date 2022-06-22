#include "Star_Pipeline.hpp"

namespace star {
namespace core {
	StarPipeline::StarPipeline(StarDevice* device, common::Shader* vertShader, common::Shader* fragShader, PipelineConfigSettings& configSettings) :
		starDevice(device) {
        createGraphicsPipeline(vertShader, fragShader, configSettings); 
	}
	
	StarPipeline::~StarPipeline() {
		this->starDevice->getDevice().destroyPipeline(this->graphicsPipeline); 
	}

	void StarPipeline::createGraphicsPipeline(common::Shader* vertShader, common::Shader* fragShader, PipelineConfigSettings& configSettings) {
		//std::vector<uint32_t> vertShaderCode = vertShader->GetSpirV();
		this->vertShaderModule = createShaderModule(vertShader->GetSpirV());
		this->fragShaderModule = createShaderModule(fragShader->GetSpirV());

        auto bindingDescriptions = VulkanVertex::getBindingDescription();
        auto attributeDescriptions = VulkanVertex::getAttributeDescriptions();

        //TODO: move this out of here
        vk::PipelineShaderStageCreateInfo vertShaderStageInfo{};
        vertShaderStageInfo.sType = vk::StructureType::ePipelineShaderStageCreateInfo;
        vertShaderStageInfo.stage = vk::ShaderStageFlagBits::eVertex;
        vertShaderStageInfo.module = this->vertShaderModule;
        vertShaderStageInfo.pName = "main"; //the function to invoke in the shader module

        vk::PipelineShaderStageCreateInfo fragShaderStageInfo{};
        fragShaderStageInfo.sType = vk::StructureType::ePipelineShaderStageCreateInfo;
        fragShaderStageInfo.stage = vk::ShaderStageFlagBits::eFragment;
        fragShaderStageInfo.module = this->fragShaderModule; 
        fragShaderStageInfo.pName = "main";

        //store these creation infos for later use 
        vk::PipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

        vk::PipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = vk::StructureType::ePipelineVertexInputStateCreateInfo;
        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.pVertexBindingDescriptions = &bindingDescriptions;
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
        vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

        vk::PipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType = vk::StructureType::ePipelineInputAssemblyStateCreateInfo;
        inputAssembly.topology = vk::PrimitiveTopology::eTriangleList;
        inputAssembly.primitiveRestartEnable = VK_FALSE;


        /* Depth and Stencil Testing */
        //if using depth or stencil buffer, a depth and stencil tests are neeeded
        vk::PipelineDepthStencilStateCreateInfo depthStencil{};
        depthStencil.sType = vk::StructureType::ePipelineDepthStencilStateCreateInfo;
        depthStencil.depthTestEnable = VK_TRUE;             //specifies if depth of new fragments should be compared to the depth buffer to test for actual display state
        depthStencil.depthWriteEnable = VK_TRUE;            //specifies if the new depth of fragments that pass the depth tests should be written to the depth buffer 
        depthStencil.depthCompareOp = vk::CompareOp::eLess;   //comparison that is performed to keep or discard fragments - here this is: lower depth = closer, so depth of new frags should be less
        //following are for optional depth bound testing - keep frags within a specific range 
        depthStencil.depthBoundsTestEnable = VK_FALSE;
        depthStencil.minDepthBounds = 0.0f;                 //optional 
        depthStencil.maxDepthBounds = 1.0f;                 //optional
        //following are used for stencil tests - make sure that format of depth image contains a stencil component
        depthStencil.stencilTestEnable = VK_FALSE;


        /* Dynamic State */
        //some parts of the pipeline can be changed without recreating the entire pipeline
        //if this is defined, the data for the dynamic structures will have to be provided at draw time
        vk::DynamicState dynamicStates[] = {
            vk::DynamicState::eViewport,
            vk::DynamicState::eLineWidth
        };
        vk::PipelineDynamicStateCreateInfo dynamicStateInfo{};
        dynamicStateInfo.sType = vk::StructureType::ePipelineDynamicStateCreateInfo;
        dynamicStateInfo.dynamicStateCount = 2;
        dynamicStateInfo.pDynamicStates = dynamicStates;

        /* Pipeline */
        vk::GraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = vk::StructureType::eGraphicsPipelineCreateInfo;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shaderStages;
        //ref all previously created structs
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &configSettings.viewportInfo; 
        pipelineInfo.pRasterizationState = &configSettings.rasterizationInfo;
        pipelineInfo.pMultisampleState = &configSettings.multisampleInfo;
        pipelineInfo.pDepthStencilState = &configSettings.depthStencilInfo;
        pipelineInfo.pColorBlendState = &configSettings.colorBlendInfo;
        pipelineInfo.pDynamicState = nullptr; // Optional
        pipelineInfo.layout = configSettings.pipelineLayout;
        //render pass info - ensure renderpass is compatible with pipeline --check khronos docs
        pipelineInfo.renderPass = configSettings.renderPass;
        pipelineInfo.subpass = 0; //index where the graphics pipeline will be used 
        //allow switching to new pipeline (inheritance) 
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional -- handle to existing pipeline that is being switched to
        pipelineInfo.basePipelineIndex = -1; // Optional

        //finally creating the pipeline -- this call has the capability of creating multiple pipelines in one call
        //2nd arg is set to null -> normally for graphics pipeline cache (can be used to store and reuse data relevant to pipeline creation across multiple calls to vkCreateGraphicsPipeline)

        //this->graphicsPipeline = this->starDevice->getDevice().get().createGraphicsPipeline(VK_NULL_HANDLE, pipelineInfo); 
        auto result = this->starDevice->getDevice().createGraphicsPipelines(VK_NULL_HANDLE, pipelineInfo);
        if (result.result != vk::Result::eSuccess) {
            throw std::runtime_error("failed to create graphics pipeline");
        }
        if (result.value.size() > 1) {
            throw std::runtime_error("unknown error has occurred, more than one pipeline was created ");
        }
        this->graphicsPipeline = result.value.at(0);

        //destroy the shader modules that were created 
        this->starDevice->getDevice().destroyShaderModule(this->vertShaderModule);
        this->starDevice->getDevice().destroyShaderModule(this->fragShaderModule);
	}

	vk::ShaderModule StarPipeline::createShaderModule(const std::vector<uint32_t>& sourceCode) {
		vk::ShaderModuleCreateInfo createInfo{};
		createInfo.sType = vk::StructureType::eShaderModuleCreateInfo;
		createInfo.codeSize = 4 * sourceCode.size();
		createInfo.pCode = sourceCode.data();

		VkShaderModule shaderModule = this->starDevice->getDevice().createShaderModule(createInfo);
		if (!shaderModule) {
			throw std::runtime_error("failed to create shader module");
		}
        return shaderModule;
	}
}
}