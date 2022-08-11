#include "Star_Pipeline.hpp"

namespace star::core{
	StarPipeline::StarPipeline(StarDevice* device, PipelineConfigSettings& configSettings, common::Shader& inVertShader, common::Shader* inFragShader) :
		starDevice(device) {
        createGraphicsPipeline(inVertShader, inFragShader, configSettings); 
	}

	StarPipeline::~StarPipeline() {
		this->starDevice->getDevice().destroyPipeline(this->graphicsPipeline); 
	}

    void StarPipeline::bind(vk::CommandBuffer commandBuffer) {
        commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, this->graphicsPipeline);
    }

    void StarPipeline::defaultPipelineConfigInfo(PipelineConfigSettings& configInfo, vk::Extent2D swapChainExtent) {
		/* Rasterizer */
		//takes the geometry and creates fragments which are then passed onto the fragment shader 
		//also does: depth testing, face culling, and the scissor test
		vk::PipelineRasterizationStateCreateInfo rasterizer{};
		rasterizer.sType = vk::StructureType::ePipelineRasterizationStateCreateInfo;
		//if set to true -> fragments that are beyond near and far planes are set to those distances rather than being removed
		rasterizer.depthClampEnable = VK_FALSE;

		//polygonMode determines how frags are generated. Different options: 
		//1. VK_POLYGON_MODE_FILL: fill the area of the polygon with fragments
		//2. VK_POLYGON_MODE_LINE: polygon edges are drawn as lines 
		//3. VK_POLYGON_MODE_POINT: polygon verticies are drawn as points
		//NOTE: using any other than fill, requires GPU feature
		rasterizer.polygonMode = vk::PolygonMode::eFill;

		//available line widths, depend on GPU. If above 1.0f, required wideLines GPU feature
		rasterizer.lineWidth = 1.0f; //measured in fragment widths

		//cullMode : type of face culling to use.
		rasterizer.cullMode = vk::CullModeFlagBits::eNone;
		rasterizer.frontFace = vk::FrontFace::eCounterClockwise;

		//depth values can be used in way that is known as 'shadow mapping'. 
		//rasterizer is capable of changing depth values through constant addition or biasing based on frags slope 
		//this is left as off for now 
		rasterizer.depthBiasEnable = VK_FALSE;
		rasterizer.depthBiasConstantFactor = 0.0f; //optional 
		rasterizer.depthBiasClamp = 0.0f; //optional 
		rasterizer.depthBiasSlopeFactor = 0.0f; //optional

		/* Multisampling */
		//this is one of the methods of performing anti-aliasing
		//enabling requires GPU feature -- left off for this tutorial 
		vk::PipelineMultisampleStateCreateInfo multisampling{};
		multisampling.sType = vk::StructureType::ePipelineMultisampleStateCreateInfo;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = vk::SampleCountFlagBits::e1;
		multisampling.minSampleShading = 1.0f; //optional 
		multisampling.pSampleMask = nullptr; //optional
		multisampling.alphaToCoverageEnable = VK_FALSE; //optional
		multisampling.alphaToOneEnable = VK_FALSE; //optional

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

		/* Color blending */
		// after the fragShader has returned a color, it must be combined with the color already in the framebuffer
		// there are two ways to do this: 
		//      1. mix the old and new value to produce final color
		//      2. combine the old a new value using a bitwise operation 
		//two structs are needed to create this functionality: 
		//  1. VkPipelineColorBlendAttachmentState: configuration per attached framebuffer 
		//  2. VkPipelineColorBlendStateCreateInfo: global configuration
		//only using one framebuffer in this project -- both of these are disabled in this project
		vk::PipelineColorBlendAttachmentState colorBlendAttachment{};
		//colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
		colorBlendAttachment.blendEnable = VK_FALSE;

		vk::PipelineColorBlendStateCreateInfo colorBlending{};
		colorBlending.sType = vk::StructureType::ePipelineColorBlendStateCreateInfo;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = vk::LogicOp::eCopy;
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;
		colorBlending.blendConstants[0] = 0.0f;
		colorBlending.blendConstants[1] = 0.0f;
		colorBlending.blendConstants[2] = 0.0f;
		colorBlending.blendConstants[3] = 0.0f;
		
		configInfo.rasterizationInfo = rasterizer;
		configInfo.multisampleInfo = multisampling;
		configInfo.depthStencilInfo = depthStencil;
		configInfo.colorBlendInfo = colorBlending;
		configInfo.colorBlendAttachment = colorBlendAttachment;
    }

	void StarPipeline::createGraphicsPipeline(const common::Shader& inVertShader, common::Shader* inFragShader, PipelineConfigSettings& configSettings) {
        assert(configSettings.pipelineLayout && "Pipeline layout must be defined"); 
		assert(configSettings.viewportInfo.viewportCount != 0 && configSettings.viewportInfo.pViewports && "Pipeline must have viewport defined");
		std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;

		Shader vertShader = Shader(inVertShader);\
		this->vertShaderModule = createShaderModule(*vertShader.compiledCode);
		if (inFragShader != nullptr) {
			Shader fragShader = Shader(*inFragShader);
			this->fragShaderModule = createShaderModule(*fragShader.compiledCode);
		}

        auto bindingDescriptions = VulkanVertex::getBindingDescription();
        auto attributeDescriptions = VulkanVertex::getAttributeDescriptions();

        //TODO: move this out of here
        vk::PipelineShaderStageCreateInfo vertShaderStageInfo{};
        vertShaderStageInfo.sType = vk::StructureType::ePipelineShaderStageCreateInfo;
        vertShaderStageInfo.stage = vk::ShaderStageFlagBits::eVertex;
        vertShaderStageInfo.module = this->vertShaderModule;
        vertShaderStageInfo.pName = "main"; //the function to invoke in the shader module

		shaderStages.push_back(vertShaderStageInfo);

		if (fragShaderModule) {
			vk::PipelineShaderStageCreateInfo fragShaderStageInfo{};
			fragShaderStageInfo.sType = vk::StructureType::ePipelineShaderStageCreateInfo;
			fragShaderStageInfo.stage = vk::ShaderStageFlagBits::eFragment;
			fragShaderStageInfo.module = this->fragShaderModule;
			fragShaderStageInfo.pName = "main";
			shaderStages.push_back(fragShaderStageInfo);
		}

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
        pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
        pipelineInfo.pStages = shaderStages.data();
        //ref all previously created structs
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &configSettings.viewportInfo; 
        pipelineInfo.pRasterizationState = &configSettings.rasterizationInfo;
        pipelineInfo.pMultisampleState = &configSettings.multisampleInfo;
        pipelineInfo.pDepthStencilState = &configSettings.depthStencilInfo;
        pipelineInfo.pColorBlendState = &configSettings.colorBlendInfo;
		pipelineInfo.pDynamicState = &dynamicStateInfo; 
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