#include "BasicVulkanRenderer.h"

typedef std::chrono::high_resolution_clock Clock;

namespace star::core {
	VulkanRenderer::VulkanRenderer(common::ConfigFile& configFile, common::RenderOptions& renderOptions,
		common::FileResourceManager<common::Shader>& shaderManager, common::FileResourceManager<common::GameObject>& objectManager,
		TextureManager& textureManager, MapManager& mapManager, 
		MaterialManager& materialManager, common::Camera& inCamera,
		std::vector<common::Handle>& objectHandleList, std::vector<common::Light*>& inLightList,
		StarWindow& window) :
		materialManager(materialManager), textureManager(textureManager), 
		mapManager(mapManager), star::common::Renderer(configFile, renderOptions, shaderManager, objectManager, inCamera, objectHandleList),
		starWindow(window), lightList(inLightList)
	{
		common::GameObject* currentObject = nullptr;
		common::Light* currLight = nullptr;
		this->starDevice = std::unique_ptr<StarDevice>(new StarDevice(this->starWindow));
	}

	VulkanRenderer::~VulkanRenderer() {
		this->starDevice->getDevice().waitIdle();
		cleanup();
	}

	void VulkanRenderer::updateUniformBuffer(uint32_t currentImage) {
		static auto startTime = std::chrono::high_resolution_clock::now();

		auto currentTime = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

		//update global ubo 
		GlobalUniformBufferObject globalUbo;
		globalUbo.proj = glm::perspective(glm::radians(45.0f), swapChainExtent.width / (float)swapChainExtent.height, 0.1f, 10.0f);
		//glm designed for openGL where the Y coordinate of the flip coordinates is inverted. Fix this by flipping the sign on the scaling factor of the Y axis in the projection matrix.
		globalUbo.proj[1][1] *= -1;
		globalUbo.view = this->camera.getDisplayMatrix();
		globalUbo.inverseView = this->camera.getInverseViewMatrix(); 
		globalUbo.numLights = static_cast<uint32_t>(this->lightList.size()); 
		globalUbo.renderOptions = this->renderOptions.getRenderOptions(); 

		this->globalUniformBuffers[currentImage]->writeToBuffer(&globalUbo, sizeof(globalUbo));

		//update buffer for light positions
		//std::vector<glm::vec4> lightPositions(this->lightList.size());
		//std::vector<glm::vec4> lightColors(this->lightList.size());
		std::vector<LightBufferObject> lightInformation(this->lightList.size()); 
		LightBufferObject newBufferObject{};
		common::Light* currLight = nullptr; 
		
		//write buffer information
		for (size_t i = 0; i < this->lightList.size(); i++) {
			currLight = this->lightList.at(i); 
			newBufferObject.position = glm::vec4{ currLight->getPosition(), 1.0f };
			newBufferObject.specular = currLight->getSpecular();
			newBufferObject.ambient = currLight->getAmbient();
			newBufferObject.diffuse = currLight->getDiffuse();
			newBufferObject.specular = currLight->getSpecular();
			lightInformation[i] = newBufferObject; 
		}
		this->lightBuffers[currentImage]->writeToBuffer(lightInformation.data(), sizeof(LightBufferObject) * lightInformation.size());

		for (size_t i = 0; i < this->RenderSysObjs.size(); i++) {
			RenderSysObjs.at(i)->updateBuffers(currentImage);
		}

		this->lightRenderSys->updateBuffers(currentImage); 
	}


	void VulkanRenderer::pollEvents() {
		glfwPollEvents();
	}

	void VulkanRenderer::prepare() {
		createSwapChain();
		createImageViews();
		createRenderPass();

		this->globalPool = StarDescriptorPool::Builder(*this->starDevice.get())
			.setMaxSets((this->swapChainImages.size()))
			.addPoolSize(vk::DescriptorType::eUniformBuffer, this->swapChainImages.size())
			.addPoolSize(vk::DescriptorType::eStorageBuffer, this->lightList.size())
			.build();

		this->globalSetLayout = StarDescriptorSetLayout::Builder(*this->starDevice.get())
			.addBinding(0, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment)
			.addBinding(1, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment)
			.build();

		this->globalDescriptorSets.resize(this->swapChainImages.size());


		vk::ShaderStageFlagBits stages{};
		vk::Device device = this->starDevice->getDevice();
		this->RenderSysObjs.push_back(std::make_unique<RenderSysObj>(*this->starDevice, this->swapChainImages.size(), this->globalSetLayout->getDescriptorSetLayout(), this->swapChainExtent, this->renderPass));
		RenderSysObj* tmpRenderSysObj = this->RenderSysObjs.at(0).get();
		uint32_t meshVertCounter = 0; 

		for (size_t i = 0; i < this->objectList.size(); i++) {
			common::GameObject& currObject = this->objectManager.get(this->objectList.at(i));
			meshVertCounter = 0;

			//check if the vulkan object has a shader registered for the desired stage that is different than the one needed for the current object
			for (size_t j = 0; j < this->RenderSysObjs.size(); j++) {
				//check if object shaders are in vulkan object
				RenderSysObj* object = this->RenderSysObjs.at(j).get();
				if (!object->hasShader(vk::ShaderStageFlagBits::eVertex) && (!object->hasShader(vk::ShaderStageFlagBits::eFragment))) {
					//vulkan object does not have either a vertex or a fragment shader 
					object->registerShader(vk::ShaderStageFlagBits::eVertex, this->shaderManager.get(currObject.getVertShader()), currObject.getVertShader());
					object->registerShader(vk::ShaderStageFlagBits::eFragment, this->shaderManager.get(currObject.getFragShader()), currObject.getFragShader());
					RenderObject::Builder builder(*this->starDevice, currObject);
					builder.setNumFrames(this->swapChainImages.size()); 

					for (auto& mesh : currObject.getMeshes()) {
						builder.addMesh(
							RenderMesh::Builder(*this->starDevice)
								.setMesh(*mesh)
								.setRenderSettings(object->getNumVerticies() + meshVertCounter)
								.setMaterial(RenderMaterial::Builder(*this->starDevice, this->materialManager, this->textureManager, this->mapManager)
									.setMaterial(mesh->getMaterial())
									.build())
								.build());
						meshVertCounter += mesh->getTriangles()->size() * 3; 
					}
					object->addObject(std::move(builder.build()));
				}
				else if ((object->getBaseShader(vk::ShaderStageFlagBits::eVertex).containerIndex != currObject.getVertShader().containerIndex) ||
					(object->getBaseShader(vk::ShaderStageFlagBits::eFragment).containerIndex != currObject.getFragShader().containerIndex)) {
					//vulkan object has shaders but they are not the same as the shaders needed for current render object
					this->RenderSysObjs.push_back(std::make_unique<RenderSysObj>(*this->starDevice, this->swapChainImages.size(), this->globalSetLayout->getDescriptorSetLayout(), this->swapChainExtent, this->renderPass));
					RenderSysObj* newObject = this->RenderSysObjs.at(this->RenderSysObjs.size()).get();
					newObject->registerShader(vk::ShaderStageFlagBits::eVertex, this->shaderManager.get(currObject.getVertShader()), currObject.getVertShader());
					newObject->registerShader(vk::ShaderStageFlagBits::eFragment, this->shaderManager.get(currObject.getFragShader()), currObject.getFragShader());
					newObject->addObject(std::move(RenderObject::Builder(*this->starDevice, currObject)
						.setNumFrames(this->swapChainImages.size())
						.build()));
				}
				else {
					//vulkan object has the same shaders as the render object 
					RenderObject::Builder builder(*this->starDevice, currObject);
					builder.setNumFrames(this->swapChainImages.size());

					for (auto& mesh : currObject.getMeshes()) {
						builder.addMesh(RenderMesh::Builder(*this->starDevice)
							.setMesh(*mesh)
							.setRenderSettings(object->getNumVerticies() + meshVertCounter)
							.setMaterial(RenderMaterial::Builder(*this->starDevice, this->materialManager, this->textureManager, this->mapManager)
								.setMaterial(mesh->getMaterial())
								.build())
							.build());
						meshVertCounter += mesh->getTriangles()->size() * 3;
					}
					object->addObject(builder.build());
				}
			}
		}
		std::vector<vk::DescriptorSetLayout> globalSets = { this->globalSetLayout->getDescriptorSetLayout() }; 
		tmpRenderSysObj->init(globalSets);

		/* Init Point Light Render System */
		this->lightRenderSys = std::make_unique<RenderSysPointLight>(*this->starDevice, this->swapChainImages.size(), this->globalSetLayout->getDescriptorSetLayout(), this->swapChainExtent, this->renderPass);
		common::GameObject* currLinkedObj = nullptr; 
		int vertexCounter = 0; 
		for (auto light : this->lightList) {
			if (light->hasLinkedObject()) {
				currLinkedObj = &this->objectManager.get(light->getLinkedObjectHandle());
				if (!this->lightRenderSys->hasShader(vk::ShaderStageFlagBits::eVertex) && !this->lightRenderSys->hasShader(vk::ShaderStageFlagBits::eFragment)) {
					this->lightRenderSys->registerShader(vk::ShaderStageFlagBits::eVertex, this->shaderManager.getResource(currLinkedObj->getVertShader()), currLinkedObj->getVertShader()); 
					this->lightRenderSys->registerShader(vk::ShaderStageFlagBits::eFragment, this->shaderManager.getResource(currLinkedObj->getFragShader()), currLinkedObj->getFragShader()); 
				}
				if ((lightRenderSys->getBaseShader(vk::ShaderStageFlagBits::eFragment).containerIndex == currLinkedObj->getFragShader().containerIndex)
						|| (lightRenderSys->getBaseShader(vk::ShaderStageFlagBits::eVertex).containerIndex == currLinkedObj->getVertShader().containerIndex)) {
					auto builder = RenderObject::Builder(*this->starDevice, this->objectManager.get(light->getLinkedObjectHandle()));
					builder.setNumFrames(this->swapChainImages.size());

					for (auto& mesh : currLinkedObj->getMeshes()) {
						builder.addMesh(RenderMesh::Builder(*this->starDevice)
							.setMesh(*mesh)
							.setRenderSettings(vertexCounter)
							.setMaterial(RenderMaterial::Builder(*this->starDevice, this->materialManager, this->textureManager, this->mapManager)
								.setMaterial(mesh->getMaterial())
								.build())
							.build());
						vertexCounter += mesh->getTriangles()->size() * 3; 
					}
					lightRenderSys->addLight(light, builder.build(), this->swapChainImages.size());
				}
				else {
					throw std::runtime_error("More than one shader type is not permitted for light linked object");
				}
			}
		}
		this->lightRenderSys->setPipelineLayout(this->RenderSysObjs.at(0)->getPipelineLayout()); 
		this->lightRenderSys->init(globalSets); 

		createDepthResources();
		createFramebuffers();
		createRenderingBuffers();

		std::unique_ptr<std::vector<vk::DescriptorBufferInfo>> bufferInfos{};
		for (size_t i = 0; i < this->swapChainImages.size(); i++) {
			//global
			bufferInfos = std::make_unique<std::vector<vk::DescriptorBufferInfo>>();

			auto globalBufferInfo = vk::DescriptorBufferInfo{
				this->globalUniformBuffers[i]->getBuffer(),
				0,
				sizeof(GlobalUniformBufferObject)};

			//buffer descriptors for point light locations 
			auto lightBufferInfo = vk::DescriptorBufferInfo{
				this->lightBuffers[i]->getBuffer(),
				0,
				sizeof(LightBufferObject) * this->lightList.size()};

			StarDescriptorWriter(*this->starDevice.get(), *this->globalSetLayout, *this->globalPool)
				.writeBuffer(0, &globalBufferInfo)
				.writeBuffer(1, &lightBufferInfo)
				.build(this->globalDescriptorSets.at(i));
		}

		createCommandBuffers();
		createSemaphores();
		createFences();
		createFenceImageTracking();
	}

	void VulkanRenderer::draw() {
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

	void VulkanRenderer::cleanup() {
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

	void VulkanRenderer::cleanupSwapChain() {
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

	void VulkanRenderer::createSwapChain() {
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
		std::vector<uint32_t> queueFamilyIndicies;
		if (indicies.transferFamily.has_value())
			 queueFamilyIndicies = std::vector<uint32_t>{ indicies.graphicsFamily.value(), indicies.transferFamily.value(), indicies.presentFamily.value() };
		else
			 queueFamilyIndicies = std::vector<uint32_t>{ indicies.graphicsFamily.value(), indicies.presentFamily.value() };

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
			createInfo.pQueueFamilyIndices = queueFamilyIndicies.data();
		}
		else if (indicies.graphicsFamily != indicies.presentFamily && indicies.presentFamily == indicies.transferFamily) {
			uint32_t explicitQueueFamilyInd[] = { indicies.graphicsFamily.value(), indicies.presentFamily.value() };
			createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndicies.data();
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

	void VulkanRenderer::recreateSwapChain() {
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

		createDepthResources();

		createFramebuffers();

		//uniform buffers are dependent on the number of swap chain images, will need to recreate since they are destroyed in cleanupSwapchain()
		createRenderingBuffers();

		//createDescriptorPool();

		//createDescriptorSets();

		createCommandBuffers();
	}

	vk::SurfaceFormatKHR VulkanRenderer::chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats) {
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

	vk::PresentModeKHR VulkanRenderer::chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes) {
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

	vk::Extent2D VulkanRenderer::chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities) {
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

	void VulkanRenderer::createImageViews() {
		swapChainImageViews.resize(swapChainImages.size());

		//need to create an imageView for each of the images available
		for (size_t i = 0; i < swapChainImages.size(); i++) {
			//swapChainImageViews[i] = createImageView(swapChainImages[i], swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
			swapChainImageViews[i] = createImageView(swapChainImages[i], swapChainImageFormat, vk::ImageAspectFlagBits::eColor);
		}
	}

	vk::ImageView VulkanRenderer::createImageView(vk::Image image, vk::Format format, vk::ImageAspectFlagBits aspectFlags) {
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

	void VulkanRenderer::createRenderPass() {
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

	vk::Format VulkanRenderer::findDepthFormat() {
		//utilizing the VK_FORMAT_FEATURE_ flag to check for candidates that have a depth component.
		return this->starDevice->findSupportedFormat(
			{ vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint },
			vk::ImageTiling::eOptimal,
			vk::FormatFeatureFlagBits::eDepthStencilAttachment);
	}

	vk::ShaderModule VulkanRenderer::createShaderModule(const std::vector<uint32_t>& code) {
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

	void VulkanRenderer::createDepthResources() {
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

	void VulkanRenderer::createImage(uint32_t width, uint32_t height, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage, vk::MemoryPropertyFlagBits properties, vk::Image& image, vk::DeviceMemory& imageMemory) {
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

	void VulkanRenderer::createFramebuffers() {
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

	void VulkanRenderer::createRenderingBuffers() {
		RenderSysObj* tmpRenderSysObj = this->RenderSysObjs.at(0).get();
		vk::DeviceSize globalBufferSize = sizeof(GlobalUniformBufferObject) * tmpRenderSysObj->getNumRenderObjects();

		this->globalUniformBuffers.resize(this->swapChainImages.size());
		if (this->lightList.size() > 0) {
			this->lightBuffers.resize(this->swapChainImages.size()); 
		}

		for (size_t i = 0; i < swapChainImages.size(); i++) {
			this->globalUniformBuffers[i] = std::make_unique<StarBuffer>(*this->starDevice.get(), tmpRenderSysObj->getNumRenderObjects(), sizeof(GlobalUniformBufferObject),
				vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
			this->globalUniformBuffers[i]->map();

			//create light buffers 
			if (this->lightList.size() > 0) {
				this->lightBuffers[i] = std::make_unique<StarBuffer>(*this->starDevice, this->lightList.size(), sizeof(LightBufferObject) * this->lightList.size(),
					vk::BufferUsageFlagBits::eStorageBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent); 
				this->lightBuffers[i]->map();
			}
		}
	}

	void VulkanRenderer::createCommandBuffers() {

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

				tmpRenderSysObj->bind(newBuffers[i]);

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

	void VulkanRenderer::createSemaphores() {
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

	void VulkanRenderer::createFences() {
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

	void VulkanRenderer::createFenceImageTracking() {
		//note: just like createFences() this too can be wrapped into semaphore creation. Seperated for understanding.

		//need to ensure the frame that is going to be drawn to, is the one linked to the expected fence.
		//If, for any reason, vulkan returns an image out of order, we will be able to handle that with this link
		imagesInFlight.resize(swapChainImages.size(), VK_NULL_HANDLE);

		//initially, no frame is using any image so this is going to be created without an explicit link
	}
}