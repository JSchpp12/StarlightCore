#include "StarSystem_RenderObj.hpp"

namespace star {
namespace core {

RenderSysObj::~RenderSysObj() {
	if (this->ownerOfSetLayout)
		this->starDevice->getDevice().destroyPipelineLayout(this->pipelineLayout);
}

void RenderSysObj::registerShader(vk::ShaderStageFlagBits stage, common::Shader* newShader, common::Handle newShaderHandle) {
	if (stage & vk::ShaderStageFlagBits::eVertex) {
		this->vertShader = newShader; 
		this->vertShaderHandle = newShaderHandle; 
	}
	else {
		this->fragShader = newShader; 
		this->fragShaderHandle = newShaderHandle; 
	}
}

void RenderSysObj::addObject(common::Handle newObjectHandle, common::GameObject* newObject, size_t numSwapChainImages) {
	auto numIndicies = newObject->getIndicies()->size();
	auto numVerticies = newObject->getVerticies()->size();

	this->totalNumIndicies += numIndicies;
	this->totalNumVerticies += numVerticies;

	this->renderObjects.push_back(std::move(RenderObject::Builder().setFromObject(newObjectHandle, newObject).setNumFrames(numSwapChainImages).build())); 
}

bool RenderSysObj::hasShader(vk::ShaderStageFlagBits stage) {
	if (stage & vk::ShaderStageFlagBits::eVertex && this->vertShader != nullptr) {
		return true; 
	}
	else if (stage & vk::ShaderStageFlagBits::eFragment && this->fragShader != nullptr) {
		return true; 
	}
	return false; 
}

common::Handle RenderSysObj::getBaseShader(vk::ShaderStageFlags stage) {
	if (stage & vk::ShaderStageFlagBits::eVertex) {
		return this->vertShaderHandle; 
	}
	else {
		return this->fragShaderHandle; 
	}
}

void RenderSysObj::setPipelineLayout(vk::PipelineLayout newPipelineLayout) {
	this->ownerOfSetLayout = false; 
	this->pipelineLayout = newPipelineLayout;
}

size_t RenderSysObj::getNumRenderObjects()
{
	return this->renderObjects.size();
}

RenderObject* star::core::RenderSysObj::getRenderObjectAt(size_t index) {
	return this->renderObjects.at(index).get();
}

void star::core::RenderSysObj::bind(vk::CommandBuffer& commandBuffer) {
	this->starPipeline->bind(commandBuffer);
}

void star::core::RenderSysObj::updateBuffers(uint32_t currentImage) {
	UniformBufferObject newBufferObject{};
	std::vector<UniformBufferObject> bufferObjects(this->renderObjects.size());

	for (size_t i = 0; i < this->renderObjects.size(); i++) {
		newBufferObject.modelMatrix = this->renderObjects.at(i)->getGameObject()->getDisplayMatrix(); 
		newBufferObject.normalMatrix = this->renderObjects.at(i)->getGameObject()->getNormalMatrix(); 

		bufferObjects.at(i) = newBufferObject; 
	}

	this->uniformBuffers[currentImage]->writeToBuffer(bufferObjects.data(), sizeof(UniformBufferObject) * bufferObjects.size()); 
}

void star::core::RenderSysObj::render(vk::CommandBuffer& commandBuffer, int swapChainImageIndex) {
	vk::DeviceSize offsets{};
	commandBuffer.bindVertexBuffers(0, this->vertexBuffer->getBuffer(), offsets);

	commandBuffer.bindIndexBuffer(this->indexBuffer->getBuffer(), 0, vk::IndexType::eUint32);

	int vertexCount = 0; 
	RenderObject* currRenderObject = nullptr; 
	for (size_t i = 0; i < this->renderObjects.size(); i++) {
		currRenderObject = this->renderObjects.at(i).get();
		commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, this->pipelineLayout, 1, 1, &this->renderObjects.at(i)->getDefaultDescriptorSets()->at(swapChainImageIndex), 0, nullptr);
		commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, this->pipelineLayout, 2, 1, &this->renderObjects.at(i)->getStaticDescriptorSet(), 0, nullptr);
		auto numToDraw = currRenderObject->getNumVerticies(); 
		commandBuffer.drawIndexed(numToDraw, 1, 0, vertexCount, 0); 
		vertexCount += currRenderObject->getNumIndicies(); 
	}
}

void star::core::RenderSysObj::init(std::vector<vk::DescriptorSetLayout> globalDescriptorSets) {
	//create needed buffers 
	createVertexBuffer();
	createIndexBuffer();
	createObjectMaterialBuffer(); 
	createRenderBuffers(); 
	createDescriptorPool();
	createStaticDescriptors(); 
	createDescriptors();
	if (!this->pipelineLayout)
		createPipelineLayout(globalDescriptorSets); 
	createPipeline(); 
}

void RenderSysObj::createVertexBuffer() {
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
		*this->starDevice,
		vertexSize,
		vertexCount, 
		vk::BufferUsageFlagBits::eTransferSrc,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
	}; 
	stagingBuffer.map();
	stagingBuffer.writeToBuffer(vertexList.data()); 

	this->vertexBuffer = std::make_unique<StarBuffer>(
		*this->starDevice,
		vertexSize,
		vertexCount,
		vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst, 
		vk::MemoryPropertyFlagBits::eDeviceLocal);

	this->starDevice->copyBuffer(stagingBuffer.getBuffer(), this->vertexBuffer->getBuffer(), bufferSize); 
}

void RenderSysObj::createObjectMaterialBuffer() {
	std::unique_ptr<MaterialBufferObject> newBufferObject;
	std::vector<MaterialBufferObject> bufferInfo(this->renderObjects.size());
	RenderObject* currObject = nullptr; 

	auto testone = sizeof(MaterialBufferObject); 
	auto minProp = this->starDevice->getPhysicalDevice().getProperties().limits.minStorageBufferOffsetAlignment;
	auto alignmentOfElements = StarBuffer::getAlignment(sizeof(MaterialBufferObject), minProp);

	vk::DeviceSize bufferSize = alignmentOfElements * bufferInfo.size();
	uint32_t objectSize = sizeof(MaterialBufferObject);
	uint32_t objectCount = bufferInfo.size();

	StarBuffer stagingBuffer{
		*this->starDevice,
		objectSize,
		objectCount,
		vk::BufferUsageFlagBits::eTransferSrc,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, minProp };
	stagingBuffer.map();

	for (size_t i = 0; i < this->renderObjects.size(); i++) {
		currObject = this->renderObjects.at(i).get();

		newBufferObject = std::make_unique<MaterialBufferObject>(MaterialBufferObject{
			currObject->getGameObject()->getMaterial()->surfaceColor,
			currObject->getGameObject()->getMaterial()->highlightColor,
			currObject->getGameObject()->getMaterial()->shinyCoefficient}); 
		stagingBuffer.writeToBuffer(newBufferObject.get(), sizeof(MaterialBufferObject), stagingBuffer.getAlignmentSize() * i);
	}

	//this will eventually be used to store object textures, need a large buffer (storage buffer)
	this->objectMaterialBuffer = std::make_unique<StarBuffer>(
		*this->starDevice,
		objectSize,
		objectCount, 
		vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst,
		vk::MemoryPropertyFlagBits::eDeviceLocal, minProp);

	this->starDevice->copyBuffer(stagingBuffer.getBuffer(), this->objectMaterialBuffer->getBuffer(), stagingBuffer.getBufferSize());
}

void RenderSysObj::createIndexBuffer() {
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
		*this->starDevice, 
		indexSize, 
		this->totalNumIndicies, 
		vk::BufferUsageFlagBits::eTransferSrc, 
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
	};
	stagingBuffer.map(); 
	stagingBuffer.writeToBuffer(indiciesList.data()); 

	this->indexBuffer = std::make_unique<StarBuffer>(
		*this->starDevice, 
		indexSize, 
		this->totalNumIndicies, 
		vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst,
		vk::MemoryPropertyFlagBits::eDeviceLocal );
	this->starDevice->copyBuffer(stagingBuffer.getBuffer(), this->indexBuffer->getBuffer(), bufferSize);
}

void RenderSysObj::createDescriptorPool() {
	//create descriptor pools 
	this->descriptorPool = StarDescriptorPool::Builder(*this->starDevice)
		.setMaxSets(this->numSwapChainImages * this->renderObjects.size() + this->renderObjects.size())
		.addPoolSize(vk::DescriptorType::eUniformBuffer, this->numSwapChainImages * this->renderObjects.size())
		.addPoolSize(vk::DescriptorType::eStorageBuffer, this->numSwapChainImages)
		.build();
}

void RenderSysObj::createStaticDescriptors() {
	this->staticDescriptorSetLayout = StarDescriptorSetLayout::Builder(*this->starDevice)
		.addBinding(0, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eAllGraphics)
		.build();

	//create descritptor sets 
	vk::DescriptorBufferInfo bufferInfo{};
	auto test = this->objectMaterialBuffer->getAlignmentSize(); 

	for (int i = 0; i < this->renderObjects.size(); i++) {
		bufferInfo = vk::DescriptorBufferInfo{
			this->objectMaterialBuffer->getBuffer(),
			this->objectMaterialBuffer->getAlignmentSize()* i,
			sizeof(MaterialBufferObject)
		};

		StarDescriptorWriter(*this->starDevice, *this->staticDescriptorSetLayout, *this->descriptorPool)
			.writeBuffer(0, &bufferInfo)
			.build(this->renderObjects.at(i)->getStaticDescriptorSet());
	}
}

void RenderSysObj::createDescriptors() {
	//create descriptor layouts
	this->descriptorSetLayout = StarDescriptorSetLayout::Builder(*this->starDevice)
		.addBinding(0, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex)
		.build();

	auto minProp = this->starDevice->getPhysicalDevice().getProperties().limits.minUniformBufferOffsetAlignment;
	auto minAlignmentOfUBOElements = StarBuffer::getAlignment(sizeof(UniformBufferObject), minProp);

	//create descritptor sets 
	vk::DescriptorBufferInfo bufferInfo{}; 
	for (int i = 0; i < this->numSwapChainImages; i++) {		
		for (int j = 0; j < this->renderObjects.size(); j++) {

			bufferInfo = vk::DescriptorBufferInfo{
				this->uniformBuffers[i]->getBuffer(),
				minAlignmentOfUBOElements* j,
				sizeof(UniformBufferObject)
			};

			StarDescriptorWriter(*this->starDevice, *this->descriptorSetLayout, *this->descriptorPool)
				.writeBuffer(0, &bufferInfo)
				.build(this->renderObjects.at(j)->getDefaultDescriptorSets()->at(i));
		}
	}
}

void RenderSysObj::createRenderBuffers() {
	this->uniformBuffers.resize(this->numSwapChainImages);

	auto minUniformSize = this->starDevice->getPhysicalDevice().getProperties().limits.minUniformBufferOffsetAlignment;
	for (size_t i = 0; i < numSwapChainImages; i++) {
		this->uniformBuffers[i] = std::make_unique<StarBuffer>(*this->starDevice, this->renderObjects.size(), sizeof(UniformBufferObject), 
			vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, minUniformSize);
		this->uniformBuffers[i]->map();
	}
}

void RenderSysObj::createPipelineLayout(std::vector<vk::DescriptorSetLayout> globalDescriptorSets) {
	globalDescriptorSets.push_back(this->descriptorSetLayout->getDescriptorSetLayout()); 
	globalDescriptorSets.push_back(this->staticDescriptorSetLayout->getDescriptorSetLayout());

	/* Pipeline Layout */
	//uniform values in shaders need to be defined here 
	vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = vk::StructureType::ePipelineLayoutCreateInfo;
	pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(globalDescriptorSets.size());
	pipelineLayoutInfo.pSetLayouts = globalDescriptorSets.data();
	pipelineLayoutInfo.pushConstantRangeCount = 0;
	pipelineLayoutInfo.pPushConstantRanges = nullptr;

	this->pipelineLayout = this->starDevice->getDevice().createPipelineLayout(pipelineLayoutInfo);
	if (!this->pipelineLayout) {
		throw std::runtime_error("failed to create pipeline layout");
	}
}

void RenderSysObj::createPipeline() {
	/* Scissor */
	//this defines in which regions pixels will actually be stored. 
	//any pixels outside will be discarded 

	//we just want to draw the whole framebuffer for now
	vk::Rect2D scissor{};
	//scissor.offset = { 0, 0 };
	scissor.extent = swapChainExtent;

	/* Viewport */
	//Viewport describes the region of the framebuffer where the output will be rendered to
	vk::Viewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;

	viewport.width = (float)this->swapChainExtent.width;
	viewport.height = (float)this->swapChainExtent.height;
	//Specify values range of depth values to use for the framebuffer. If not doing anything special, leave at default
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	//put scissor and viewport together into struct for creation 
	vk::PipelineViewportStateCreateInfo viewportState{};
	viewportState.sType = vk::StructureType::ePipelineViewportStateCreateInfo;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

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
	rasterizer.cullMode = vk::CullModeFlagBits::eBack;
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

	PipelineConfigSettings config{};
	config.viewportInfo = viewportState;
	config.rasterizationInfo = rasterizer;
	config.multisampleInfo = multisampling;
	config.depthStencilInfo = depthStencil;
	config.colorBlendInfo = colorBlending;
	config.colorBlendAttachment = colorBlendAttachment;
	config.pipelineLayout = this->pipelineLayout; 
	config.renderPass = this->renderPass;

	this->starPipeline = std::make_unique<StarPipeline>(this->starDevice, this->vertShader, this->fragShader, config);
}
}
}