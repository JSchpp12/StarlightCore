#include "VulkanRenderer.h"

typedef std::chrono::high_resolution_clock Clock;

star::core::VulkanRenderer::VulkanRenderer(common::ConfigFile* configFile,
    common::FileResourceManager<common::Shader>* shaderManager,
    common::FileResourceManager<common::GameObject>* objectManager,
    common::FileResourceManager<common::Texture>* textureManager,
    common::Camera* inCamera,
    std::vector<common::Handle>* objectHandleList) :
    star::common::Renderer(configFile, shaderManager, objectManager, textureManager, inCamera, objectHandleList),
    glfwRequiredExtensionsCount(new uint32_t)
{
    common::GameObject* currentObject;

    //TODO: check data before creating needed objects -- ensure all objects are valid  
    //find out how many unique vulkan objects will be needed 
}

star::core::VulkanRenderer::~VulkanRenderer() {
    this->device.waitIdle();
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
    //globalUbo.view = glm::lookAt(glm::vec3(3.0f, 3.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));

    void* globalData = this->device.mapMemory(this->globalUniformBuffersMemory[currentImage], 0, sizeof(GlobalUniformBufferObject));
    memcpy(globalData, &globalUbo, sizeof(globalUbo)); 
    this->device.unmapMemory(globalUniformBuffersMemory[currentImage]);

    //update per object data

    VulkanObject* tmpVulkanObject = this->vulkanObjects.at(0).get();
    common::GameObject* currObject = nullptr;

    std::vector<UniformBufferObject> ubos; 
    ubos.resize(tmpVulkanObject->getNumRenderObjects());

    currObject = this->objectManager->Get(tmpVulkanObject->getRenderObjectAt(0)->getHandle());


    //UniformBufferObject obj; 
    //obj.model = currObject->getDisplayMatrix(); 
    ////obj.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));

    //void* data = this->device.mapMemory(this->uniformBuffersMemory[currentImage], 0, sizeof(UniformBufferObject));
    //memcpy(data, &obj, sizeof(obj));
    //this->device.unmapMemory(uniformBuffersMemory[currentImage]); 

    std::unique_ptr<UniformBufferObject> newBufferObject; 
    auto test = tmpVulkanObject->getNumRenderObjects();
    for (size_t i = 0; i < tmpVulkanObject->getNumRenderObjects(); i++) {
        newBufferObject = std::make_unique<UniformBufferObject>(UniformBufferObject());

        RenderObject* currRenderObject = tmpVulkanObject->getRenderObjectAt(i);
        currObject = this->objectManager->Get(currRenderObject->getHandle()); 
            //this->objectManager->Get(tmpVulkanObject->getObjectHandleAt(i));
        //glm::mat4(1,0f) = identity matrix
        //time * radians(90) = rotate 90degrees per second
        
        newBufferObject->modelMatrix = currObject->getDisplayMatrix();
        newBufferObject->normalMatrix = currObject->getNormalMatrix(); 

        //look at geometry from above at 45 degree angle 
        /* LookAt takes:
        *   1. eye position
        *   2. center position
        *   3. up axis
        */


        //perspective projection with 45 degree vertical field of view -- important to use swapChainExtent to calculate aspect ratio (REFRESH WITH WINDOW RESIZE)
        /* perspective takes:
        *   1. fov
        *   2. aspect ratio
        *   3. near view plane
        *   4. far view plane
        */
        //copy data to the current uniform buffer 

        ubos.at(i) = *newBufferObject;

    }

    auto tmp1 = sizeof(UniformBufferObject);
    auto tmp = sizeof(ubos) * ubos.size();
    void* data = this->device.mapMemory(this->uniformBuffersMemory[currentImage], 0, sizeof(UniformBufferObject) * tmpVulkanObject->getNumRenderObjects());
    memcpy(data, ubos.data(), sizeof(UniformBufferObject) * ubos.size());
    this->device.unmapMemory(uniformBuffersMemory[currentImage]);
}

void star::core::VulkanRenderer::prepareGLFW(int width, int height, GLFWkeyfun keyboardCallbackFunction, GLFWmousebuttonfun mouseButtonCallback, GLFWcursorposfun cursorPositionCallback, GLFWscrollfun scrollCallback) {
    //actually make sure to init glfw
    glfwInit();
    //tell GLFW to create a window but to not include a openGL instance as this is a default behavior
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    //disable resizing functionality in glfw as this will not be handled in the first tutorial
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    //create a window, 3rd argument allows selection of monitor, 4th argument only applies to openGL
    this->glfwWindow = glfwCreateWindow(width, height, "Vulkan", nullptr, nullptr);

    //need to give GLFW a pointer to current instance of this class
    glfwSetWindowUserPointer(this->glfwWindow, this);

    // glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);

    //set keyboard callbacks
    auto callback = glfwSetKeyCallback(this->glfwWindow, keyboardCallbackFunction);

    auto cursorCallback = glfwSetCursorPosCallback(this->glfwWindow, cursorPositionCallback);

    auto mouseBtnCallback = glfwSetMouseButtonCallback(this->glfwWindow, mouseButtonCallback);

    auto mouseScrollCallback = glfwSetScrollCallback(this->glfwWindow, scrollCallback);

    // this->glfwRequiredExtensions = std::make_unique<std::vector<vk::ExtensionProperties>>(new std::vector<vk::ExtensionProperties>(**requiredExtensions)); 
    this->glfwRequiredExtensions = std::make_unique<const char**>(glfwGetRequiredInstanceExtensions(this->glfwRequiredExtensionsCount.get()));

    createInstance();

    VkSurfaceKHR surfaceTmp;
    if (glfwCreateWindowSurface(this->instance, this->glfwWindow, nullptr, &surfaceTmp) != VK_SUCCESS) {
        throw std::runtime_error("failed to create window surface");
    }
    this->surface = new vk::UniqueSurfaceKHR(surfaceTmp, this->instance);
}

bool star::core::VulkanRenderer::shouldCloseWindow() {
    return glfwWindowShouldClose(this->glfwWindow);
}

void star::core::VulkanRenderer::pollEvents() {
    glfwPollEvents();
}

void star::core::VulkanRenderer::prepare() {
    //init vulkan 
    pickPhysicalDevice();
    createLogicalDevice();
    createSwapChain();

    createImageViews();
    createRenderPass();

    common::GameObject* currObject = nullptr; 
    vk::ShaderStageFlagBits stages{};
    this->vulkanObjects.push_back(std::make_unique<VulkanObject>(this->device, this->swapChainImages.size()));
    VulkanObject* tmpVulkanObject = this->vulkanObjects.at(0).get();

    for (size_t i = 0; i < this->objectList->size(); i++) {
        currObject = this->objectManager->Get(this->objectList->at(i));

        this->numVerticies += currObject->getVerticies()->size();
        this->numIndicies += currObject->getIndicies()->size();

        //check if the vulkan object has a shader registered for the desired stage that is different than the one needed for the current object
        for (size_t j = 0; j < this->vulkanObjects.size(); j++) {
            //check if object shaders are in vulkan object
            VulkanObject* object = this->vulkanObjects.at(j).get(); 
            if (!object->hasShader(vk::ShaderStageFlagBits::eVertex) && (!object->hasShader(vk::ShaderStageFlagBits::eFragment))) {
                //vulkan object does not have either a vertex or a fragment shader 
                object->registerShader(vk::ShaderStageFlagBits::eVertex, currObject->getVertShader());
                object->registerShader(vk::ShaderStageFlagBits::eFragment, currObject->getFragShader());
                object->addObject(this->objectList->at(i), currObject, this->swapChainImages.size());
            }
            else if ((object->getBaseShader(vk::ShaderStageFlagBits::eVertex).containerIndex != currObject->getVertShader().containerIndex) ||
                (object->getBaseShader(vk::ShaderStageFlagBits::eFragment).containerIndex != currObject->getFragShader().containerIndex)) {
                //vulkan object has shaders but they are not the same as the shaders needed for current render object
                this->vulkanObjects.push_back(std::make_unique<VulkanObject>(this->device, this->swapChainImages.size()));
                VulkanObject* newObject = this->vulkanObjects.at(this->vulkanObjects.size()).get();
                newObject->registerShader(vk::ShaderStageFlagBits::eVertex, currObject->getVertShader());
                newObject->registerShader(vk::ShaderStageFlagBits::eFragment, currObject->getFragShader());
                newObject->addObject(this->objectList->at(i), currObject, this->swapChainImages.size());

            }
            else {
                //vulkan object has the same shaders as the render object 
                object->addObject(this->objectList->at(i), currObject, this->swapChainImages.size());
            }
        }
    }

        //set up pool
    //one uniform buffer per frame
    this->globalPool = StarDescriptorPool::Builder(this->device)
        .setMaxSets((this->swapChainImages.size()))
        .addPoolSize(vk::DescriptorType::eUniformBuffer, this->swapChainImages.size())
        .build();
    //need object information for each frame 
    this->perObjectStaticPool = StarDescriptorPool::Builder(this->device)
        .setMaxSets((this->swapChainImages.size() * tmpVulkanObject->getNumRenderObjects()))
        .addPoolSize(vk::DescriptorType::eUniformBuffer, this->swapChainImages.size() * tmpVulkanObject->getNumRenderObjects())
        //.addPoolSize(vk::DescriptorType::eCombinedImageSampler, this->swapChainImages.size() * tmpVulkanObject->getNumRenderObjects())
        .build();

    this->globalSetLayout = StarDescriptorSetLayout::Builder(this->device)
        .addBinding(0, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment)
        .build();

    this->perObjectStaticLayout = StarDescriptorSetLayout::Builder(this->device)
        .addBinding(0, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex, 1)
        .build(); 

    this->globalDescriptorSets.resize(this->swapChainImages.size());



    createGraphicsPipeline();
    createDepthResources();
    createFramebuffers();
    createCommandPools();
    createTextureImage();
    createTextureImageView();
    createTextureSampler();
    createVertexBuffers();
    createIndexBuffer();
    createRenderingBuffers();

    //group up descriptors into sets 
    std::unique_ptr<std::vector<vk::DescriptorBufferInfo>> bufferInfos{}; 

    //tmp
    //this->perObjectDescriptorSets.resize(this->swapChainImages.size());
    //for (auto& tmp : this->perObjectDescriptorSets) {
    //    tmp.resize(tmpVulkanObject->getNumRenderObjects()); 
    //}

    for (size_t i = 0; i < this->swapChainImages.size(); i++) {
        //global
        bufferInfos = std::make_unique<std::vector<vk::DescriptorBufferInfo>>(); 

        bufferInfos->push_back(vk::DescriptorBufferInfo{
            this->globalUniformBuffers[i],
            0,
            sizeof(GlobalUniformBufferObject) });

        StarDescriptorWriter(this->device, *this->globalSetLayout, *this->globalPool)
            .writeBuffer(0, &bufferInfos.get()->at(0))
            .build(this->globalDescriptorSets.at(i));

        //auto bufferInfo1 = vk::DescriptorBufferInfo{
        //    this->uniformBuffers[i],
        //    sizeof(UniformBufferObject),
        //    sizeof(UniformBufferObject)
        //}; 

        //push this to the correct object 

        ////create offset into per object ubo 
        for (size_t j = 0; j < tmpVulkanObject->getNumRenderObjects(); j++) {
            //per object data -- updated every frame
            auto bufferInfo = vk::DescriptorBufferInfo{
                this->uniformBuffers[i],
                sizeof(UniformBufferObject) * j,
                sizeof(UniformBufferObject) };

            //bufferInfos = std::make_unique<std::vector<vk::DescriptorBufferInfo>>();
            StarDescriptorWriter(this->device, *this->perObjectStaticLayout, *this->perObjectStaticPool)
                .writeBuffer(0, &bufferInfo)
                .build(tmpVulkanObject->getRenderObjectAt(j)->getDefaultDescriptorSets()->at(i));
        }
    }

    //bufferInfos = std::make_unique<std::vector<vk::DescriptorBufferInfo>>();
    //bufferInfos->push_back(
    //    vk::DescriptorBufferInfo{
    //        this->uniformBuffers[0],
    //        0,
    //        sizeof(UniformBufferObject) });

    //StarDescriptorWriter(this->device, *this->perObjectStaticLayout, *this->perObjectStaticPool)
    //    .writeBuffer(0, bufferInfos.get())
    //    .build(testSet, true);

    //for (size_t i = 0; i < this->swapChainImages.size(); i++) {
    //    //for (size_t j = 0; j < tmpVulkanObject->getNumRenderObjects(); j++) {
    //    auto bufferInfo = vk::DescriptorBufferInfo{};
    //    bufferInfo.buffer = uniformBuffers[i];
    //    bufferInfo.offset = 0;
    //    bufferInfo.range = sizeof(UniformBufferObject); 

    //    std::vector<vk::DescriptorSet> newSets;
    //    StarDescriptorWriter(this->device, *this->globalSetLayout, *this->globalPool)
    //        .writeBuffer(0, &bufferInfo)
    //        .build(newSets, true);

    //    this->globalDescriptorSets[i].push_back(newSets.at(0));

    //    auto bufferInfo2 = vk::DescriptorBufferInfo{}; 
    //    bufferInfo2.buffer = uniformBuffers[i];
    //    bufferInfo2.offset = sizeof(UniformBufferObject); 
    //    bufferInfo2.range = sizeof(UniformBufferObject);


    //    //create pool for texture and texture sampler
    //    vk::DescriptorImageInfo imageInfo{};
    //    imageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
    //    imageInfo.imageView = textureImageView;
    //    imageInfo.sampler = textureSampler;

    //    //WARNING: only using the first set created 
    //    std::vector<vk::DescriptorSet> newSetsN;
    //    StarDescriptorWriter(this->device, *this->globalSetLayout, *this->globalPool)
    //        .writeBuffer(0, &bufferInfo2)
    //        .build(newSetsN, true);

    //    this->globalDescriptorSets[i].push_back(newSetsN.at(0));
    //}

    //createDescriptorPool();
    //createDescriptorSets();
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
    this->device.waitForFences(inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

    /* Get Image From Swapchain */


    //as is extension we must use vk*KHR naming convention
    //UINT64_MAX -> 3rd argument: used to specify timeout in nanoseconds for image to become available
    /* Suboptimal SwapChain notes */
        //vulkan can return two different flags 
        // 1. VK_ERROR_OUT_OF_DATE_KHR: swap chain has become incompatible with the surface and cant be used for rendering. (Window resize)
        // 2. VK_SUBOPTIMAL_KHR: swap chain can still be used to present to the surface, but the surface properties no longer match
    auto result = this->device.acquireNextImageKHR(swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame]);

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
        this->device.waitForFences(1, &imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
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
    submitInfo.pCommandBuffers = &graphicsCommandBuffers[imageIndex];

    //what semaphores to signal when command buffers have finished
    vk::Semaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame] };

    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    //set fence to unsignaled state
    this->device.resetFences(1, &inFlightFences[currentFrame]);

    auto submitResult = this->graphicsQueue.submit(1, &submitInfo, inFlightFences[currentFrame]);
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
    auto presentResult = presentQueue.presentKHR(presentInfo);

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

    this->device.destroySampler(this->textureSampler);
    this->device.destroyImageView(this->textureImageView);
    this->device.destroyImage(this->textureImage);
    this->device.freeMemory(this->textureImageMemory);

    //this->device.destroyDescriptorSetLayout(this->descriptorSetLayout);

    VulkanObject* currVulkanObject = this->vulkanObjects.at(0).get();

    this->device.destroyBuffer(currVulkanObject->indexBuffer);
    this->device.freeMemory(currVulkanObject->indexBufferMemory);
    this->device.destroyBuffer(currVulkanObject->vertexBuffer);
    this->device.freeMemory(currVulkanObject->vertexBufferMemory);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        this->device.destroySemaphore(renderFinishedSemaphores[i]);
        this->device.destroySemaphore(imageAvailableSemaphores[i]);
        this->device.destroyFence(inFlightFences[i]);
    }


    currVulkanObject->cleanup(); 
    this->device.destroyDescriptorSetLayout(this->globalSetLayout->getDescriptorSetLayout());
    this->device.destroyDescriptorPool(this->globalPool->getDescriptorPool()); 
    this->device.destroyDescriptorSetLayout(this->perObjectStaticLayout->getDescriptorSetLayout()); 
    this->device.destroyDescriptorPool(this->perObjectStaticPool->getDescriptorPool()); 

    //this->device.destroyDescriptorSetLayout(this->globalSetLayout->getDescriptorSetLayout()); 
    //this->device.destroyDescriptorSetLayout(this->perObjectStaticLayout->getDescriptorSetLayout()); 
    //for (size_t i = 0; i < currVulkanObject->getNumRenderObjects(); i++) {
    //    currRenderObject = currVulkanObject->getRenderObjectAt(i); 
    //    std::vector<vk::DescriptorSetcurrRenderObject->getDefaultDescriptorSets()
    //}
    //this->device.destroyDescriptorPool(*this->globalPool.get());

    //this->device.destroyDescriptorSetLayout(*this->globalSetLayout.get()); 

    this->device.destroyCommandPool(this->transferCommandPool);
    this->device.destroyCommandPool(this->graphicsCommandPool);

    this->device.destroy();

    this->instance.destroySurfaceKHR(this->surface->get());
    this->instance.destroy();

    glfwDestroyWindow(this->glfwWindow);
    glfwTerminate();
}

void star::core::VulkanRenderer::cleanupSwapChain() {
    auto& tmpVulkanObject = this->vulkanObjects.at(0);
    this->device.destroyImageView(this->depthImageView);
    this->device.destroyImage(this->depthImage);
    this->device.freeMemory(this->depthImageMemory);

    for (auto framebuffer : this->swapChainFramebuffers) {
        this->device.destroyFramebuffer(framebuffer);
    }

    this->device.freeCommandBuffers(this->graphicsCommandPool, this->graphicsCommandBuffers);


    this->device.destroyPipeline(tmpVulkanObject->pipelines.at(0));
    this->device.destroyPipelineLayout(tmpVulkanObject->getPipelineLayout());
    this->device.destroyRenderPass(this->renderPass);

    for (auto imageView : this->swapChainImageViews) {
        this->device.destroyImageView(imageView);
    }

    this->device.destroySwapchainKHR(this->swapChain);

    for (size_t i = 0; i < swapChainImages.size(); i++) {
        this->device.destroyBuffer(uniformBuffers[i]);
        this->device.freeMemory(uniformBuffersMemory[i]);

        this->device.destroyBuffer(this->globalUniformBuffers[i]); 
        this->device.freeMemory(this->globalUniformBuffersMemory[i]);
    }

    //this->device.destroyDescriptorPool(this->descriptorPool);
}

bool star::core::VulkanRenderer::checkValidationLayerSupport() {
    uint32_t layerCount;
    vk::enumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<vk::LayerProperties> availableLayers(layerCount);
    vk::enumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char* layerName : validationLayers) {
        bool layerFound = false;

        for (const auto& layerProperties : availableLayers) {
            if (strcmp(layerName, layerProperties.layerName) == 0) {
                layerFound = true;
                break;
            }
        }

        if (!layerFound) {
            return false;
        }
    }
    return true;
}

void star::core::VulkanRenderer::createInstance() {
    uint32_t extensionCount = 0;

    if (enableValidationLayers && !checkValidationLayerSupport()) {
        throw std::runtime_error("validation layers requested, but not available");
    }

    //enumerate required extensions
    std::vector<vk::ExtensionProperties> requiredExtensions(***this->glfwRequiredExtensions);
    vk::enumerateInstanceExtensionProperties(nullptr, this->glfwRequiredExtensionsCount.get(), requiredExtensions.data());

    //get a count of the number of supported extensions on the system
    //first argument is a filter for type -- leaving null to get all 
    vk::enumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
    std::vector<vk::ExtensionProperties> extensions(extensionCount);
    //query the extension details
    vk::enumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

    int foundExtensions = 0;
    for (const auto& extension : requiredExtensions) {
        bool found = false;

        for (const auto& availableExtension : extensions) {
            if (found) {
                foundExtensions++;
                break;
            }

            found = ((*extension.extensionName == *availableExtension.extensionName) && (extension.specVersion == availableExtension.specVersion));
        }

    }

    if (foundExtensions != *this->glfwRequiredExtensionsCount.get()) {
        throw std::runtime_error("Not all required extensions found for glfw");
    }

    vk::ApplicationInfo appInfo{};
    appInfo.sType = vk::StructureType::eApplicationInfo;
    // appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Hello Triangle";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "Starlight";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    vk::InstanceCreateInfo createInfo{};
    createInfo.sType = vk::StructureType::eInstanceCreateInfo;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = *this->glfwRequiredExtensionsCount.get();
    createInfo.ppEnabledExtensionNames = *this->glfwRequiredExtensions.get();
    createInfo.enabledLayerCount = 0;
    if (enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    }
    else {
        createInfo.enabledLayerCount = 0;
    }

    /*
    All vulkan objects follow this pattern of creation :
    1.pointer to a struct with creation info
        2.pointer to custom allocator callbacks, (nullptr) here
        3.pointer to the variable that stores the handle to the new object
    */
    //TODO: PUT A TRY HERE
    this->instance = vk::createInstance(createInfo);
}

void star::core::VulkanRenderer::pickPhysicalDevice() {
    std::vector<vk::PhysicalDevice> devices = this->instance.enumeratePhysicalDevices();

    //check devices and see if they are suitable for use
    for (const auto& device : devices) {
        if (isDeviceSuitable(device)) {
            physicalDevice = device;
            break;
        }
    }

    if (devices.size() == 0) {
        throw std::runtime_error("failed to find suitable GPU!");
    }

    if (!physicalDevice) {
        throw std::runtime_error("failed to find suitable GPU!");
    }
}

bool star::core::VulkanRenderer::isDeviceSuitable(vk::PhysicalDevice device) {
    /*
    Method of querying specific information about a device and checking if that device features support for a geometryShader
    VkPhysicalDeviceProperties deviceProperties;
    VkPhysicalDeviceFeatures deviceFeatures;

    vkGetPhysicalDeviceProperties(device, &deviceProperties);
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

    return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && deviceFeatures.geometryShader;
*/
    bool swapChainAdequate = false;
    QueueFamilyIndices indicies = findQueueFamilies(device);
    bool extensionsSupported = checkDeviceExtensionSupport(device);
    if (extensionsSupported) {
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }

    vk::PhysicalDeviceFeatures supportedFeatures = device.getFeatures();

    return indicies.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
}

star::core::VulkanRenderer::QueueFamilyIndices star::core::VulkanRenderer::findQueueFamilies(vk::PhysicalDevice device) {
    QueueFamilyIndices indicies;

    // device.getQueueFamilyProperties(queueFamilyCount, queueFamilies.data()); 
    std::vector<vk::QueueFamilyProperties> queueFamilies = device.getQueueFamilyProperties();

    //need to find a graphicsQueue that supports VK_QUEUE_GRAPHICS_BIT 
    int i = 0;
    for (const auto& queueFamily : queueFamilies) {
        vk::Bool32 presentSupport = device.getSurfaceSupportKHR(i, this->surface->get());

        //pick the family that supports presenting to the display 
        if (presentSupport) {
            indicies.presentFamily = i;
        }
        //pick family that has graphics support
        if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) {
            indicies.graphicsFamily = i;
        }
        else if (queueFamily.queueFlags & vk::QueueFlagBits::eTransfer) {
            //for transfer family, pick family that does not support graphics but does support transfer queue
            indicies.transferFamily = i;
        }

        //--COULD DO :: pick a device that supports both of these in the same queue for increased performance--
        i++;
    }

    return indicies;
}

bool star::core::VulkanRenderer::checkDeviceExtensionSupport(vk::PhysicalDevice device) {
    uint32_t extensionCount;
    device.enumerateDeviceExtensionProperties(nullptr, &extensionCount, nullptr);
    std::vector<vk::ExtensionProperties> availableExtensions(extensionCount);
    device.enumerateDeviceExtensionProperties(nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

    //iterate through extensions looking for those that are required
    for (const auto& extension : availableExtensions) {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

star::core::VulkanRenderer::SwapChainSupportDetails star::core::VulkanRenderer::querySwapChainSupport(vk::PhysicalDevice device) {
    SwapChainSupportDetails details;
    uint32_t formatCount, presentModeCount;

    //get surface capabilities 
    details.capabilities = device.getSurfaceCapabilitiesKHR(this->surface->get());

    device.getSurfaceFormatsKHR(this->surface->get(), &formatCount, nullptr);

    device.getSurfacePresentModesKHR(this->surface->get(), &presentModeCount, nullptr);

    if (formatCount != 0) {
        //resize vector in order to hold all available formats
        details.formats.resize(formatCount);
        device.getSurfaceFormatsKHR(this->surface->get(), &formatCount, details.formats.data());
    }

    if (presentModeCount != 0) {
        //resize for same reasons as format 
        details.presentModes.resize(presentModeCount);
        device.getSurfacePresentModesKHR(this->surface->get(), &presentModeCount, details.presentModes.data());
    }

    return details;
}

void star::core::VulkanRenderer::createLogicalDevice() {
    float queuePrioriy = 1.0f;
    QueueFamilyIndices indicies = findQueueFamilies(this->physicalDevice);

    //need multiple structs since we now have a seperate family for presenting and graphics 
    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = { indicies.graphicsFamily.value(), indicies.presentFamily.value(), indicies.transferFamily.value() };

    for (uint32_t queueFamily : uniqueQueueFamilies) {
        //create a struct to contain the information required 
        //create a queue with graphics capabilities
        vk::DeviceQueueCreateInfo  queueCreateInfo{};
        vk::StructureType::eApplicationInfo;
        queueCreateInfo.sType = vk::StructureType::eDeviceQueueCreateInfo;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        //most drivers support only a few queue per queueFamily 
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePrioriy;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    //specifying device features that we want to use -- can pull any of the device features that was queried before...for now use nothing
    const vk::PhysicalDeviceFeatures deviceFeatures{};
    // deviceFeatures.samplerAnisotropy = VK_TRUE;  

    //Create actual logical device
    const vk::DeviceCreateInfo createInfo{
        vk::DeviceCreateFlags(),                                                        //device creation flags
        static_cast<uint32_t>(queueCreateInfos.size()),                                 //queue create info count 
        queueCreateInfos.data(),                                                        //device queue create info
        enableValidationLayers ? static_cast<uint32_t>(deviceExtensions.size()) : 0,    //enabled layer count 
        enableValidationLayers ? deviceExtensions.data() : VK_NULL_HANDLE,              //enables layer names
        static_cast<uint32_t>(deviceExtensions.size()),                                 //enabled extension coun 
        deviceExtensions.data(),                                                        //enabled extension names 
        &deviceFeatures                                                                 //enabled features
    };

    //call to create the logical device 
    device = physicalDevice.createDevice(createInfo);

    this->graphicsQueue = device.getQueue(indicies.graphicsFamily.value(), 0);
    this->presentQueue = device.getQueue(indicies.presentFamily.value(), 0);
    this->transferQueue = device.getQueue(indicies.transferFamily.value(), 0);
}

void star::core::VulkanRenderer::createSwapChain() {
    //TODO: current implementation requires halting to all rendering when recreating swapchain. Can place old swap chain in oldSwapChain field 
    //  in order to prevent this and allow rendering to continue
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);

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
    createInfo.surface = this->surface->get();

    //specify image information for the surface 
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1; //1 unless using 3D display 
    //createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; //how are these images going to be used? Color attachment since we are rendering to them (can change for postprocessing effects)
    createInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment; //how are these images going to be used? Color attachment since we are rendering to them (can change for postprocessing effects)

    QueueFamilyIndices indicies = findQueueFamilies(physicalDevice);
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

    this->swapChain = this->device.createSwapchainKHR(createInfo);

    //if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
    //    throw std::runtime_error("failed to create swap chain");
    //}

    //get images in the newly created swapchain 
    this->swapChainImages = this->device.getSwapchainImagesKHR(this->swapChain);

    //save swapChain information for later use
    swapChainImageFormat = surfaceFormat.format;
    swapChainExtent = extent;
}

void star::core::VulkanRenderer::recreateSwapChain() {
    int width = 0, height = 0;
    //check for window minimization and wait for window size to no longer be 0
    glfwGetFramebufferSize(this->glfwWindow, &width, &height);
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(this->glfwWindow, &width, &height);
        glfwWaitEvents();
    }
    //wait for device to finish any current actions
    vkDeviceWaitIdle(device);

    cleanupSwapChain();

    //create swap chain itself 
    createSwapChain();

    //image views depend directly on swap chain images so these need to be recreated
    createImageViews();

    //render pass depends on the format of swap chain images
    createRenderPass();

    //viewport and scissor rectangle size are declared during pipeline creation, so the pipeline must be recreated
    //can use dynamic states for viewport and scissor to avoid this 
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
        glfwGetFramebufferSize(this->glfwWindow, &width, &height);

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
        //VkImageViewCreateInfo createInfo{};
        //createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        //createInfo.image = swapChainImages[i];

        ////specify how the image will be interpreted
        //createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        //createInfo.format = swapChainImageFormat;

        ////the next fields allows to swizzle RGB values -- leaving as defaults for now 
        //createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        //createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        //createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        //createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

        ////subresourceRange describes image purpose -- this use is color targets without any mipmapping levels or multiple layers
        //createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        //createInfo.subresourceRange.baseMipLevel = 0;
        //createInfo.subresourceRange.levelCount = 1;
        //createInfo.subresourceRange.baseArrayLayer = 0;
        //createInfo.subresourceRange.layerCount = 1;

        //swapChainImageViews[i] = createImageView(swapChainImages[i], swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
        swapChainImageViews[i] = createImageView(swapChainImages[i], swapChainImageFormat, vk::ImageAspectFlagBits::eColor);
        //if (vkCreateImageView(device, &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS) {
        //    throw std::runtime_error("failed to create image views");
        //}

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

    vk::ImageView imageView = device.createImageView(viewInfo);

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

    this->renderPass = device.createRenderPass(renderPassInfo);
    if (!renderPass) {
        throw std::runtime_error("failed to create render pass");
    }
}

vk::Format star::core::VulkanRenderer::findDepthFormat() {
    //utilizing the VK_FORMAT_FEATURE_ flag to check for candidates that have a depth component.
    return findSupportedFormat(
        { vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint },
        vk::ImageTiling::eOptimal,
        vk::FormatFeatureFlagBits::eDepthStencilAttachment);
}

vk::Format star::core::VulkanRenderer::findSupportedFormat(const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlagBits features) {
    for (vk::Format format : candidates) {
        //VkFormatProperties: 
            //linearTilingFeatures
            //optimalTilingFeatures
            //bufferFeatures
        vk::FormatProperties props = physicalDevice.getFormatProperties(format);

        //check if the properties matches the requirenments for tiling
        if (tiling == vk::ImageTiling::eLinear && (props.linearTilingFeatures & features) == features) {
            return format;
        }
        else if ((tiling == vk::ImageTiling::eOptimal) && (props.optimalTilingFeatures & features) == features) {
            return format;
        }
    }

    throw std::runtime_error("failed to find supported format!");
}

//void star::core::VulkanRenderer::createDescriptorSetLayout() {
//    vk::DescriptorSetLayoutBinding setLayoutBindings;
//
//    /* Binding 0 : Uniform buffers (MVP matricies) */
//    setLayoutBindings.binding = 0;
//    setLayoutBindings.descriptorType = vk::DescriptorType::eUniformBuffer;    //for this, we are using a uniform buffer object (UBO)
//    setLayoutBindings.descriptorCount = 1;                                   //can pass an array of uniform buffer objects, for this we are only using one 
//
//    //which shader stages are going to use the descriptor 
//    setLayoutBindings.stageFlags = vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment;
//
//    //only needed for image sampling related descriptors -- not used now
//    setLayoutBindings.pImmutableSamplers = nullptr;
//
//    //binding for combined image sampler descriptor
//    vk::DescriptorSetLayoutBinding samplerLayoutBinding{};
//    samplerLayoutBinding.binding = 1;
//    samplerLayoutBinding.descriptorCount = 1;
//    samplerLayoutBinding.descriptorType = vk::DescriptorType::eCombinedImageSampler;
//    samplerLayoutBinding.pImmutableSamplers = nullptr;
//    samplerLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eFragment;                     //use in the fragment shader
//
//    std::array<vk::DescriptorSetLayoutBinding, 2> bindings = { setLayoutBindings, samplerLayoutBinding };
//    vk::DescriptorSetLayoutCreateInfo layoutInfo{};
//    layoutInfo.sType = vk::StructureType::eDescriptorSetLayoutCreateInfo;
//    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
//    layoutInfo.pBindings = bindings.data();
//
//    this->descriptorSetLayout = device.createDescriptorSetLayout(layoutInfo);
//    if (!this->descriptorSetLayout) {
//        throw std::runtime_error("failed to create descriptor set layout!");
//    }
//}

void star::core::VulkanRenderer::createGraphicsPipeline() {
    auto bindingDescriptions = VulkanVertex::getBindingDescription();
    auto attributeDescriptions = VulkanVertex::getAttributeDescriptions();


    //common::Object* currObject = this->objectManager->Get(objectHandle); 

    //auto fragShaderCode = readFile("media/shaders/fragShader.frag.spv");
    //auto vertShaderCode = readFile("media/shaders/vertShader.vert.spv");
    if (vulkanObjects.size() > 1) {
        throw std::runtime_error("The creation of more than one pipeline is not yet supported");
    }

    for (size_t i = 0; i < this->vulkanObjects.size(); i++) {
        VulkanObject* vulkanObject = this->vulkanObjects.at(i).get(); 
        auto vertShaderCode = this->shaderManager->Get(vulkanObject->getBaseShader(vk::ShaderStageFlagBits::eVertex))->GetSpirV();
        auto fragShaderCode = this->shaderManager->Get(vulkanObject->getBaseShader(vk::ShaderStageFlagBits::eFragment))->GetSpirV();
        vulkanObject->registerShaderModule(vk::ShaderStageFlagBits::eVertex, createShaderModule(vertShaderCode));
        vulkanObject->registerShaderModule(vk::ShaderStageFlagBits::eFragment, createShaderModule(fragShaderCode));

        //assign each shader module to a specific stage of the graphics pipeline
        //vert shader first
        vk::PipelineShaderStageCreateInfo vertShaderStageInfo{};
        vertShaderStageInfo.sType = vk::StructureType::ePipelineShaderStageCreateInfo;
        vertShaderStageInfo.stage = vk::ShaderStageFlagBits::eVertex;
        vertShaderStageInfo.module = vulkanObject->getShaderModule(vk::ShaderStageFlagBits::eVertex);
        vertShaderStageInfo.pName = "main"; //the function to invoke in the shader module
        //optional member -> pSpecializationInfo: 
        //  allows specification for values to shader constants. Use a single single shader module whos function could be customized through this optional value. 
        //  if not useing, set to nullptr which is done automatically in this case with the constructor of the struct. 
        //  Additionally: it is a good choice to use this value instead of variables so that graphics driver can remove if statements if needed for optimization

        //create pipeline info for fragment shader 
        vk::PipelineShaderStageCreateInfo fragShaderStageInfo{};
        fragShaderStageInfo.sType = vk::StructureType::ePipelineShaderStageCreateInfo;
        fragShaderStageInfo.stage = vk::ShaderStageFlagBits::eFragment;
        fragShaderStageInfo.module = vulkanObject->getShaderModule(vk::ShaderStageFlagBits::eFragment);
        fragShaderStageInfo.pName = "main";

        //store these creation infos for later use 
        vk::PipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

        vk::PipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = vk::StructureType::ePipelineVertexInputStateCreateInfo;

        //pVertexBindingDescriptions and pVertexAttributeDescription -> point to arrays of structs to load vertex data
        //for now: leaving blank as the verticies are hard coded in the shaders
        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.pVertexBindingDescriptions = &bindingDescriptions;
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
        vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

        /*
        VkPipelineInputAssemblyStateCreateInfo -> Describes 2 things:
            1.what kind of geometry will be drawn
                described in topology member:
                    -VK_PRIMITIVE_TOPOLOGY_POINT_LIST: points from verticies
                    -VK_PRIMITIVE_TOPOLOGY_LINE_LIST: line from every 2 verticies without reuse
                    -VK_PRIMITIVE_TOPOLOGY_LINE_STRIP: line strip
                    -VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST: triangle from every 3 verticies without reuse
                    -VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP: 2nd and 3rd vertex of every triangle are used as first two verticies of the next triangle
            2.if primitive restart should be enabled
        */
        //Verticies are normally loaded from vertex buffer in sequential order 
        //element buffer can be used to specify this information manually
        //this allows the reuse of verticies!
        vk::PipelineInputAssemblyStateCreateInfo inputAssembly{};
        //inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.sType = vk::StructureType::ePipelineInputAssemblyStateCreateInfo;
        //inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssembly.topology = vk::PrimitiveTopology::eTriangleList;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        /* Viewport */
        //Viewport describes the region of the framebuffer where the output will be rendered to
        vk::Viewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;

        viewport.width = (float)swapChainExtent.width;
        viewport.height = (float)swapChainExtent.height;
        //Specify values range of depth values to use for the framebuffer. If not doing anything special, leave at default
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        /* Scissor */
        //this defines in which regions pixels will actually be stored. 
        //any pixels outside will be discarded 

        //we just want to draw the whole framebuffer for now
        vk::Rect2D scissor{};
        //scissor.offset = { 0, 0 };
        scissor.extent = swapChainExtent;

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

        // vk::PushConstantRange pushConstant; 
        // pushConstant.offset = 0; 
        // pushConstant.size = sizeof(ObjectPushConstants); 
        // pushConstant.stageFlags = vk::ShaderStageFlagBits::eVertex; 

        std::vector<vk::DescriptorSetLayout> descriptorSetLayouts{this->globalSetLayout->getDescriptorSetLayout(), this->perObjectStaticLayout->getDescriptorSetLayout()};
        /* Pipeline Layout */
        //uniform values in shaders need to be defined here 
        vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
        //pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.sType = vk::StructureType::ePipelineLayoutCreateInfo;
        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
        pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
        // pipelineLayoutInfo.pushConstantRangeCount = 1;
        // pipelineLayoutInfo.pPushConstantRanges = &pushConstant;
        pipelineLayoutInfo.pushConstantRangeCount = 0; 
        pipelineLayoutInfo.pPushConstantRanges = nullptr; 

        vk::PipelineLayout newLayout = this->device.createPipelineLayout(pipelineLayoutInfo);
        if (!newLayout) {
            throw std::runtime_error("failed to create pipeline layout");
        }
        vulkanObject->setPipelineLayout(newLayout);

        /* Pipeline */
        vk::GraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = vk::StructureType::eGraphicsPipelineCreateInfo;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shaderStages;
        //ref all previously created structs
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pDepthStencilState = &depthStencil;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDynamicState = nullptr; // Optional
        pipelineInfo.layout = vulkanObject->getPipelineLayout();
        //render pass info - ensure renderpass is compatible with pipeline --check khronos docs
        pipelineInfo.renderPass = renderPass;
        pipelineInfo.subpass = 0; //index where the graphics pipeline will be used 
        //allow switching to new pipeline (inheritance) 
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional -- handle to existing pipeline that is being switched to
        pipelineInfo.basePipelineIndex = -1; // Optional

        //finally creating the pipeline -- this call has the capability of creating multiple pipelines in one call
        //2nd arg is set to null -> normally for graphics pipeline cache (can be used to store and reuse data relevant to pipeline creation across multiple calls to vkCreateGraphicsPipeline)

        //this->graphicsPipeline = this->device.get().createGraphicsPipeline(VK_NULL_HANDLE, pipelineInfo); 
        auto result = this->device.createGraphicsPipelines(VK_NULL_HANDLE, pipelineInfo);
        if (result.result != vk::Result::eSuccess) {
            throw std::runtime_error("failed to create graphics pipeline");
        }
        if (result.value.size() > 1) {
            throw std::runtime_error("unknown error has occurred, more than one pipeline was created ");
        }
        vulkanObject->addPipelines(result.value);

        //destroy the shader modules that were created 
        this->device.destroyShaderModule(vulkanObject->getShaderModule(vk::ShaderStageFlagBits::eVertex));
        this->device.destroyShaderModule(vulkanObject->getShaderModule(vk::ShaderStageFlagBits::eFragment));
    }
}

vk::ShaderModule star::core::VulkanRenderer::createShaderModule(const std::vector<uint32_t>& code) {
    vk::ShaderModuleCreateInfo createInfo{};
    createInfo.sType = vk::StructureType::eShaderModuleCreateInfo;
    createInfo.codeSize = 4 * code.size();
    createInfo.pCode = code.data();

    VkShaderModule shaderModule = this->device.createShaderModule(createInfo);
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

    image = this->device.createImage(imageInfo);
    if (!image) {
        throw std::runtime_error("failed to create image");
    }

    /* Allocate the memory for the imag*/
    vk::MemoryRequirements memRequirements = this->device.getImageMemoryRequirements(image);

    vk::MemoryAllocateInfo allocInfo{};
    allocInfo.sType = vk::StructureType::eMemoryAllocateInfo;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

    imageMemory = this->device.allocateMemory(allocInfo);
    if (!imageMemory) {
        throw std::runtime_error("failed to allocate image memory!");
    }

    this->device.bindImageMemory(image, imageMemory, 0);
}

uint32_t star::core::VulkanRenderer::findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties) {
    //query available memory -- right now only concerned with memory type, not the heap that it comes from
    /*VkPhysicalDeviceMemoryProperties contains:
    1. memoryTypes
    2. memoryHeaps - distinct memory resources (dedicated VRAM or swap space)
    */
    vk::PhysicalDeviceMemoryProperties memProperties = this->physicalDevice.getMemoryProperties();

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        //use binary AND to test each bit (Left Shift)
        //check memory types array for more detailed information on memory capabilities
            //we need to be able to write to memory, so speficially looking to be able to MAP to the memory to write to it from the CPU -- VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
        //also need VK_MEMORY_PROPERTY_HOST_COHERENT_BIT

        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type");
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

        swapChainFramebuffers[i] = this->device.createFramebuffer(framebufferInfo);
        if (!swapChainFramebuffers[i]) {
            throw std::runtime_error("failed to create framebuffer");
        }
    }
}

void star::core::VulkanRenderer::createCommandPools() {
    QueueFamilyIndices queueFamilyIndicies = findQueueFamilies(physicalDevice);

    /* Command Buffers */
    //command buffers must be submitted on one of the device queues (graphics or presentation queues in this case)
    //must only be submitted on a single type of queue
    //creating commands for drawing, as such these are submitted on the graphics family 
    /* Two possible flags for command pools:
        1.VK_COMMAND_POOL_CREATE_TRANSIENT_BIT: warn vulkan that the command pool is changed often
        2.VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT: allow command buffers to be rerecorded individually, without this all command buffers are reset at the same time
    */
    //commandPoolInfo.flags = 0; //optional -- will not be changing or resetting any command buffers 

    //graphics command buffer
    createPool(queueFamilyIndicies.graphicsFamily.value(), vk::CommandPoolCreateFlagBits{}, graphicsCommandPool);

    //command buffer for transfer queue 
    createPool(queueFamilyIndicies.transferFamily.value(), vk::CommandPoolCreateFlagBits{}, transferCommandPool);

    //temporary command pool --unused at this time
    //createPool(queueFamilyIndicies.graphicsFamily.value(), VK_COMMAND_POOL_CREATE_TRANSIENT_BIT, tempCommandPool); 

}

void star::core::VulkanRenderer::createPool(uint32_t queueFamilyIndex, vk::CommandPoolCreateFlagBits flags, vk::CommandPool& pool) {
    QueueFamilyIndices queueFamilyIndicies = findQueueFamilies(physicalDevice);

    vk::CommandPoolCreateInfo commandPoolInfo{};
    commandPoolInfo.sType = vk::StructureType::eCommandPoolCreateInfo;
    commandPoolInfo.queueFamilyIndex = queueFamilyIndex;
    commandPoolInfo.flags = flags;

    pool = this->device.createCommandPool(commandPoolInfo);

    if (!pool) {
        throw std::runtime_error("unable to create pool");
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
    createBuffer(imageSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, stagingBuffer, stagingBufferMemory);

    //copy over to staging buffer
    void* data;
    data = this->device.mapMemory(stagingBufferMemory, 0, imageSize);
    memcpy(data, texture->data(), static_cast<size_t>(imageSize));
    this->device.unmapMemory(stagingBufferMemory);

    //stbi_image_free(pixels);

    createImage(texture->width(), texture->height(), vk::Format::eR8G8B8A8Srgb, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled, vk::MemoryPropertyFlagBits::eDeviceLocal, textureImage, textureImageMemory);

    //copy staging buffer to texture image 
    transitionImageLayout(textureImage, vk::Format::eR8G8B8A8Srgb, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);

    copyBufferToImage(stagingBuffer, textureImage, static_cast<uint32_t>(texture->width()), static_cast<uint32_t>(texture->height()));

    //prepare final image for texture mapping in shaders 
    transitionImageLayout(textureImage, vk::Format::eR8G8B8A8Srgb, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);

    this->device.destroyBuffer(stagingBuffer);
    this->device.freeMemory(stagingBufferMemory);
}


void star::core::VulkanRenderer::copyBufferToImage(vk::Buffer buffer, vk::Image image, uint32_t width, uint32_t height) {
    bool useTransferPool = true;

    vk::CommandBuffer commandBuffer = beginSingleTimeCommands(useTransferPool);

    //specify which region of the buffer will be copied to the image 
    vk::BufferImageCopy region{};
    region.bufferOffset = 0;                                            //specifies byte offset in the buffer at which the pixel values start
    //the following specify the layout of pixel information in memory
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    //the following indicate what part of the image we want to copy to 
    //region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    //region.imageOffset = { 0, 0, 0 };
    region.imageOffset = vk::Offset3D{};
    region.imageOffset = vk::Offset3D{};
    region.imageExtent = vk::Extent3D{
        width,
        height,
        1
    };

    //enque copy operation 
    commandBuffer.copyBufferToImage(
        buffer,
        image,
        vk::ImageLayout::eTransferDstOptimal,       //assuming image is already in optimal format for copy operations
        region
    );

    endSingleTimeCommands(commandBuffer, useTransferPool);
}

void star::core::VulkanRenderer::createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, vk::Buffer& buffer, vk::DeviceMemory& bufferMemory) {
    vk::BufferCreateInfo bufferInfo{};

    //bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.sType = vk::StructureType::eBufferCreateInfo;
    bufferInfo.size = size;
    bufferInfo.usage = usage;                           //purpose of data in buffer
    bufferInfo.sharingMode = vk::SharingMode::eExclusive; //buffers can be owned by specific queue family or shared between them at the same time. This only used for graphics queue

    buffer = this->device.createBuffer(bufferInfo);

    if (!buffer) {
        throw std::runtime_error("failed to create buffer");
    }

    //need to allocate memory for the buffer object
    /* VkMemoryRequirements:
        1. size - number of required bytes in memory
        2. alignments - offset in bytes where the buffer begins in the allocated region of memory (depends on bufferInfo.useage and bufferInfo.flags)
        3. memoryTypeBits - bit fied of the memory types that are suitable for the buffer
    */
    vk::MemoryRequirements memRequirenments = this->device.getBufferMemoryRequirements(buffer);

    assert(size <= memRequirenments.size);

    vk::MemoryAllocateInfo allocInfo{};
    //allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.sType = vk::StructureType::eMemoryAllocateInfo;
    allocInfo.allocationSize = memRequirenments.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirenments.memoryTypeBits, properties);

    bufferMemory = this->device.allocateMemory(allocInfo);

    //should not call vkAllocateMemory for every object. Bundle objects into one call and use offsets 
    if (!bufferMemory) {
        throw std::runtime_error("failed to allocate buffer memory");
    }

    //4th argument: offset within the region of memory. Since memory is allocated specifically for this vertex buffer, the offset is 0
    //if not 0, required to be divisible by memRequirenments.alignment
    this->device.bindBufferMemory(buffer, bufferMemory, 0);
}

void star::core::VulkanRenderer::transitionImageLayout(vk::Image image, vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout) {
    vk::CommandBuffer commandBuffer = beginSingleTimeCommands();

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

    endSingleTimeCommands(commandBuffer);
}

void star::core::VulkanRenderer::createTextureImageView() {
    //this->textureImageView = createImageView(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
    this->textureImageView = createImageView(textureImage, vk::Format::eR8G8B8A8Srgb, vk::ImageAspectFlagBits::eColor);
}

void star::core::VulkanRenderer::createTextureSampler() {
    //get device properties for amount of anisotropy permitted
    vk::PhysicalDeviceProperties deviceProperties = this->physicalDevice.getProperties();

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

    this->textureSampler = this->device.createSampler(samplerInfo);
    if (!this->textureSampler) {
        throw std::runtime_error("failed to create texture sampler!");
    }
}

void star::core::VulkanRenderer::createVertexBuffers() {
    vk::DeviceSize bufferSize;
    vk::Buffer stagingBuffer;
    vk::DeviceMemory stagingBufferMemory;

    //TODO: ensure that more objects can be drawn 
    common::GameObject* currObject = nullptr;

    std::vector<common::Vertex>* currRenderObjectVerticies = nullptr;

    std::unique_ptr<std::vector<star::common::Vertex>> vertexList;
    size_t vertexCounter = 0;

    for (size_t i = 0; i < this->vulkanObjects.size(); i++){
        VulkanObject* vulkanObject = this->vulkanObjects.at(i).get(); 

        vertexCounter = 0;
        vertexList = std::make_unique<std::vector<star::common::Vertex>>(std::vector<star::common::Vertex>());

        //only resize vector container once 
        vertexList->resize(vulkanObject->totalNumVerticies);

        for (size_t i = 0; i < vulkanObject->getNumRenderObjects(); i++) {
            RenderObject* currRenderObject= vulkanObject->getRenderObjectAt(i);
            currObject = this->objectManager->Get(currRenderObject->getHandle());

            //go through all objects in object list and generate the vertex indicies -- only works with 1 object at the moment 
            currRenderObjectVerticies = currObject->getVerticies();

            //copy verticies from the render object into the total vertex list for the vulkan object
            for (size_t j = 0; j < currRenderObjectVerticies->size(); j++) {
                vertexList->at(vertexCounter) = currRenderObjectVerticies->at(j);
                vertexCounter++;
            }
        }

        bufferSize = sizeof(vertexList->at(0)) * vertexList->size();

        createBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, stagingBuffer, stagingBufferMemory);

        /* Filling the vertex buffer */
        void* data;

        //access a region of the specified memory resource defined by an offset and size 
        //can also specify VK_WHOLE_SIZE to map all memory 
        //currrently no memory flags available in API (time of writing) so must be set to 0
        data = this->device.mapMemory(stagingBufferMemory, 0, bufferSize);
        memcpy(data, vertexList->data(), (size_t)bufferSize); //simply copy data into mapped memory
        this->device.unmapMemory(stagingBufferMemory);

        /* Staging Buffer */
        //New flags 
        //VK_BUFFER_USAGE_TRANSFER_SRC_BIT: buffer can be used as source in a memory transfer 
        //VK_BUFFER_USAGE_TRANSFER_DST_BIT: buffer can be used as destination in a memory transfer 
        //createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory);
        createBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal, vulkanObject->vertexBuffer, vulkanObject->vertexBufferMemory);

        copyBuffer(stagingBuffer, vulkanObject->vertexBuffer, bufferSize); //actually call to copy memory


        //cleanup 
        this->device.destroyBuffer(stagingBuffer);
        vkFreeMemory(device, stagingBufferMemory, nullptr);
    }

    /* Memory Copy Note */
    //note: driver might not immediately copy the data into the buffer memory, ex: caching
    //also possible that writes not visible to mapped memory yet
    //two ways to handle this: 
        //1. use memory heap that is host coherent (VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) -- memory is matched to the GPU
        //2. call vkFlushMappedMemoryRanges after writing tot he mapped memory and then call vkInvalidateMappedMemoryRanges before reading from mapped memory
    //this project used (1) memory always matches but might lead to slightly worse performance 
    //Flushing memory ranges or using coherent calls does not mean they are passed to GPU. Vulkan does this in the background and memory is guaranteed to be on
        //on GPU before next call to vkQueueSubmit
}

void star::core::VulkanRenderer::createIndexBuffer() {
    vk::DeviceSize bufferSize;
    vk::Buffer stagingBuffer;
    vk::DeviceMemory stagingBufferMemory;

    //TODO: will only support one object at the moment
    std::unique_ptr<std::vector<uint32_t>> indiciesList;
    std::vector<uint32_t>* currRenderObjectIndicies = nullptr;
    common::GameObject* currObject = nullptr;
    size_t indexCounter = 0; //used to keep track of index offsets 
    bool firstObject = true;

    //go through each vulkan object and build its index list
    for (size_t i = 0; i < this->vulkanObjects.size(); i++){
        VulkanObject* vulkanObject = this->vulkanObjects.at(i).get(); 
        indiciesList = std::make_unique<std::vector<uint32_t>>(std::vector<uint32_t>());

        //calc number of indicies
        indiciesList->resize(vulkanObject->totalNumIndicies);

        for (size_t i = 0; i < vulkanObject->getNumRenderObjects(); i++) {
            RenderObject* currRenderObject = vulkanObject->getRenderObjectAt(i);
            currRenderObjectIndicies = this->objectManager->Get(currRenderObject->getHandle())->getIndicies();

            for (size_t j = 0; j < currRenderObjectIndicies->size(); j++) {
                if (i > 0) {
                    //not the first object 
                    //displace the index counter by the number of indicies previously used
                    indiciesList->at(indexCounter) = indexCounter + currRenderObjectIndicies->at(j);
                }
                else {
                    indiciesList->at(indexCounter) = indexCounter;
                }

                indexCounter++;
            }
        }

        bufferSize = sizeof(indiciesList->at(0)) * indiciesList->size();

        //createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);
        createBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, stagingBuffer, stagingBufferMemory);

        void* data = this->device.mapMemory(stagingBufferMemory, 0, bufferSize);
        memcpy(data, indiciesList->data(), (size_t)bufferSize);
        this->device.unmapMemory(stagingBufferMemory);

        //note the use of VK_BUFFER_USAGE_INDEX_BUFFER_BIT due to the use of the indicies
        //createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory);
        createBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal, vulkanObject->indexBuffer, vulkanObject->indexBufferMemory);

        copyBuffer(stagingBuffer, vulkanObject->indexBuffer, bufferSize);

        this->device.destroyBuffer(stagingBuffer);
        this->device.freeMemory(stagingBufferMemory);
    };
}

void star::core::VulkanRenderer::createRenderingBuffers() {
    /*Create Uniform Buffer */
    //just set up the buffers, we are not going to be updating the buffers here since they need updated every frame
    VulkanObject* tmpVulkanObject = this->vulkanObjects.at(0).get();

    vk::DeviceSize uboBufferSize = sizeof(UniformBufferObject) * tmpVulkanObject->getNumRenderObjects();

    uniformBuffers.resize(swapChainImages.size());
    uniformBuffersMemory.resize(swapChainImages.size());

    for (size_t i = 0; i < swapChainImages.size(); i++) {
        createBuffer(uboBufferSize, vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, uniformBuffers[i], uniformBuffersMemory[i]);
    }

    vk::DeviceSize globalBufferSize = sizeof(GlobalUniformBufferObject); 

    this->globalUniformBuffers.resize(swapChainImages.size());
    this->globalUniformBuffersMemory.resize(swapChainImages.size()); 

    for (size_t i = 0; i < swapChainImages.size(); i++) {
        createBuffer(globalBufferSize, vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, globalUniformBuffers[i], globalUniformBuffersMemory[i]); 
    }

}

void star::core::VulkanRenderer::createCommandBuffers() {

    for (auto& vulkanObject : this->vulkanObjects) {
        /* Graphics Command Buffer */
        graphicsCommandBuffers.resize(swapChainFramebuffers.size());

        vk::CommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = vk::StructureType::eCommandBufferAllocateInfo;
        allocInfo.commandPool = graphicsCommandPool;
        // .level - specifies if the allocated command buffers are primay or secondary
        // ..._PRIMARY : can be submitted to a queue for execution, but cannot be called from other command buffers
        // ..._SECONDARY : cannot be submitted directly, but can be called from primary command buffers (good for reuse of common operations)
        allocInfo.level = vk::CommandBufferLevel::ePrimary;
        allocInfo.commandBufferCount = (uint32_t)graphicsCommandBuffers.size();

        this->graphicsCommandBuffers = this->device.allocateCommandBuffers(allocInfo);
        if (this->graphicsCommandBuffers.size() == 0) {
            throw std::runtime_error("failed to allocate command buffers");
        }

        if (this->vulkanObjects.size() > 1) {
            throw std::runtime_error("More than one shader group is not yet supported"); 
        }

        VulkanObject* tmpVulkanObject = this->vulkanObjects.at(0).get();

        /* Begin command buffer recording */
        for (size_t i = 0; i < graphicsCommandBuffers.size(); i++) {
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

            this->graphicsCommandBuffers[i].begin(beginInfo);
            if (!this->graphicsCommandBuffers[i]) {
                throw std::runtime_error("failed to begin recording command buffer");
            }

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
            this->graphicsCommandBuffers[i].beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);

            /* Drawing Commands */
            //Args: 
                //2. compute or graphics pipeline
                //3. pipeline object
            //vkCmdBindPipeline(graphicsCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
            this->graphicsCommandBuffers[i].bindPipeline(vk::PipelineBindPoint::eGraphics, vulkanObject->pipelines.at(0));

            vk::Buffer vertexBuffers[] = { vulkanObject->vertexBuffer };
            //TODO: need to allow for an offset for each buffer
            vk::DeviceSize offsets = {};

            //bind vertex buffers -> how to pass information to the vertex shader once it is uploaded to the GPU
            this->graphicsCommandBuffers[i].bindVertexBuffers(0, vulkanObject->vertexBuffer, offsets);

            this->graphicsCommandBuffers[i].bindIndexBuffer(vulkanObject->indexBuffer, 0, vk::IndexType::eUint32);

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
            //this->graphicsCommandBuffers[i].bindDescriptorSets(vk::PipelineBindPoint::eGraphics, vulkanObject.getPipelineLayout(), 0, 1, , 0, nullptr);

            //bind global descriptor
            this->graphicsCommandBuffers[i].bindDescriptorSets(vk::PipelineBindPoint::eGraphics, tmpVulkanObject->getPipelineLayout(), 0, 1, &this->globalDescriptorSets.at(i), 0, nullptr);

            uint32_t vertexCount = 0;
            //for (size_t j = 0; j < tmpVulkanObject->getNumRenderObjects(); j++) {
            for (size_t j = 0; j < tmpVulkanObject->getNumRenderObjects(); j++) {
                // pushConstant = std::make_unique<ObjectPushConstants>();
                // auto tmp = ObjectPushConstants(); 
                RenderObject* renderObj = tmpVulkanObject->getRenderObjectAt(j);
                common::GameObject* tmpObject = this->objectManager->Get(renderObj->getHandle());

                //this->graphicsCommandBuffers[i].bindDescriptorSets(vk::PipelineBindPoint::eGraphics, tmpVulkanObject->getPipelineLayout(), 0, 1, &testSet, 0, nullptr);
                auto test = renderObj->getDefaultDescriptorSets(); 
                this->graphicsCommandBuffers[i].bindDescriptorSets(vk::PipelineBindPoint::eGraphics, tmpVulkanObject->getPipelineLayout(), 1, 1, &renderObj->getDefaultDescriptorSets()->at(i), 0, nullptr);
                //this->graphicsCommandBuffers[i].bindDescriptorSets(vk::PipelineBindPoint::eGraphics, tmpVulkanObject->getPipelineLayout(), 1, 1, &this->testSets.at((i * 2) + j), 0, nullptr);
                //this->graphicsCommandBuffers[i].bindDescriptorSets(vk::PipelineBindPoint::eGraphics, tmpVulkanObject->getPipelineLayout(), 1, 1, &this->perObjectDescriptorSets.at(i).at(j), 0, nullptr);
                //now create call to draw
                //Args:    
                    //2. vertexCount: how many verticies to draw
                    //3. instanceCount: used for instanced render, use 1 otherwise
                    //4. firstVertex: offset in VBO, defines lowest value of gl_VertexIndex
                    //5. firstInstance: offset for instanced rendering, defines lowest value of gl_InstanceIndex -> not using so leave at 1 
                //TODO: currently drawing ALL verticies...might need to make more flexible with more objects
                // pushConstant->modelIndex = j; 
                // this->graphicsCommandBuffers[i].pushConstants(tmpVulkanObject->getPipelineLayout(), vk::ShaderStageFlagBits::eVertex, 0, sizeof(ObjectPushConstants), pushConstant.get());
                auto numToDraw = renderObj->getNumVerticies(); 
                this->graphicsCommandBuffers[i].drawIndexed(numToDraw, 1, 0, vertexCount, 0);
                //vkCmdDrawIndexed(graphicsCommandBuffers[i], numToDraw, 1, 0, vertexCount, 0);
                vertexCount += renderObj->getNumIndicies(); 
            }

            //vkCmdDrawIndexed(graphicsCommandBuffers[i], 3, 1, 0, 0, 0); 

            //can now finish render pass
            //vkCmdEndRenderPass(graphicsCommandBuffers[i]);
            this->graphicsCommandBuffers[i].endRenderPass(); 

            //record command buffer
            if (vkEndCommandBuffer(graphicsCommandBuffers[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to record command buffer");
            }
        }
    }
}

void star::core::VulkanRenderer::createSemaphores() {
    imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);

    vk::SemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = vk::StructureType::eSemaphoreCreateInfo;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        this->imageAvailableSemaphores[i] = this->device.createSemaphore(semaphoreInfo);
        this->renderFinishedSemaphores[i] = this->device.createSemaphore(semaphoreInfo);

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
        this->inFlightFences[i] = this->device.createFence(fenceInfo);
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

vk::CommandBuffer star::core::VulkanRenderer::beginSingleTimeCommands(bool useTransferPool) {
    //allocate using temporary command pool
    vk::CommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = vk::StructureType::eCommandBufferAllocateInfo;
    allocInfo.level = vk::CommandBufferLevel::ePrimary;
    allocInfo.commandPool = useTransferPool ? transferCommandPool : graphicsCommandPool;
    allocInfo.commandBufferCount = 1;

    //TODO: this returns a vector -- need to make only return one 
    vk::CommandBuffer tmpCommandBuffer = this->device.allocateCommandBuffers(allocInfo).at(0);

    vk::CommandBufferBeginInfo beginInfo{};
    //beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.sType = vk::StructureType::eCommandBufferBeginInfo;
    beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit; //only planning on using this command buffer once 

    tmpCommandBuffer.begin(beginInfo);

    return tmpCommandBuffer;
}

void star::core::VulkanRenderer::endSingleTimeCommands(vk::CommandBuffer commandBuff, bool useTransferPool) {
    commandBuff.end();
    //submit the buffer for execution
    vk::SubmitInfo submitInfo{};
    submitInfo.sType = vk::StructureType::eSubmitInfo;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuff;

    if (useTransferPool) {
        this->transferQueue.submit(submitInfo);
        this->transferQueue.waitIdle();
        this->device.freeCommandBuffers(this->transferCommandPool, 1, &commandBuff);
    }
    else {
        //use graphics pool
        this->graphicsQueue.submit(submitInfo);
        this->graphicsQueue.waitIdle();
        this->device.freeCommandBuffers(this->graphicsCommandPool, 1, &commandBuff);
    }
}

void star::core::VulkanRenderer::copyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size) {
    bool useTransferPool = true;
    vk::CommandBuffer commandBuffer = beginSingleTimeCommands(useTransferPool);

    vk::BufferCopy copyRegion{};
    copyRegion.srcOffset = 0;
    copyRegion.dstOffset = 0;
    copyRegion.size = size;

    //note: cannot specify VK_WHOLE_SIZE as before 
    commandBuffer.copyBuffer(srcBuffer, dstBuffer, copyRegion);

    endSingleTimeCommands(commandBuffer, useTransferPool);
}

