#include "StarSystem_RenderShadows.hpp"

namespace star::core {
	RenderSys_Shadows::~RenderSys_Shadows() {

	}

	void RenderSys_Shadows::render(vk::CommandBuffer& commandBuffer, int swapChainImageIndex) {
		vk::DeviceSize offsets{};
		commandBuffer.bindVertexBuffers(0, this->vertexBuffer->getBuffer(), offsets);

		commandBuffer.bindIndexBuffer(this->indexBuffer->getBuffer(), 0, vk::IndexType::eUint32);

		int vertexCount = 0;
		RenderObject* currRenderObject = nullptr;
		for (size_t i = 0; i < this->renderObjects.size(); i++) {
			//TODO: move per gameobject binding to the renderObjects class 
			commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, this->pipelineLayout, 1, 1, &this->renderObjects.at(i)->getDefaultDescriptorSets().at(swapChainImageIndex), 0, nullptr);
			this->renderObjects.at(i)->render(commandBuffer, this->pipelineLayout, swapChainImageIndex);
		}
	}

	void RenderSys_Shadows::updateBuffers(uint32_t currentImage) {
		UniformBufferObject newBufferObject{}; 

		//write per object info 
		auto minProp = this->starDevice.getPhysicalDevice().getProperties().limits.minUniformBufferOffsetAlignment; 
		auto minAlignmentOfUBOElements = StarBuffer::getAlignment(sizeof(UniformBufferObject), minProp);
		for (int i = 0; i < this->renderObjects.size(); i++) {
			newBufferObject.modelMatrix = this->renderObjects.at(i)->getGameObject().getDisplayMatrix(); 

			uniformBuffers[currentImage]->writeToBuffer(&newBufferObject, sizeof(UniformBufferObject), minAlignmentOfUBOElements * i);
		}

		//write light info 
		LightPositionBufferObject lightInfo{}; 

		auto minAlignmentOfLightElements = StarBuffer::getAlignment(sizeof(LightPositionBufferObject), minProp); 
		for (int i = 0; i < this->lights.size(); i++) {
			lightInfo.viewMatrix = this->lights.at(i)->getViewMatrix(); 
			lightInfoBuffers[currentImage]->writeToBuffer(&lightInfo, sizeof(LightPositionBufferObject), minAlignmentOfLightElements * i);
		}
	}

	void RenderSys_Shadows::createRenderBuffers() {
		uniformBuffers.resize(this->numSwapChainImages); 
		lightInfoBuffers.resize(this->numSwapChainImages);

		auto minUniformSize = this->starDevice.getPhysicalDevice().getProperties().limits.minUniformBufferOffsetAlignment; 
		for (size_t i = 0; i < numSwapChainImages; i++) {
			this->uniformBuffers[i] = std::make_unique<StarBuffer>(this->starDevice, sizeof(UniformBufferObject), this->renderObjects.size(),
				vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, minUniformSize);
			this->uniformBuffers[i]->map();

			lightInfoBuffers[i] = std::make_unique<StarBuffer>(this->starDevice, sizeof(LightPositionBufferObject), 1,
				vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, minUniformSize);
			lightInfoBuffers[i]->map();
		}
	}

	void RenderSys_Shadows::createDescriptorPool() {
		//create descriptor pools 
		this->descriptorPool = StarDescriptorPool::Builder(this->starDevice)
			.setMaxSets(50)																								//allocate large number of  descriptor sets
			.addPoolSize(vk::DescriptorType::eUniformBuffer, this->numSwapChainImages * this->renderObjects.size())
			.addPoolSize(vk::DescriptorType::eUniformBuffer, this->numSwapChainImages * this->lights.size() * this->renderObjects.size())			//right now this should only be 1
			.build();
	}

	void RenderSys_Shadows::createDescriptorLayouts() {
		descriptorSetLayout = StarDescriptorSetLayout::Builder(this->starDevice)
			.addBinding(0, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex)
			.addBinding(1, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex)
			.build(); 
	}

	void RenderSys_Shadows::createDescriptors() {
		vk::DescriptorBufferInfo bufferInfo{}; 
		vk::DescriptorBufferInfo lightInfo{}; 

		lightDescriptors.resize(this->numSwapChainImages); 

		auto minProp = this->starDevice.getPhysicalDevice().getProperties().limits.minUniformBufferOffsetAlignment;
		auto minAlignmentOfUBOElements = StarBuffer::getAlignment(sizeof(UniformBufferObject), minProp);

		//only dealing with 1 light source for now 
		for (int i = 0; i < this->numSwapChainImages; i++) {
			auto descriptorWriter = StarDescriptorWriter(this->starDevice, *descriptorSetLayout, *descriptorPool); 

			lightInfo = vk::DescriptorBufferInfo{
				lightInfoBuffers[i]->getBuffer(),
				0,
				sizeof(LightPositionBufferObject)
			};
			descriptorWriter.writeBuffer(1, &lightInfo);

			for (int j = 0; j < this->renderObjects.size(); j++) {
				bufferInfo = vk::DescriptorBufferInfo{
					uniformBuffers[i]->getBuffer(),
					minAlignmentOfUBOElements * j,
					sizeof(UniformBufferObject)
				};

				if (!descriptorWriter
					.writeBuffer(0, &bufferInfo)
					.build(this->renderObjects.at(j)->getDefaultDescriptorSets().at(i))) {
					throw std::runtime_error("Failed to create descriptors for object");
				}
			}
		}
	}

	void RenderSys_Shadows::createPipelineLayout(std::vector<vk::DescriptorSetLayout> globalDescriptorSets) {
		globalDescriptorSets.push_back(descriptorSetLayout->getDescriptorSetLayout()); 

		vk::PipelineLayoutCreateInfo pipelineLayoutInfo{}; 
		pipelineLayoutInfo.sType = vk::StructureType::ePipelineLayoutCreateInfo; 
		pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(globalDescriptorSets.size());
		pipelineLayoutInfo.pSetLayouts = globalDescriptorSets.data(); 
		pipelineLayoutInfo.pushConstantRangeCount = 0;
		pipelineLayoutInfo.pPushConstantRanges = nullptr; 

		this->pipelineLayout = this->starDevice.getDevice().createPipelineLayout(pipelineLayoutInfo);
		if (!this->pipelineLayout) {
			throw std::runtime_error("Failed for create pipeline layout");
		}
	}

	void RenderSys_Shadows::createPipeline() {
		assert(vertShader && fragShader && "Shaders need to be provided");

		PipelineConfigSettings pipelineConfig{}; 
		StarPipeline::defaultPipelineConfigInfo(pipelineConfig, swapChainExtent);

		//viewport 
		vk::Rect2D scissor{};
		scissor.offset = vk::Offset2D{ 0, 0 };
		scissor.extent = swapChainExtent;

		vk::Viewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;

		viewport.width = (float)this->swapChainExtent.width;
		viewport.height = (float)this->swapChainExtent.height;
		//Specify values range of depth values to use for the framebuffer. If not doing anything special, leave at default
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

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
		rasterizer.cullMode = vk::CullModeFlagBits::eBack;
		rasterizer.frontFace = vk::FrontFace::eCounterClockwise;
		rasterizer.depthBiasEnable = VK_TRUE; 
		rasterizer.depthBiasConstantFactor = 4.0f; 
		rasterizer.depthBiasSlopeFactor = 1.5f; 

		vk::PipelineViewportStateCreateInfo viewportState{};
		viewportState.sType = vk::StructureType::ePipelineViewportStateCreateInfo;
		viewportState.viewportCount = 1;
		viewportState.pViewports = &viewport;
		viewportState.scissorCount = 1;
		viewportState.pScissors = &scissor;

		pipelineConfig.pipelineLayout = pipelineLayout;
		pipelineConfig.viewportInfo = viewportState;
		pipelineConfig.renderPass = renderPass;
		pipelineConfig.rasterizationInfo = rasterizer; 

		this->starPipeline = std::make_unique<StarPipeline>(&this->starDevice, pipelineConfig, *this->vertShader);
	}
}