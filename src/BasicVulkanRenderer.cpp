#include "BasicVulkanRenderer.h"

typedef std::chrono::high_resolution_clock Clock;

star::core::VulkanRenderer::VulkanRenderer(common::ConfigFile* configFile,
	common::FileResourceManager<common::Shader>* shaderManager, common::FileResourceManager<common::GameObject>* objectManager,
	common::FileResourceManager<common::Texture>* textureManager, common::Camera* inCamera,
	std::vector<common::Handle>* objectHandleList, std::vector<common::Light*>& lightHandleList,
	StarWindow& window) :
	star::common::Renderer(configFile, shaderManager, objectManager, textureManager, inCamera, objectHandleList),
	glfwRequiredExtensionsCount(new uint32_t), starWindow(window), lightList(lightHandleList)
{
	common::GameObject* currentObject = nullptr;
	common::Light* currLight = nullptr;
	this->starDevice = std::unique_ptr<StarDevice>(new StarDevice(this->starWindow));
	for (size_t i = 0; i < this->lightList.size(); i++) {
		currLight = this->lightList.at(i);
		if (currLight->getType() == common::Type::Light::point && this->pointLight == nullptr) {
			this->pointLight = this->lightList.at(i);;
		}
		else if (currLight->getType() == common::Type::Light::directional && this->ambientLight == nullptr) {
			this->ambientLight = this->lightList.at(i);
		}
		else {
			throw std::runtime_error("More than one light source of each type is not yet supported");
		}
	}
}

star::core::VulkanRenderer::~VulkanRenderer() {
	this->starDevice->getDevice().waitIdle();
	cleanup();
}

void star::core::VulkanRenderer::updateUniformBuffer(uint32_t currentImage) {
	static auto startTime = std::chrono::high_resolution_clock::now();

	auto currentTime = std::chrono::high_resolution_clock::now();
	float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

	//update global ubo 
	GlobalUniformBufferObject globalUbo;
	globalUbo.proj = glm::perspective(glm::radians(45.0f), swapChainExtent.width / (float)swapChainExtent.height, 0.1f, 10.0f);
	//glm designed for openGL where the Y coordinate of the flip coordinates is inverted. Fix this by flipping the sign on the scaling factor of the Y axis in the projection matrix.
	globalUbo.proj[1][1] *= -1;
	globalUbo.view = this->camera->getDisplayMatrix();
	globalUbo.ambientLightColor = this->ambientLight->getColor();
	globalUbo.lightPosition = this->pointLight->getPosition();
	globalUbo.lightColor = this->pointLight->getColor();
	this->globalUniformBuffers[currentImage]->writeToBuffer(&globalUbo, sizeof(globalUbo));

	for (size_t i = 0; i < this->RenderSysObjs.size(); i++) {
		RenderSysObjs.at(i)->updateBuffers(currentImage);
	}

	this->lightRenderSys->updateBuffers(currentImage); 

	//UniformBufferObject obj; 
	//obj.model = currObject->getDisplayMatrix(); 
	////obj.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));

	//void* data = this->starDevice->getDevice().mapMemory(this->uniformBuffersMemory[currentImage], 0, sizeof(UniformBufferObject));
	//memcpy(data, &obj, sizeof(obj));
	//this->starDevice->getDevice().unmapMemory(uniformBuffersMemory[currentImage]); 

	//std::unique_ptr<RenderSysObj::UniformBufferObject> newBufferObject;
	//auto test = tmpRenderSysObj->getNumRenderObjects();
	//for (size_t i = 0; i < tmpRenderSysObj->getNumRenderObjects(); i++) {
	//	newBufferObject = std::make_unique<RenderSysObj::UniformBufferObject>(RenderSysObj::UniformBufferObject());

	//	RenderObject* currRenderObject = tmpRenderSysObj->getRenderObjectAt(i);
	//	currObject = this->objectManager->Get(currRenderObject->getHandle()); 
	//		//this->objectManager->Get(tmpRenderSysObj->getObjectHandleAt(i));
	//	//glm::mat4(1,0f) = identity matrix
	//	//time * radians(90) = rotate 90degrees per second
	//	
	//	newBufferObject->modelMatrix = currObject->getDisplayMatrix();
	//	newBufferObject->normalMatrix = currObject->getNormalMatrix(); 
	//	//look at geometry from above at 45 degree angle 
	//	/* LookAt takes:
	//	*   1. eye position
	//	*   2. center position
	//	*   3. up axis
	//	*/

	//	//perspective projection with 45 degree vertical field of view -- important to use swapChainExtent to calculate aspect ratio (REFRESH WITH WINDOW RESIZE)
	//	/* perspective takes:
	//	*   1. fov
	//	*   2. aspect ratio
	//	*   3. near view plane
	//	*   4. far view plane
	//	*/
	//	//copy data to the current uniform buffer 

	//	ubos.at(i) = *newBufferObject;
	//}

	//TODO: this NEEDS to be more dynamic 
	//create ubo for light 

	//this->uniformBuffers[currentImage]->writeToBuffer(ubos.data(), sizeof(RenderSysObj::UniformBufferObject) * ubos.size());
}

void star::core::VulkanRenderer::prepareGLFW(int width, int height, GLFWkeyfun keyboardCallbackFunction, GLFWmousebuttonfun mouseButtonCallback, GLFWcursorposfun cursorPositionCallback, GLFWscrollfun scrollCallback) {

}


void star::core::VulkanRenderer::pollEvents() {
	glfwPollEvents();
}

void star::core::VulkanRenderer::prepare() {
	createSwapChain();
	createImageViews();
	createRenderPass();

	this->globalPool = StarDescriptorPool::Builder(this->starDevice.get())
		.setMaxSets((this->swapChainImages.size()))
		.addPoolSize(vk::DescriptorType::eUniformBuffer, this->swapChainImages.size())
		.build();

	this->globalSetLayout = StarDescriptorSetLayout::Builder(this->starDevice.get())
		.addBinding(0, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment)
		.build();

	this->globalDescriptorSets.resize(this->swapChainImages.size());


	common::GameObject* currObject = nullptr;
	vk::ShaderStageFlagBits stages{};
	vk::Device device = this->starDevice->getDevice();
	this->RenderSysObjs.push_back(std::make_unique<RenderSysObj>(this->starDevice.get(), this->swapChainImages.size(), this->globalSetLayout->getDescriptorSetLayout(), this->swapChainExtent, this->renderPass));
	RenderSysObj* tmpRenderSysObj = this->RenderSysObjs.at(0).get();

	for (size_t i = 0; i < this->objectList->size(); i++) {
		currObject = this->objectManager->Get(this->objectList->at(i));

		//check if the vulkan object has a shader registered for the desired stage that is different than the one needed for the current object
		for (size_t j = 0; j < this->RenderSysObjs.size(); j++) {
			//check if object shaders are in vulkan object
			RenderSysObj* object = this->RenderSysObjs.at(j).get();
			if (!object->hasShader(vk::ShaderStageFlagBits::eVertex) && (!object->hasShader(vk::ShaderStageFlagBits::eFragment))) {
				//vulkan object does not have either a vertex or a fragment shader 
				object->registerShader(vk::ShaderStageFlagBits::eVertex, this->shaderManager->Get(currObject->getVertShader()), currObject->getVertShader());
				object->registerShader(vk::ShaderStageFlagBits::eFragment, this->shaderManager->Get(currObject->getFragShader()), currObject->getFragShader());
				object->addObject(this->objectList->at(i), currObject, this->swapChainImages.size());
			}
			else if ((object->getBaseShader(vk::ShaderStageFlagBits::eVertex).containerIndex != currObject->getVertShader().containerIndex) ||
				(object->getBaseShader(vk::ShaderStageFlagBits::eFragment).containerIndex != currObject->getFragShader().containerIndex)) {
				//vulkan object has shaders but they are not the same as the shaders needed for current render object
				this->RenderSysObjs.push_back(std::make_unique<RenderSysObj>(this->starDevice.get(), this->swapChainImages.size(), this->globalSetLayout->getDescriptorSetLayout(), this->swapChainExtent, this->renderPass));
				RenderSysObj* newObject = this->RenderSysObjs.at(this->RenderSysObjs.size()).get();
				newObject->registerShader(vk::ShaderStageFlagBits::eVertex, this->shaderManager->Get(currObject->getVertShader()), currObject->getVertShader());
				newObject->registerShader(vk::ShaderStageFlagBits::eFragment, this->shaderManager->Get(currObject->getFragShader()), currObject->getFragShader());
				newObject->addObject(this->objectList->at(i), currObject, this->swapChainImages.size());
			}
			else {
				//vulkan object has the same shaders as the render object 
				object->addObject(this->objectList->at(i), currObject, this->swapChainImages.size());
			}
		}
	}
	tmpRenderSysObj->init();

	//TODO: might need more than one light system
	this->lightRenderSys = std::make_unique<RenderSysPointLight>(this->starDevice.get(), this->swapChainImages.size(), this->globalSetLayout->getDescriptorSetLayout(), this->swapChainExtent, this->renderPass);
	common::GameObject* lightLinkedObject = this->objectManager->Get(this->pointLight->getLinkedObjectHandle());
	this->lightRenderSys->registerShader(vk::ShaderStageFlagBits::eVertex, this->shaderManager->Get(lightLinkedObject->getVertShader()), lightLinkedObject->getVertShader());
	this->lightRenderSys->registerShader(vk::ShaderStageFlagBits::eFragment, this->shaderManager->Get(lightLinkedObject->getFragShader()), lightLinkedObject->getFragShader());
	for (auto light : this->lightList) {
		if (light->getType() == common::Type::Light::point) {
			common::GameObject* lightLinkedObject = this->objectManager->Get(light->getLinkedObjectHandle());

			this->lightRenderSys->addLight(light, lightLinkedObject, this->swapChainImages.size());
		}
	}
	this->lightRenderSys->init(); 

	createDepthResources();
	createFramebuffers();
	createTextureImage();
	createTextureImageView();
	createTextureSampler();
	createRenderingBuffers();
	createGraphicsPipeline();

	std::unique_ptr<std::vector<vk::DescriptorBufferInfo>> bufferInfos{};
	for (size_t i = 0; i < this->swapChainImages.size(); i++) {
		//global
		bufferInfos = std::make_unique<std::vector<vk::DescriptorBufferInfo>>();

		bufferInfos->push_back(vk::DescriptorBufferInfo{
			this->globalUniformBuffers[i]->getBuffer(),
			0,
			sizeof(GlobalUniformBufferObject) });

		StarDescriptorWriter(this->starDevice.get(), *this->globalSetLayout, *this->globalPool)
			.writeBuffer(0, &bufferInfos.get()->at(0))
			.build(this->globalDescriptorSets.at(i));
	}

	createCommandBuffers();
	createSemaphores();
	createFences();
	createFenceImageTracking();
}

void star::core::VulkanRenderer::draw() {
	/* Goals of each call to drawFrame:
   *   get an image from the swap chain
   *   execute command buffer with that image as attachment in the framebuffer
   *   return the image to swapchain for presentation
   * by default each of these steps would be executed asynchronously so need method of synchronizing calls with rendering
   * two ways of doing this:
   *   1. fences
   *       accessed through calls to vkWaitForFences
   *       designed to synchronize appliecation itself with rendering ops
   *   2. semaphores
   *       designed to synchronize opertaions within or across command queues
   * need to sync queu operations of draw and presentation commmands -> using semaphores
   */

   //wait for fence to be ready 
   // 3. 'VK_TRUE' -> waiting for all fences
   // 4. timeout 
	this->starDevice->getDevice().waitForFences(inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

	/* Get Image From Swapchain */


	//as is extension we must use vk*KHR naming convention
	//UINT64_MAX -> 3rd argument: used to specify timeout in nanoseconds for image to become available
	/* Suboptimal SwapChain notes */
		//vulkan can return two different flags 
		// 1. VK_ERROR_OUT_OF_DATE_KHR: swap chain has become incompatible with the surface and cant be used for rendering. (Window resize)
		// 2. VK_SUBOPTIMAL_KHR: swap chain can still be used to present to the surface, but the surface properties no longer match
	auto result = this->starDevice->getDevice().acquireNextImageKHR(swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame]);

	if (result.result == vk::Result::eErrorOutOfDateKHR) {
		//the swapchain is no longer optimal according to vulkan. Must recreate a more efficient swap chain
		recreateSwapChain();
		return;
	}
	else if (result.result != vk::Result::eSuccess && result.result != vk::Result::eSuboptimalKHR) {
		//for VK_SUBOPTIMAL_KHR can also recreate swap chain. However, chose to continue to presentation stage
		throw std::runtime_error("failed to aquire swap chain image");
	}
	uint32_t imageIndex = result.value;

	//check if a previous frame is using the current image
	if (imagesInFlight[imageIndex]) {
		this->starDevice->getDevice().waitForFences(1, &imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
	}
	//mark image as now being in use by this frame by assigning the fence to it 
	imagesInFlight[imageIndex] = inFlightFences[currentFrame];

	updateUniformBuffer(imageIndex);


	/* Command Buffer */
	vk::SubmitInfo submitInfo{};
	submitInfo.sType = vk::StructureType::eSubmitInfo;

	vk::Semaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame] };

	//where in the pipeline should the wait happen, want to wait until image becomes available
	//wait at stage of color attachment -> theoretically allows for shader execution before wait 
	vk::PipelineStageFlags waitStages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput }; //each entry corresponds through index to waitSemaphores[]
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;

	//which command buffers to submit for execution -- should submit command buffer that binds the swap chain image that was just acquired as color attachment
	submitInfo.commandBufferCount = 1;
	//submitInfo.pCommandBuffers = &graphicsCommandBuffers[imageIndex];
	submitInfo.pCommandBuffers = &this->starDevice->getGraphicsCommandBuffers()->at(imageIndex);

	//what semaphores to signal when command buffers have finished
	vk::Semaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame] };

	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	//set fence to unsignaled state
	this->starDevice->getDevice().resetFences(1, &inFlightFences[currentFrame]);
	auto submitResult = this->starDevice->getGraphicsQueue().submit(1, &submitInfo, inFlightFences[currentFrame]);
	if (submitResult != vk::Result::eSuccess) {
		throw std::runtime_error("failed to submit draw command buffer");
	}

	/* Presentation */
	vk::PresentInfoKHR presentInfo{};
	//presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.sType = vk::StructureType::ePresentInfoKHR;

	//what to wait for 
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	//what swapchains to present images to 
	vk::SwapchainKHR swapChains[] = { swapChain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageIndex;

	//can use this to get results from swap chain to check if presentation was successful
	presentInfo.pResults = nullptr; // Optional

	//make call to present image
	auto presentResult = this->starDevice->getPresentQueue().presentKHR(presentInfo);

	//if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || frameBufferResized) {
	if (presentResult == vk::Result::eErrorOutOfDateKHR || presentResult == vk::Result::eSuboptimalKHR || frameBufferResized) {
		frameBufferResized = false;
		recreateSwapChain();
	}
	else if (presentResult != vk::Result::eSuccess) {
		throw std::runtime_error("failed to present swap chain image");
	}

	//advance to next frame
	currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void star::core::VulkanRenderer::cleanup() {
	cleanupSwapChain();

	this->starDevice->getDevice().destroySampler(this->textureSampler);
	this->starDevice->getDevice().destroyImageView(this->textureImageView);
	this->starDevice->getDevice().destroyImage(this->textureImage);
	this->starDevice->getDevice().freeMemory(this->textureImageMemory);

	RenderSysObj* currRenderSysObj = this->RenderSysObjs.at(0).get();

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		this->starDevice->getDevice().destroySemaphore(renderFinishedSemaphores[i]);
		this->starDevice->getDevice().destroySemaphore(imageAvailableSemaphores[i]);
		this->starDevice->getDevice().destroyFence(inFlightFences[i]);
	}
}

void star::core::VulkanRenderer::cleanupSwapChain() {
	auto& tmpRenderSysObj = this->RenderSysObjs.at(0);
	this->starDevice->getDevice().destroyImageView(this->depthImageView);
	this->starDevice->getDevice().destroyImage(this->depthImage);
	this->starDevice->getDevice().freeMemory(this->depthImageMemory);

	for (auto framebuffer : this->swapChainFramebuffers) {
		this->starDevice->getDevice().destroyFramebuffer(framebuffer);
	}

	this->starDevice->getDevice().destroyRenderPass(this->renderPass);

	for (auto imageView : this->swapChainImageViews) {
		this->starDevice->getDevice().destroyImageView(imageView);
	}

	this->starDevice->getDevice().destroySwapchainKHR(this->swapChain);
}

void star::core::VulkanRenderer::createSwapChain() {
	//TODO: current implementation requires halting to all rendering when recreating swapchain. Can place old swap chain in oldSwapChain field 
	//  in order to prevent this and allow rendering to continue
	SwapChainSupportDetails swapChainSupport = this->starDevice->getSwapChainSupportDetails();

	vk::SurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
	vk::PresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
	vk::Extent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

	//how many images should be in the swap chain 
	//in order to avoid extra waiting for driver overhead, author of tutorial recommends +1 of the minimum
	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

	//with this additional +1 make sure not to go over maximum permitted 
	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}

	vk::SwapchainCreateInfoKHR createInfo{};
	createInfo.sType = vk::StructureType::eSwapchainCreateInfoKHR;
	//createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;     
	createInfo.surface = this->starDevice->getSurface();

	//specify image information for the surface 
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1; //1 unless using 3D display 
	//createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; //how are these images going to be used? Color attachment since we are rendering to them (can change for postprocessing effects)
	createInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment; //how are these images going to be used? Color attachment since we are rendering to them (can change for postprocessing effects)

	QueueFamilyIndicies indicies = this->starDevice->findPhysicalQueueFamilies();
	uint32_t queueFamilyIndicies[] = { indicies.graphicsFamily.value(), indicies.transferFamily.value(), indicies.presentFamily.value() };

	if (indicies.graphicsFamily != indicies.presentFamily && indicies.presentFamily != indicies.transferFamily) {
		/*need to handle how images will be transferred between different queues
		* so we need to draw images on the graphics queue and then submitting them to the presentation queue
		* Two ways of handling this:
		* 1. VK_SHARING_MODE_EXCLUSIVE: an image is owned by one queue family at a time and can be transferred between groups
		* 2. VK_SHARING_MODE_CONCURRENT: images can be used across queue families without explicit ownership
		*/
		//createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
		createInfo.queueFamilyIndexCount = 3;
		createInfo.pQueueFamilyIndices = queueFamilyIndicies;
	}
	else if (indicies.graphicsFamily != indicies.presentFamily && indicies.presentFamily == indicies.transferFamily) {
		uint32_t explicitQueueFamilyInd[] = { indicies.graphicsFamily.value(), indicies.presentFamily.value() };
		createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndicies;
	}
	else {
		//same family is used for graphics and presenting
		createInfo.imageSharingMode = vk::SharingMode::eExclusive;
		createInfo.queueFamilyIndexCount = 0; //optional
		createInfo.pQueueFamilyIndices = nullptr; //optional
	}

	//can specify certain transforms if they are supported (like 90 degree clockwise rotation)
	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
	createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;

	//what present mode is going to be used
	createInfo.presentMode = presentMode;
	//if clipped is set to true, we dont care about color of pixes that arent in sight -- best performance to enable this
	createInfo.clipped = VK_TRUE;

	//for now, only assume we are making one swapchain
	createInfo.oldSwapchain = VK_NULL_HANDLE;

	this->swapChain = this->starDevice->getDevice().createSwapchainKHR(createInfo);

	//if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
	//    throw std::runtime_error("failed to create swap chain");
	//}

	//get images in the newly created swapchain 
	this->swapChainImages = this->starDevice->getDevice().getSwapchainImagesKHR(this->swapChain);

	//save swapChain information for later use
	swapChainImageFormat = surfaceFormat.format;
	swapChainExtent = extent;
}

void star::core::VulkanRenderer::recreateSwapChain() {
	int width = 0, height = 0;
	//check for window minimization and wait for window size to no longer be 0
	glfwGetFramebufferSize(this->starWindow.getGLFWwindow(), &width, &height);
	while (width == 0 || height == 0) {
		glfwGetFramebufferSize(this->starWindow.getGLFWwindow(), &width, &height);
		glfwWaitEvents();
	}
	//wait for device to finish any current actions
	vkDeviceWaitIdle(this->starDevice->getDevice());

	cleanupSwapChain();

	//create swap chain itself 
	createSwapChain();

	//image views depend directly on swap chain images so these need to be recreated
	createImageViews();

	//render pass depends on the format of swap chain images
	createRenderPass();

	////viewport and scissor rectangle size are declared during pipeline creation, so the pipeline must be recreated
	////can use dynamic states for viewport and scissor to avoid this 
	createGraphicsPipeline();

	createDepthResources();

	createFramebuffers();

	//uniform buffers are dependent on the number of swap chain images, will need to recreate since they are destroyed in cleanupSwapchain()
	createRenderingBuffers();

	//createDescriptorPool();

	//createDescriptorSets();

	createCommandBuffers();
}

vk::SurfaceFormatKHR star::core::VulkanRenderer::chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats) {
	for (const auto& availableFormat : availableFormats) {
		//check if a format allows 8 bits for R,G,B, and alpha channel
		//use SRGB color space

		if (availableFormat.format == vk::Format::eB8G8R8A8Srgb && availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
			return availableFormat;
		}
	}

	//if nothing matches what we are looking for, just take what is available
	return availableFormats[0];
}

vk::PresentModeKHR star::core::VulkanRenderer::chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes) {
	/*
	* There are a number of swap modes that are in vulkan
	* 1. VK_PRESENT_MODE_IMMEDIATE_KHR: images submitted by application are sent to the screen right away -- can cause tearing
	* 2. VK_PRESENT_MODE_FIFO_RELAXED_KHR: images are placed in a queue and images are sent to the display in time with display refresh (VSYNC like). If queue is full, application has to wait
	*   "Vertical blank" -> time when the display is refreshed
	* 3. VK_PRESENT_MODE_FIFO_RELAXED_KHR: same as above. Except if the application is late, and the queue is empty: the next image submitted is sent to display right away instead of waiting for next blank.
	* 4. VK_PRESENT_MODE_MAILBOX_KHR: similar to #2 option. Instead of blocking applicaiton when the queue is full, the images in the queue are replaced with newer images.
	*   This mode can be used to render frames as fast as possible while still avoiding tearing. Kind of like "tripple buffering". Does not mean that framerate is unlocked however.
	*   Author of tutorial statement: this mode [4] is a good tradeoff if energy use is not a concern. On mobile devices it might be better to go with [2]
	*/

	for (const auto& availablePresentMode : availablePresentModes) {
		if (availablePresentMode == vk::PresentModeKHR::eMailbox) {
			return availablePresentMode;
		}
	}

	//only VK_PRESENT_MODE_FIFO_KHR is guaranteed to be available
	return vk::PresentModeKHR::eFifo;
}

vk::Extent2D star::core::VulkanRenderer::chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities) {
	/*
	* "swap extent" -> resolution of the swap chain images (usually the same as window resultion
	* Range of available resolutions are defined in VkSurfaceCapabilitiesKHR
	* Resultion can be changed by setting value in currentExtent to the maximum value of a uint32_t
	*   then: the resolution can be picked by matching window size in minImageExtent and maxImageExtent
	*/
	if (capabilities.currentExtent.width != UINT32_MAX) {
		return capabilities.currentExtent;
	}
	else {
		//vulkan requires that resultion be defined in pixels -- if a high DPI display is used, screen coordinates do not match with pixels
		int width, height;
		glfwGetFramebufferSize(this->starWindow.getGLFWwindow(), &width, &height);

		vk::Extent2D actualExtent = {
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		};

		//(clamp) -- keep the width and height bounded by the permitted resolutions 
		actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

		return actualExtent;
	}
}

void star::core::VulkanRenderer::createImageViews() {
	swapChainImageViews.resize(swapChainImages.size());

	//need to create an imageView for each of the images available
	for (size_t i = 0; i < swapChainImages.size(); i++) {
		//swapChainImageViews[i] = createImageView(swapChainImages[i], swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
		swapChainImageViews[i] = createImageView(swapChainImages[i], swapChainImageFormat, vk::ImageAspectFlagBits::eColor);
	}
}

vk::ImageView star::core::VulkanRenderer::createImageView(vk::Image image, vk::Format format, vk::ImageAspectFlagBits aspectFlags) {
	vk::ImageViewCreateInfo viewInfo{};
	viewInfo.sType = vk::StructureType::eImageViewCreateInfo;
	viewInfo.image = image;
	viewInfo.viewType = vk::ImageViewType::e2D;
	viewInfo.format = format;
	viewInfo.subresourceRange.aspectMask = aspectFlags;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;

	vk::ImageView imageView = this->starDevice->getDevice().createImageView(viewInfo);

	if (!imageView) {
		throw std::runtime_error("failed to create texture image view!");
	}

	return imageView;
}

void star::core::VulkanRenderer::createRenderPass() {
	/*  Single render pass consists of many small subpasses
	each subpasses are subsequent rendering operations that depend on the contents of framebuffers in the previous pass.
	It is best to group these into one rendering pass, then vulkan can optimize for this in order to save memory bandwidth.
	For this program, we are going to stick with one subpass
*/
/* Depth attachment */
	vk::AttachmentDescription depthAttachment{};
	depthAttachment.format = findDepthFormat();
	depthAttachment.samples = vk::SampleCountFlagBits::e1;
	depthAttachment.loadOp = vk::AttachmentLoadOp::eClear;
	depthAttachment.storeOp = vk::AttachmentStoreOp::eDontCare;     //wont be used after draw 
	depthAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
	depthAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
	depthAttachment.initialLayout = vk::ImageLayout::eUndefined;    //dont care about previous depth contents 
	depthAttachment.finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

	vk::AttachmentReference depthAttachmentRef{};
	depthAttachmentRef.attachment = 1;
	depthAttachmentRef.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

	/* Color attachment */
	vk::AttachmentDescription colorAttachment{};
	//format of color attachment needs to match the swapChain image format
	colorAttachment.format = swapChainImageFormat;
	//no multisampling needed so leave at 1 samples
	colorAttachment.samples = vk::SampleCountFlagBits::e1;
	colorAttachment.loadOp = vk::AttachmentLoadOp::eClear;
	colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;

	/* Choices for loadOp:
		1. VK_ATTACHMENT_LOAD_OP_LOAD: preserve the existing contents of the attachment
		2. VK_ATTACHMENT_LOAD_OP_CLEAR: clear the values to a constant at the start
		3. VK_ATTACHMENT_LOAD_OP_DONT_CARE: existing contents are undefined
	*/
	//what do to with data before rendering
	colorAttachment.loadOp = vk::AttachmentLoadOp::eClear;
	/* Choices for storeOp:
		1. VK_ATTACHMENT_STORE_OP_STORE: rendered contents will be stored for later use
		2. VK_ATTACHMENT_STORE_OP_DONT_CARE: contents of the frame buffer will be undefined after the rendering operation
	*/
	//what to do with data after rendering
	colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;    //since we want to see the rendered triangle, we are going to store 

	/*Image layouts can change depending on what operation is being performed
	* Possible layouts are:
	* 1. VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL: image is used as a color attachment
	* 2. VK_IMAGE_LAYOUT_PRESENT_SRC_KHR: images to be presented in the swap chain
	* 3. VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL: images to be used as destination for memory copy operation
	*/
	//dont care what format image is in before render - contents of image are not guaranteed to be preserved 
	colorAttachment.initialLayout = vk::ImageLayout::eUndefined;
	//want image to be ready for display 
	colorAttachment.finalLayout = vk::ImageLayout::ePresentSrcKHR;

	/* Color attachment references */
	vk::AttachmentReference colorAttachmentRef{};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;    //will give best performance

	/* Subpass */
	vk::SubpassDescription subpass{};
	subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;
	subpass.pDepthStencilAttachment = &depthAttachmentRef;

	/* Subpass Dependencies */
	vk::SubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests;
	dependency.srcAccessMask = vk::AccessFlagBits::eNone;
	dependency.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests;
	dependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite; //allow for write 

	/* Render Pass */
	std::array<vk::AttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };
	vk::RenderPassCreateInfo renderPassInfo{};
	//renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.sType = vk::StructureType::eRenderPassCreateInfo;
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	this->renderPass = this->starDevice->getDevice().createRenderPass(renderPassInfo);
	if (!renderPass) {
		throw std::runtime_error("failed to create render pass");
	}
}

vk::Format star::core::VulkanRenderer::findDepthFormat() {
	//utilizing the VK_FORMAT_FEATURE_ flag to check for candidates that have a depth component.
	return this->starDevice->findSupportedFormat(
		{ vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint },
		vk::ImageTiling::eOptimal,
		vk::FormatFeatureFlagBits::eDepthStencilAttachment);
}

void star::core::VulkanRenderer::createGraphicsPipeline() {
	//this->RenderSysObjs.at(0)->createPipeline(); 

	//for (size_t i = 0; i < this->RenderSysObjs.size(); i++) {
	//	RenderSysObj* RenderSysObj = this->RenderSysObjs.at(i).get();

	//	/* Scissor */
	//	//this defines in which regions pixels will actually be stored. 
	//	//any pixels outside will be discarded 

	//	//we just want to draw the whole framebuffer for now
	//	vk::Rect2D scissor{};
	//	//scissor.offset = { 0, 0 };
	//	scissor.extent = swapChainExtent;

	//	/* Viewport */
	//	//Viewport describes the region of the framebuffer where the output will be rendered to
	//	vk::Viewport viewport{};
	//	viewport.x = 0.0f;
	//	viewport.y = 0.0f;

	//	viewport.width = (float)this->swapChainExtent.width;
	//	viewport.height = (float)this->swapChainExtent.height;
	//	//Specify values range of depth values to use for the framebuffer. If not doing anything special, leave at default
	//	viewport.minDepth = 0.0f;
	//	viewport.maxDepth = 1.0f;

	//	//put scissor and viewport together into struct for creation 
	//	vk::PipelineViewportStateCreateInfo viewportState{};
	//	viewportState.sType = vk::StructureType::ePipelineViewportStateCreateInfo;
	//	viewportState.viewportCount = 1;
	//	viewportState.pViewports = &viewport;
	//	viewportState.scissorCount = 1;
	//	viewportState.pScissors = &scissor;

	//	/* Rasterizer */
	// //takes the geometry and creates fragments which are then passed onto the fragment shader 
	// //also does: depth testing, face culling, and the scissor test
	//	vk::PipelineRasterizationStateCreateInfo rasterizer{};
	//	rasterizer.sType = vk::StructureType::ePipelineRasterizationStateCreateInfo;
	//	//if set to true -> fragments that are beyond near and far planes are set to those distances rather than being removed
	//	rasterizer.depthClampEnable = VK_FALSE;

	//	//polygonMode determines how frags are generated. Different options: 
	//	//1. VK_POLYGON_MODE_FILL: fill the area of the polygon with fragments
	//	//2. VK_POLYGON_MODE_LINE: polygon edges are drawn as lines 
	//	//3. VK_POLYGON_MODE_POINT: polygon verticies are drawn as points
	//	//NOTE: using any other than fill, requires GPU feature
	//	rasterizer.polygonMode = vk::PolygonMode::eFill;

	//	//available line widths, depend on GPU. If above 1.0f, required wideLines GPU feature
	//	rasterizer.lineWidth = 1.0f; //measured in fragment widths

	//	//cullMode : type of face culling to use.
	//	rasterizer.cullMode = vk::CullModeFlagBits::eBack;
	//	rasterizer.frontFace = vk::FrontFace::eCounterClockwise;

	//	//depth values can be used in way that is known as 'shadow mapping'. 
	//	//rasterizer is capable of changing depth values through constant addition or biasing based on frags slope 
	//	//this is left as off for now 
	//	rasterizer.depthBiasEnable = VK_FALSE;
	//	rasterizer.depthBiasConstantFactor = 0.0f; //optional 
	//	rasterizer.depthBiasClamp = 0.0f; //optional 
	//	rasterizer.depthBiasSlopeFactor = 0.0f; //optional

	//	/* Multisampling */
	//	//this is one of the methods of performing anti-aliasing
	//	//enabling requires GPU feature -- left off for this tutorial 
	//	vk::PipelineMultisampleStateCreateInfo multisampling{};
	//	multisampling.sType = vk::StructureType::ePipelineMultisampleStateCreateInfo;
	//	multisampling.sampleShadingEnable = VK_FALSE;
	//	multisampling.rasterizationSamples = vk::SampleCountFlagBits::e1;
	//	multisampling.minSampleShading = 1.0f; //optional 
	//	multisampling.pSampleMask = nullptr; //optional
	//	multisampling.alphaToCoverageEnable = VK_FALSE; //optional
	//	multisampling.alphaToOneEnable = VK_FALSE; //optional

	//	/* Depth and Stencil Testing */
	//	//if using depth or stencil buffer, a depth and stencil tests are neeeded
	//	vk::PipelineDepthStencilStateCreateInfo depthStencil{};
	//	depthStencil.sType = vk::StructureType::ePipelineDepthStencilStateCreateInfo;
	//	depthStencil.depthTestEnable = VK_TRUE;             //specifies if depth of new fragments should be compared to the depth buffer to test for actual display state
	//	depthStencil.depthWriteEnable = VK_TRUE;            //specifies if the new depth of fragments that pass the depth tests should be written to the depth buffer 
	//	depthStencil.depthCompareOp = vk::CompareOp::eLess;   //comparison that is performed to keep or discard fragments - here this is: lower depth = closer, so depth of new frags should be less
	//	//following are for optional depth bound testing - keep frags within a specific range 
	//	depthStencil.depthBoundsTestEnable = VK_FALSE;
	//	depthStencil.minDepthBounds = 0.0f;                 //optional 
	//	depthStencil.maxDepthBounds = 1.0f;                 //optional
	//	//following are used for stencil tests - make sure that format of depth image contains a stencil component
	//	depthStencil.stencilTestEnable = VK_FALSE;

	//	/* Color blending */
	//	// after the fragShader has returned a color, it must be combined with the color already in the framebuffer
	//	// there are two ways to do this: 
	//	//      1. mix the old and new value to produce final color
	//	//      2. combine the old a new value using a bitwise operation 
	//	//two structs are needed to create this functionality: 
	//	//  1. VkPipelineColorBlendAttachmentState: configuration per attached framebuffer 
	//	//  2. VkPipelineColorBlendStateCreateInfo: global configuration
	//	//only using one framebuffer in this project -- both of these are disabled in this project
	//	vk::PipelineColorBlendAttachmentState colorBlendAttachment{};
	//	//colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	//	colorBlendAttachment.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
	//	colorBlendAttachment.blendEnable = VK_FALSE;

	//	vk::PipelineColorBlendStateCreateInfo colorBlending{};
	//	colorBlending.sType = vk::StructureType::ePipelineColorBlendStateCreateInfo;
	//	colorBlending.logicOpEnable = VK_FALSE;
	//	colorBlending.logicOp = vk::LogicOp::eCopy;
	//	colorBlending.attachmentCount = 1;
	//	colorBlending.pAttachments = &colorBlendAttachment;
	//	colorBlending.blendConstants[0] = 0.0f;
	//	colorBlending.blendConstants[1] = 0.0f;
	//	colorBlending.blendConstants[2] = 0.0f;
	//	colorBlending.blendConstants[3] = 0.0f;

	//	//std::vector<vk::DescriptorSetLayout> descriptorSetLayouts{ this->globalSetLayout->getDescriptorSetLayout(), this->RenderSysObjs.at(0)->getSetLayout()->getDescriptorSetLayout()};
	//	///* Pipeline Layout */
	//	////uniform values in shaders need to be defined here 
	//	//vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
	//	////pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	//	//pipelineLayoutInfo.sType = vk::StructureType::ePipelineLayoutCreateInfo;
	//	//pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
	//	//pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
	//	//// pipelineLayoutInfo.pushConstantRangeCount = 1;
	//	//// pipelineLayoutInfo.pPushConstantRanges = &pushConstant;
	//	//pipelineLayoutInfo.pushConstantRangeCount = 0;
	//	//pipelineLayoutInfo.pPushConstantRanges = nullptr;

	//	//vk::PipelineLayout newLayout = this->starDevice->getDevice().createPipelineLayout(pipelineLayoutInfo);
	//	//if (!newLayout) {
	//	//	throw std::runtime_error("failed to create pipeline layout");
	//	//}
	//	//RenderSysObj->setPipelineLayout(newLayout);

	//	PipelineConfigSettings config{};
	//	config.viewportInfo = viewportState;
	//	config.rasterizationInfo = rasterizer;
	//	config.multisampleInfo = multisampling;
	//	config.depthStencilInfo = depthStencil;
	//	config.colorBlendInfo = colorBlending;
	//	config.colorBlendAttachment = colorBlendAttachment;
	//	config.pipelineLayout = this->RenderSysObjs.at(0)->getPipelineLayout();
	//	config.renderPass = renderPass;

	//	RenderSysObj->createPipeline(config);
	//}

	////create pipeline for light
	// /* Scissor */
	////this defines in which regions pixels will actually be stored. 
	////any pixels outside will be discarded 

	////we just want to draw the whole framebuffer for now
	//vk::Rect2D scissor{};
	////scissor.offset = { 0, 0 };
	//scissor.extent = swapChainExtent;

	///* Viewport */
	////Viewport describes the region of the framebuffer where the output will be rendered to
	//vk::Viewport viewport{};
	//viewport.x = 0.0f;
	//viewport.y = 0.0f;

	//viewport.width = (float)800;
	//viewport.height = (float)600;
	////Specify values range of depth values to use for the framebuffer. If not doing anything special, leave at default
	//viewport.minDepth = 0.0f;
	//viewport.maxDepth = 1.0f;

	////put scissor and viewport together into struct for creation 
	//vk::PipelineViewportStateCreateInfo viewportState{};
	//viewportState.sType = vk::StructureType::ePipelineViewportStateCreateInfo;
	//viewportState.viewportCount = 1;
	//viewportState.pViewports = &viewport;
	//viewportState.scissorCount = 1;
	//viewportState.pScissors = &scissor;

	///* Rasterizer */
	////takes the geometry and creates fragments which are then passed onto the fragment shader 
	////also does: depth testing, face culling, and the scissor test
	//vk::PipelineRasterizationStateCreateInfo rasterizer{};
	//rasterizer.sType = vk::StructureType::ePipelineRasterizationStateCreateInfo;
	////if set to true -> fragments that are beyond near and far planes are set to those distances rather than being removed
	//rasterizer.depthClampEnable = VK_FALSE;

	////polygonMode determines how frags are generated. Different options: 
	////1. VK_POLYGON_MODE_FILL: fill the area of the polygon with fragments
	////2. VK_POLYGON_MODE_LINE: polygon edges are drawn as lines 
	////3. VK_POLYGON_MODE_POINT: polygon verticies are drawn as points
	////NOTE: using any other than fill, requires GPU feature
	//rasterizer.polygonMode = vk::PolygonMode::eFill;

	////available line widths, depend on GPU. If above 1.0f, required wideLines GPU feature
	//rasterizer.lineWidth = 1.0f; //measured in fragment widths

	////cullMode : type of face culling to use.
	//rasterizer.cullMode = vk::CullModeFlagBits::eBack;
	//rasterizer.frontFace = vk::FrontFace::eCounterClockwise;

	////depth values can be used in way that is known as 'shadow mapping'. 
	////rasterizer is capable of changing depth values through constant addition or biasing based on frags slope 
	////this is left as off for now 
	//rasterizer.depthBiasEnable = VK_FALSE;
	//rasterizer.depthBiasConstantFactor = 0.0f; //optional 
	//rasterizer.depthBiasClamp = 0.0f; //optional 
	//rasterizer.depthBiasSlopeFactor = 0.0f; //optional

	///* Multisampling */
	////this is one of the methods of performing anti-aliasing
	////enabling requires GPU feature -- left off for this tutorial 
	//vk::PipelineMultisampleStateCreateInfo multisampling{};
	//multisampling.sType = vk::StructureType::ePipelineMultisampleStateCreateInfo;
	//multisampling.sampleShadingEnable = VK_FALSE;
	//multisampling.rasterizationSamples = vk::SampleCountFlagBits::e1;
	//multisampling.minSampleShading = 1.0f; //optional 
	//multisampling.pSampleMask = nullptr; //optional
	//multisampling.alphaToCoverageEnable = VK_FALSE; //optional
	//multisampling.alphaToOneEnable = VK_FALSE; //optional

	///* Depth and Stencil Testing */
	////if using depth or stencil buffer, a depth and stencil tests are neeeded
	//vk::PipelineDepthStencilStateCreateInfo depthStencil{};
	//depthStencil.sType = vk::StructureType::ePipelineDepthStencilStateCreateInfo;
	//depthStencil.depthTestEnable = VK_TRUE;             //specifies if depth of new fragments should be compared to the depth buffer to test for actual display state
	//depthStencil.depthWriteEnable = VK_TRUE;            //specifies if the new depth of fragments that pass the depth tests should be written to the depth buffer 
	//depthStencil.depthCompareOp = vk::CompareOp::eLess;   //comparison that is performed to keep or discard fragments - here this is: lower depth = closer, so depth of new frags should be less
	////following are for optional depth bound testing - keep frags within a specific range 
	//depthStencil.depthBoundsTestEnable = VK_FALSE;
	//depthStencil.minDepthBounds = 0.0f;                 //optional 
	//depthStencil.maxDepthBounds = 1.0f;                 //optional
	////following are used for stencil tests - make sure that format of depth image contains a stencil component
	//depthStencil.stencilTestEnable = VK_FALSE;

	///* Color blending */
	//// after the fragShader has returned a color, it must be combined with the color already in the framebuffer
	//// there are two ways to do this: 
	////      1. mix the old and new value to produce final color
	////      2. combine the old a new value using a bitwise operation 
	////two structs are needed to create this functionality: 
	////  1. VkPipelineColorBlendAttachmentState: configuration per attached framebuffer 
	////  2. VkPipelineColorBlendStateCreateInfo: global configuration
	////only using one framebuffer in this project -- both of these are disabled in this project
	//vk::PipelineColorBlendAttachmentState colorBlendAttachment{};
	////colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	//colorBlendAttachment.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
	//colorBlendAttachment.blendEnable = VK_FALSE;

	//vk::PipelineColorBlendStateCreateInfo colorBlending{};
	//colorBlending.sType = vk::StructureType::ePipelineColorBlendStateCreateInfo;
	//colorBlending.logicOpEnable = VK_FALSE;
	//colorBlending.logicOp = vk::LogicOp::eCopy;
	//colorBlending.attachmentCount = 1;
	//colorBlending.pAttachments = &colorBlendAttachment;
	//colorBlending.blendConstants[0] = 0.0f;
	//colorBlending.blendConstants[1] = 0.0f;
	//colorBlending.blendConstants[2] = 0.0f;
	//colorBlending.blendConstants[3] = 0.0f;

	//std::vector<vk::DescriptorSetLayout> descriptorSetLayouts{ this->globalSetLayout->getDescriptorSetLayout() };
	////create empty descriptor layout information, going to call wihtout vbo
	//vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
	////pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	//pipelineLayoutInfo.sType = vk::StructureType::ePipelineLayoutCreateInfo;
	//pipelineLayoutInfo.setLayoutCount = descriptorSetLayouts.size();
	//pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
	//// pipelineLayoutInfo.pushConstantRangeCount = 1;
	//// pipelineLayoutInfo.pPushConstantRanges = &pushConstant;
	//pipelineLayoutInfo.pushConstantRangeCount = 0;
	//pipelineLayoutInfo.pPushConstantRanges = nullptr;

	//vk::PipelineLayout lightPipeLayout = this->starDevice->getDevice().createPipelineLayout(pipelineLayoutInfo);
	//if (!lightPipeLayout) {
	//	throw std::runtime_error("failed to create pipeline layout");
	//}
	//this->lightRenderSysObj->setPipelineLayout(lightPipeLayout);

	//PipelineConfigSettings config{};
	//config.viewportInfo = viewportState;
	//config.rasterizationInfo = rasterizer;
	//config.multisampleInfo = multisampling;
	//config.depthStencilInfo = depthStencil;
	//config.colorBlendInfo = colorBlending;
	//config.colorBlendAttachment = colorBlendAttachment;
	//config.pipelineLayout = lightPipeLayout;
	//config.renderPass = renderPass;

	//this->lightRenderSysObj->createPipeline(config);
}

vk::ShaderModule star::core::VulkanRenderer::createShaderModule(const std::vector<uint32_t>& code) {
	vk::ShaderModuleCreateInfo createInfo{};
	createInfo.sType = vk::StructureType::eShaderModuleCreateInfo;
	createInfo.codeSize = 4 * code.size();
	createInfo.pCode = code.data();

	VkShaderModule shaderModule = this->starDevice->getDevice().createShaderModule(createInfo);
	if (!shaderModule) {
		throw std::runtime_error("failed to create shader module");
	}

	return shaderModule;
}

void star::core::VulkanRenderer::createDepthResources() {
	//depth image should have:
	//  same resolution as the color attachment (in swap chain extent)
	//  optimal tiling and device local memory 
	//Need to decide format - need to decide format for the accuracy since no direct access to the depth image from CPU side 
	//Formats for color image: 
	//  VK_FORMAT_D32_SFLOAT: 32-bit-float
	//  VK_FORMAT_D32_SFLOAT_S8_UINT: 32-bit signed float for depth and 8 bit stencil component
	//  VK_FORMAT_D24_UNFORM_S8_UINT: 24-bit float for depth and 8 bit stencil component

	vk::Format depthFormat = findDepthFormat();

	createImage(swapChainExtent.width, swapChainExtent.height, depthFormat,
		vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment,
		vk::MemoryPropertyFlagBits::eDeviceLocal, depthImage, depthImageMemory);

	this->depthImageView = createImageView(depthImage, depthFormat, vk::ImageAspectFlagBits::eDepth);
}

void star::core::VulkanRenderer::createImage(uint32_t width, uint32_t height, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage, vk::MemoryPropertyFlagBits properties, vk::Image& image, vk::DeviceMemory& imageMemory) {
	/* Create vulkan image */
	vk::ImageCreateInfo imageInfo{};
	imageInfo.sType = vk::StructureType::eImageCreateInfo;
	imageInfo.imageType = vk::ImageType::e2D;
	imageInfo.extent.width = width;
	imageInfo.extent.height = height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.format = format;
	imageInfo.tiling = tiling;
	imageInfo.initialLayout = vk::ImageLayout::eUndefined;
	imageInfo.usage = usage;
	imageInfo.samples = vk::SampleCountFlagBits::e1;
	imageInfo.sharingMode = vk::SharingMode::eExclusive;

	image = this->starDevice->getDevice().createImage(imageInfo);
	if (!image) {
		throw std::runtime_error("failed to create image");
	}

	/* Allocate the memory for the imag*/
	vk::MemoryRequirements memRequirements = this->starDevice->getDevice().getImageMemoryRequirements(image);

	vk::MemoryAllocateInfo allocInfo{};
	allocInfo.sType = vk::StructureType::eMemoryAllocateInfo;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = this->starDevice->findMemoryType(memRequirements.memoryTypeBits, properties);

	imageMemory = this->starDevice->getDevice().allocateMemory(allocInfo);
	if (!imageMemory) {
		throw std::runtime_error("failed to allocate image memory!");
	}

	this->starDevice->getDevice().bindImageMemory(image, imageMemory, 0);
}

void star::core::VulkanRenderer::createFramebuffers() {
	swapChainFramebuffers.resize(swapChainImageViews.size());

	//iterate through each image and create a buffer for it 
	for (size_t i = 0; i < swapChainImageViews.size(); i++) {
		std::array<vk::ImageView, 2> attachments = {
			swapChainImageViews[i],
			depthImageView //same depth image is going to be used for all swap chain images 
		};

		vk::FramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = vk::StructureType::eFramebufferCreateInfo;
		//make sure that framebuffer is compatible with renderPass (same # and type of attachments)
		framebufferInfo.renderPass = renderPass;
		//specify which vkImageView objects to bind to the attachment descriptions in the render pass pAttachment array
		framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		framebufferInfo.pAttachments = attachments.data();
		framebufferInfo.width = swapChainExtent.width;
		framebufferInfo.height = swapChainExtent.height;
		framebufferInfo.layers = 1; //# of layers in image arrays

		swapChainFramebuffers[i] = this->starDevice->getDevice().createFramebuffer(framebufferInfo);
		if (!swapChainFramebuffers[i]) {
			throw std::runtime_error("failed to create framebuffer");
		}
	}
}

void star::core::VulkanRenderer::createTextureImage() {
	int texWidth;
	int texHeight;
	int texChannels;

	auto currObject = this->objectManager->Get(this->Renderer::objectList->at(0));

	common::Texture* texture = this->textureManager->Get(currObject->getTexture());

	//stbi_uc* pixels = stbi_load(texture->path().c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha); 
	vk::DeviceSize imageSize = texture->width() * texture->height() * 4;

	/* Create Staging Buffer */
	vk::Buffer stagingBuffer;
	vk::DeviceMemory stagingBufferMemory;

	//buffer needs to be in host visible memory
	this->starDevice->createBuffer(imageSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, stagingBuffer, stagingBufferMemory);

	//copy over to staging buffer
	void* data;
	data = this->starDevice->getDevice().mapMemory(stagingBufferMemory, 0, imageSize);
	memcpy(data, texture->data(), static_cast<size_t>(imageSize));
	this->starDevice->getDevice().unmapMemory(stagingBufferMemory);

	//stbi_image_free(pixels);

	createImage(texture->width(), texture->height(), vk::Format::eR8G8B8A8Srgb, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled, vk::MemoryPropertyFlagBits::eDeviceLocal, textureImage, textureImageMemory);

	//copy staging buffer to texture image 
	transitionImageLayout(textureImage, vk::Format::eR8G8B8A8Srgb, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);

	this->starDevice->copyBufferToImage(stagingBuffer, textureImage, static_cast<uint32_t>(texture->width()), static_cast<uint32_t>(texture->height()));

	//prepare final image for texture mapping in shaders 
	transitionImageLayout(textureImage, vk::Format::eR8G8B8A8Srgb, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);

	this->starDevice->getDevice().destroyBuffer(stagingBuffer);
	this->starDevice->getDevice().freeMemory(stagingBufferMemory);
}

void star::core::VulkanRenderer::transitionImageLayout(vk::Image image, vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout) {
	vk::CommandBuffer commandBuffer = this->starDevice->beginSingleTimeCommands();

	//create a barrier to prevent pipeline from moving forward until image transition is complete
	vk::ImageMemoryBarrier barrier{};
	//barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;     //specific flag for image operations
	barrier.sType = vk::StructureType::eImageMemoryBarrier;     //specific flag for image operations
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;

	//if barrier is used for transferring ownership between queue families, this would be important -- set to ignore since we are not doing this
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

	barrier.image = image;
	//barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
	barrier.subresourceRange.baseMipLevel = 0;                          //image does not have any mipmap levels
	barrier.subresourceRange.levelCount = 1;                            //image is not an array
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

	//the operations that need to be completed before and after the barrier, need to be defined
	barrier.srcAccessMask = {}; //TODO
	barrier.dstAccessMask = {}; //TODO

	vk::PipelineStageFlags sourceStage, destinationStage;

	if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal) {
		//undefined transition state, dont need to wait for this to complete
		barrier.srcAccessMask = {};
		//barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

		sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
		destinationStage = vk::PipelineStageFlagBits::eTransfer;
	}
	else if (oldLayout == vk::ImageLayout::eTransferDstOptimal && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
		//transfer destination shdaer reading, will need to wait for completion. Especially in the frag shader where reads will happen
		barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
		barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

		sourceStage = vk::PipelineStageFlagBits::eTransfer;
		destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
	}
	else {
		throw std::invalid_argument("unsupported layout transition!");
	}

	//transfer writes must occurr during the pipeline transfer stage
	commandBuffer.pipelineBarrier(
		sourceStage,                        //which pipeline stages should occurr before barrier 
		destinationStage,                   //pipeline stage in which operations iwll wait on the barrier 
		{},
		{},
		nullptr,
		barrier
	);

	this->starDevice->endSingleTimeCommands(commandBuffer);
}

void star::core::VulkanRenderer::createTextureImageView() {
	//this->textureImageView = createImageView(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
	this->textureImageView = createImageView(textureImage, vk::Format::eR8G8B8A8Srgb, vk::ImageAspectFlagBits::eColor);
}

void star::core::VulkanRenderer::createTextureSampler() {
	//get device properties for amount of anisotropy permitted
	vk::PhysicalDeviceProperties deviceProperties = this->starDevice->getPhysicalDevice().getProperties();

	vk::SamplerCreateInfo samplerInfo{};
	samplerInfo.sType = vk::StructureType::eSamplerCreateInfo;
	samplerInfo.magFilter = vk::Filter::eLinear;                       //how to sample textures that are magnified 
	samplerInfo.minFilter = vk::Filter::eLinear;                       //how to sample textures that are minified

	//repeat mode - repeat the texture when going beyond the image dimensions
	samplerInfo.addressModeU = vk::SamplerAddressMode::eRepeat;
	samplerInfo.addressModeV = vk::SamplerAddressMode::eRepeat;
	samplerInfo.addressModeW = vk::SamplerAddressMode::eRepeat;

	//should anisotropic filtering be used? Really only matters if performance is a concern
	samplerInfo.anisotropyEnable = VK_TRUE;
	//specifies the limit on the number of texel samples that can be used (lower = better performance)
	samplerInfo.maxAnisotropy = deviceProperties.limits.maxSamplerAnisotropy;;
	samplerInfo.borderColor = vk::BorderColor::eIntOpaqueBlack;
	//specifies coordinate system to use in addressing texels. 
		//VK_TRUE - use coordinates [0, texWidth) and [0, texHeight]
		//VK_FALSE - use [0, 1)
	samplerInfo.unnormalizedCoordinates = VK_FALSE;

	//if comparing, the texels will first compare to a value, the result of the comparison is used in filtering operations (percentage-closer filtering on shadow maps)
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = vk::CompareOp::eAlways;

	//following apply to mipmapping -- not using here
	samplerInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = 0.0f;
	samplerInfo.anisotropyEnable = VK_FALSE;

	this->textureSampler = this->starDevice->getDevice().createSampler(samplerInfo);
	if (!this->textureSampler) {
		throw std::runtime_error("failed to create texture sampler!");
	}
}

void star::core::VulkanRenderer::createRenderingBuffers() {
	RenderSysObj* tmpRenderSysObj = this->RenderSysObjs.at(0).get();
	//vk::DeviceSize uboBufferSize = sizeof(RenderSysObj::UniformBufferObject) * tmpRenderSysObj->getNumRenderObjects();
	vk::DeviceSize globalBufferSize = sizeof(GlobalUniformBufferObject) * tmpRenderSysObj->getNumRenderObjects();

	//this->uniformBuffers.resize(this->swapChainImages.size()); 
	this->globalUniformBuffers.resize(this->swapChainImages.size());

	for (size_t i = 0; i < swapChainImages.size(); i++) {
		this->globalUniformBuffers[i] = std::make_unique<StarBuffer>(this->starDevice.get(), tmpRenderSysObj->getNumRenderObjects(), sizeof(GlobalUniformBufferObject),
			vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
		this->globalUniformBuffers[i]->map();

		//this->uniformBuffers[i] = std::make_unique<StarBuffer>(this->starDevice.get(),tmpRenderSysObj->getNumRenderObjects(), sizeof(RenderSysObj::UniformBufferObject),
		//	vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
		//this->uniformBuffers[i]->map(); 
	}

}

void star::core::VulkanRenderer::createCommandBuffers() {

	for (auto& RenderSysObj : this->RenderSysObjs) {
		/* Graphics Command Buffer */
		this->starDevice->getGraphicsCommandBuffers()->resize(swapChainFramebuffers.size());

		vk::CommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = vk::StructureType::eCommandBufferAllocateInfo;
		allocInfo.commandPool = this->starDevice->getGraphicsCommandPool();
		// .level - specifies if the allocated command buffers are primay or secondary
		// ..._PRIMARY : can be submitted to a queue for execution, but cannot be called from other command buffers
		// ..._SECONDARY : cannot be submitted directly, but can be called from primary command buffers (good for reuse of common operations)
		allocInfo.level = vk::CommandBufferLevel::ePrimary;
		allocInfo.commandBufferCount = (uint32_t)starDevice->getGraphicsCommandBuffers()->size();

		std::vector<vk::CommandBuffer> newBuffers = this->starDevice->getDevice().allocateCommandBuffers(allocInfo);
		newBuffers = this->starDevice->getDevice().allocateCommandBuffers(allocInfo);
		if (newBuffers.size() == 0) {
			throw std::runtime_error("failed to allocate command buffers");
		}

		if (this->RenderSysObjs.size() > 1) {
			throw std::runtime_error("More than one shader group is not yet supported");
		}
		core::RenderSysObj* tmpRenderSysObj = this->RenderSysObjs.at(0).get();

		/* Begin command buffer recording */
		for (size_t i = 0; i < newBuffers.size(); i++) {
			vk::CommandBufferBeginInfo beginInfo{};
			//beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.sType = vk::StructureType::eCommandBufferBeginInfo;

			//flags parameter specifies command buffer use 
				//VK_COMMAND_BUFFER_USEAGE_ONE_TIME_SUBMIT_BIT: command buffer recorded right after executing it once
				//VK_COMMAND_BUFFER_USEAGE_RENDER_PASS_CONTINUE_BIT: secondary command buffer that will be within a single render pass 
				//VK_COMMAND_BUFFER_USEAGE_SIMULTANEOUS_USE_BIT: command buffer can be resubmitted while another instance has already been submitted for execution
			beginInfo.flags = {};

			//only relevant for secondary command buffers -- which state to inherit from the calling primary command buffers 
			beginInfo.pInheritanceInfo = nullptr;

			/* NOTE:
				if the command buffer has already been recorded once, simply call vkBeginCommandBuffer->implicitly reset.
				commands cannot be added after creation
			*/

			newBuffers[i].begin(beginInfo);
			if (!newBuffers[i]) {
				throw std::runtime_error("failed to begin recording command buffer");
			}

			vk::Viewport viewport{};
			viewport.x = 0.0f;
			viewport.y = 0.0f;
			viewport.width = (float)this->swapChainExtent.width;
			viewport.height = (float)this->swapChainExtent.height;
			//Specify values range of depth values to use for the framebuffer. If not doing anything special, leave at default
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;

			newBuffers[i].setViewport(0, viewport);


			/* Begin render pass */
			//drawing starts by beginning a render pass 
			vk::RenderPassBeginInfo renderPassInfo{};
			//renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.sType = vk::StructureType::eRenderPassBeginInfo;

			//define the render pass we want
			renderPassInfo.renderPass = renderPass;

			//what attachments do we need to bind
			//previously created swapChainbuffers to hold this information 
			renderPassInfo.framebuffer = swapChainFramebuffers[i];

			//define size of render area -- should match size of attachments for best performance
			renderPassInfo.renderArea.offset = vk::Offset2D{ 0, 0 };
			renderPassInfo.renderArea.extent = swapChainExtent;

			//size of clearValues and order, should match the order of attachments
			std::array<vk::ClearValue, 2> clearValues{};
			clearValues[0].color = std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f }; //clear color for background color will be used with VK_ATTACHMENT_LOAD_OP_CLEAR
			//depth values: 
				//0.0 - closest viewing plane 
				//1.0 - furthest possible depth
			clearValues[1].depthStencil = vk::ClearDepthStencilValue{ 1.0, 0 };

			renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
			renderPassInfo.pClearValues = clearValues.data();

			/* vkCmdBeginRenderPass */
			//Args: 
				//1. command buffer to set recording to 
				//2. details of the render pass
				//3. how drawing commands within the render pass will be provided
					//OPTIONS: 
						//VK_SUBPASS_CONTENTS_INLINE: render pass commands will be embedded in the primary command buffer. No secondary command buffers executed 
						//VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS: render pass commands will be executed from the secondary command buffers
			newBuffers[i].beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);

			/* Drawing Commands */
			//Args: 
				//2. compute or graphics pipeline
				//3. pipeline object
			//vkCmdBindPipeline(graphicsCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
			//newBuffers[i].bindPipeline(vk::PipelineBindPoint::eGraphics, RenderSysObj->getPipeline());

			//vk::Buffer vertexBuffers[] = { RenderSysObj->vertexBuffer };
			//TODO: need to allow for an offset for each buffer


			tmpRenderSysObj->bind(newBuffers[i]);



			//bind vertex buffers -> how to pass information to the vertex shader once it is uploaded to the GPU
			//newBuffers[i].bindVertexBuffers(0, RenderSysObj->vertexBuffer, offsets);

			/* vkCmdBindDescriptorSets:
			*   1.
			*   2. Descriptor sets are not unique to graphics pipeliens, must specify to use in graphics or compute pipelines.
			*   3. layout that the descriptors are based on
			*   4. index of first descriptor set
			*   5. number of sets to bind
			*   6. array of sets to bind
			*   7 - 8. array of offsets used for dynamic descriptors (not used here)
			*/
			//bind the right descriptor set for each swap chain image to the descripts in the shader
			//bind global descriptor
			newBuffers[i].bindDescriptorSets(vk::PipelineBindPoint::eGraphics, tmpRenderSysObj->getPipelineLayout(), 0, 1, &this->globalDescriptorSets.at(i), 0, nullptr);
			tmpRenderSysObj->render(newBuffers[i], i);

			//bind light pipe 
			this->lightRenderSys->bind(newBuffers[i]);
			this->lightRenderSys->render(newBuffers[i], i);

			newBuffers[i].endRenderPass();

			//record command buffer
			newBuffers[i].end();
		}

		this->starDevice->setGraphicsCommandBuffers(newBuffers);
	}
}

void star::core::VulkanRenderer::createSemaphores() {
	imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);

	vk::SemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = vk::StructureType::eSemaphoreCreateInfo;

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		this->imageAvailableSemaphores[i] = this->starDevice->getDevice().createSemaphore(semaphoreInfo);
		this->renderFinishedSemaphores[i] = this->starDevice->getDevice().createSemaphore(semaphoreInfo);

		if (!this->imageAvailableSemaphores[i]) {
			throw std::runtime_error("failed to create semaphores for a frame");
		}
	}
}

void star::core::VulkanRenderer::createFences() {
	//note: fence creation can be rolled into semaphore creation. Seperated for understanding
	inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

	vk::FenceCreateInfo fenceInfo{};
	fenceInfo.sType = vk::StructureType::eFenceCreateInfo;

	//create the fence in a signaled state 
	//fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
	fenceInfo.flags = vk::FenceCreateFlagBits::eSignaled;

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		this->inFlightFences[i] = this->starDevice->getDevice().createFence(fenceInfo);
		if (!this->inFlightFences[i]) {
			throw std::runtime_error("failed to create fence object for a frame");
		}
	}
}

void star::core::VulkanRenderer::createFenceImageTracking() {
	//note: just like createFences() this too can be wrapped into semaphore creation. Seperated for understanding.

	//need to ensure the frame that is going to be drawn to, is the one linked to the expected fence.
	//If, for any reason, vulkan returns an image out of order, we will be able to handle that with this link
	imagesInFlight.resize(swapChainImages.size(), VK_NULL_HANDLE);

	//initially, no frame is using any image so this is going to be created without an explicit link
}
