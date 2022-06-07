#pragma once 
#include <vulkan/vulkan.hpp>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "SC/Renderer.hpp"
#include "SC/ConfigFile.hpp"
#include "SC/Handle.hpp"
#include "VulkanVertex.hpp"

#include <stb_image.h>

#include <string> 
#include <optional> 
#include <vector>
#include <memory>  
#include <set>
#include <chrono>

namespace star{
    namespace core{
        class VulkanRenderer : public common::Renderer{
        public:
            vk::Instance instance;
            vk::SurfaceKHR* surface;

            std::unique_ptr<const char**> glfwRequiredExtensions; 
            std::unique_ptr<uint32_t> glfwRequiredExtensionsCount; 
            
            VulkanRenderer(common::ConfigFile* configFile, common::FileResourceManager<common::Shader>* shaderManager, common::FileResourceManager<common::Object>* objectManager, common::FileResourceManager<common::Texture>* textureManager, std::vector<common::Handle>* objectHandleList);
            
            ~VulkanRenderer(); 

            //Create the vulkan instance machine 
            void createInstance();

            //attach vulkan to GLFW
            void prepareGLFW(const char** requiredExtensions, uint32_t numRequiredExtensions, GLFWwindow* window);

            void prepare();

            void drawFrame();  

            void cleanup(); 
        
        protected: 
            struct QueueFamilyIndices {
                std::optional<uint32_t> graphicsFamily;
                std::optional<uint32_t> presentFamily;
                std::optional<uint32_t> transferFamily;

                bool isComplete() {
                    return graphicsFamily.has_value() && presentFamily.has_value() && transferFamily.has_value();
                }
            };

            struct SwapChainSupportDetails {
                vk::SurfaceCapabilitiesKHR capabilities;
                std::vector<vk::SurfaceFormatKHR> formats;
                std::vector<vk::PresentModeKHR> presentModes;
            };

            #ifdef NDEBUG 
                const bool enableValidationLayers = false;
            #else
                const bool enableValidationLayers = true;
            #endif    

            bool frameBufferResized = false; //explicit declaration of resize, used if driver does not trigger VK_ERROR_OUT_OF_DATE

            //how many frames will be sent through the pipeline
            const int MAX_FRAMES_IN_FLIGHT = 2;

            //tracker for which frame is being processed of the available permitted frames
            size_t currentFrame = 0;

            //texture information
            vk::ImageView textureImageView;
            vk::Sampler textureSampler; 
            vk::Image textureImage; 
            vk::DeviceMemory textureImageMemory; 

            //Sync obj storage 
            std::vector<vk::Semaphore> imageAvailableSemaphores;
            std::vector<vk::Semaphore> renderFinishedSemaphores;

            //buffer and memory information storage
            vk::Buffer vertexBuffer;
            vk::DeviceMemory vertexBufferMemory;
            vk::Buffer indexBuffer; 
            vk::DeviceMemory indexBufferMemory;

            //vulkan command storage
            vk::CommandPool graphicsCommandPool;
            std::vector<vk::CommandBuffer> graphicsCommandBuffers;
            vk::CommandPool transferCommandPool;
            std::vector<vk::CommandBuffer> transferCommandBuffers;
            vk::CommandPool tempCommandPool; //command pool for temporary use in small operations

            //storage for multiple buffers for each swap chain image 
            std::vector<vk::Buffer> uniformBuffers; 
            std::vector<vk::DeviceMemory> uniformBuffersMemory;

            //pipeline and dependency storage
            vk::Pipeline graphicsPipeline;
            vk::RenderPass renderPass;
            vk::DescriptorSetLayout descriptorSetLayout;
            vk::PipelineLayout pipelineLayout;

            vk::DescriptorPool descriptorPool;
            std::vector<vk::DescriptorSet> descriptorSets; 

            //queue family
            vk::Queue graphicsQueue;
            vk::Queue presentQueue;
            vk::Queue transferQueue;

            vk::PhysicalDevice physicalDevice = VK_NULL_HANDLE;
            vk::Device device;

            //more swapchain info 
            vk::SwapchainKHR swapChain;
            std::vector<vk::Image> swapChainImages;
            vk::Format swapChainImageFormat;
            vk::Extent2D swapChainExtent;

            std::vector<vk::ImageView> swapChainImageViews;
            std::vector<vk::Framebuffer> swapChainFramebuffers;
            std::vector<vk::Fence> inFlightFences;
            std::vector<vk::Fence> imagesInFlight;

            //depth testing storage 
            vk::Image depthImage; 
            vk::DeviceMemory depthImageMemory; 
            vk::ImageView depthImageView; 

            const std::vector<const char*> validationLayers = {
                "VK_LAYER_KHRONOS_validation"
            };

            const std::vector<const char*> deviceExtensions = {
                VK_KHR_SWAPCHAIN_EXTENSION_NAME  //image presentation is not built into the vulkan core...need to enable it through an extension 
            };

            void updateUniformBuffer(uint32_t currentImage);

            void prepareGLFW(int width, int height ); 

            void initWindow(); 

            void cleanupSwapChain(); 

            /// <summary>
            /// Check if validation layers are supported and create the layers if needed. Will create layers for debugging builds only.
            /// </summary>
            /// <returns></returns>
            bool checkValidationLayerSupport();

            //Pick a proper physical GPU that matches the required extensions
            void pickPhysicalDevice();

            //Helper function to test each potential GPU device 
            bool isDeviceSuitable(vk::PhysicalDevice device);

            /// <summary>
            /// Find what queues are available for the device
            /// Queues support different types of commands such as : processing compute commands or memory transfer commands
            /// </summary>  
            QueueFamilyIndices findQueueFamilies(vk::PhysicalDevice device);

            /// <summary>
            /// Check if the given device supports required extensions.
            /// </summary>
            bool checkDeviceExtensionSupport(vk::PhysicalDevice device);

            /// <summary>
            /// Request specific details about swap chain support for a given device
            /// </summary>
            SwapChainSupportDetails querySwapChainSupport(vk::PhysicalDevice device);

            //Create a logical device to communicate with the physical device 
            void createLogicalDevice();     

            /// <summary>
            /// Create a swap chain that will be used in rendering images
            /// </summary>
            void createSwapChain();

            /// <summary>
            /// If the swapchain is no longer compatible, it must be recreated.
            /// </summary>
            void recreateSwapChain();

            vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats); 

            //Look through givent present modes and pick the "best" one
            vk::PresentModeKHR chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes);

            vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR & capabilities);

            /// <summary>
            /// Create an image view object for use in the rendering pipeline
            /// 'Image View': describes how to access an image and what part of an image to access
            /// </summary>
            void createImageViews();

            vk::ImageView createImageView(vk::Image image, vk::Format format, vk::ImageAspectFlagBits aspectFlags);

            /// <summary>
            /// Create a rendering pass object which will tell vulkan information about framebuffer attachments:
            /// number of color and depth buffers, how many samples to use for each, how to handle contents
            /// </summary>
            void createRenderPass();

            /// <summary>
            /// Helper function -- TODO 
            /// </summary>
            /// <returns></returns>
            vk::Format findDepthFormat();

            /// <summary>
            /// Check the hardware to make sure that the supplied formats are compatible with the current system. 
            /// </summary>
            /// <param name="candidates">List of possible formats to check</param>
            /// <param name="tiling"></param>
            /// <param name="features"></param>
            /// <returns></returns>
            vk::Format findSupportedFormat(const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlagBits features);

            /// <summary>
            /// Create the descriptors for the buffers that will be passed to the GPU with additional information regarding verticies. (model-view-projection matricies)
            /// </summary>
            void createDescriptorSetLayout();

            /// <summary>
            /// Create a graphics pipeline to handle the needs for the application with the vertex and fragment shaders. The pipeline is immutable so it must be created if any changes are needed.
            /// </summary>
            void createGraphicsPipeline();

            /// <summary>
            /// Create a shader module from bytecode. The shader module is a wrapper around the shader code with function definitions. 
            /// </summary>
            /// <param name="code">bytecode for the shader program</param>
            /// <returns></returns>
            vk::ShaderModule createShaderModule(const std::vector<uint32_t>& code);

            /// <summary>
            /// Create the depth images that will be used by vulkan to run depth tests on fragments. 
            /// </summary>
            void createDepthResources();

            /// <summary>
            /// Create Vulkan Image object with properties provided in function arguments. 
            /// </summary>
            /// <param name="width">Width of the image being created</param>
            /// <param name="height">Height of the image being created</param>
            /// <param name="format"></param>
            /// <param name="tiling"></param>
            /// <param name="usage"></param>
            /// <param name="properties"></param>
            /// <param name="image"></param>
            /// <param name="imageMemory"></param>
            void createImage(uint32_t width, uint32_t height, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage, vk::MemoryPropertyFlagBits properties, vk::Image& image, vk::DeviceMemory& imageMemory);

            /// <summary>
            /// Query the GPU for the proper memory type that matches properties defined in passed arguments. 
            /// </summary>
            /// <param name="typeFilter">Which bit field of memory types that are suitable</param>
            /// <param name="properties"></param>
            /// <returns></returns>
            uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties);

            /// <summary>
            /// Create framebuffers that will hold representations of the images in the swapchain
            /// </summary>
            void createFramebuffers();

            /// <summary>
            /// Create command pools which will contain all predefined draw commands for later use in vulkan main loop
            /// </summary>
            void createCommandPools();

            void createPool(uint32_t queueFamilyIndex, vk::CommandPoolCreateFlagBits flags, vk::CommandPool& pool);

            /// <summary>
            /// Create an image for use as a texture by vulkan
            /// </summary>
            void createTextureImage();

            /// <summary>
            /// Copy a buffer to an image.
            /// </summary>
            /// <param name="buffer"></param>
            /// <param name="image"></param>
            /// <param name="width"></param>
            /// <param name="height"></param>
            void copyBufferToImage(vk::Buffer buffer, vk::Image image, uint32_t width, uint32_t height);

            /// <summary>
            /// Create a buffer with the given arguments
            /// </summary>
            void createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, vk::Buffer& buffer, vk::DeviceMemory& bufferMemory);

            /// <summary>
            /// Transition VKimage object from old layout to new layout 
            /// </summary>
            /// <param name="image"></param>
            /// <param name="format"></param>
            /// <param name="oldLayout"></param>
            /// <param name="newLayout"></param>
            void transitionImageLayout(vk::Image image, vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout);

            /// <summary>
            /// Create the image view for the texture image
            /// </summary>
            void createTextureImageView();

            /// <summary>
            /// Create a sampler for the texture image that will be used by the shader to address the colors in the texture.
            /// </summary>
            void createTextureSampler();

            /// <summary>
            /// Create a vertex buffer to hold the vertex information that will be passed to the GPU. 
            /// </summary>
            void createVertexBuffer();

            /// <summary>
            /// Create a buffer to contain vertex indicies information before being passed to the GPU. 
            /// </summary>
            void createIndexBuffer();

            /// <summary>
            /// Create a buffer to hold the UBO data for each shader. Create a buffer for each swap chain image
            /// </summary>
            void createRenderingBuffers();

            /// <summary>
            /// Create descriptor pools to bind the uniform buffer descriptor to each VkBuffer. 
            /// </summary>
            void createDescriptorPool();

            /// <summary>
            /// Allocate memory for the descriptor sets. 
            /// </summary>
            void createDescriptorSets();

            /// <summary>
            /// Allocate and record the commands for each swapchain image
            /// </summary>
            void createCommandBuffers();

            /// <summary>
            /// Create semaphores that are going to be used to sync rendering and presentation queues
            /// </summary>
            void createSemaphores();

            /// <summary>
            /// Fences are needed for CPU-GPU sync. Creates these required objects
            /// </summary>
            void createFences();

            /// <summary>
            /// Create tracking information in order to link fences with the swap chain images using 
            /// </summary>
            void createFenceImageTracking();

#pragma region HelperFunctions 
            /// <summary>
            /// Helper function to execute single time use command buffers
            /// </summary>
            /// <param name="useTransferPool">Should command be submitted to the transfer command pool. Will be submitted to the graphics pool otherwise.</param>
            /// <returns></returns>
            vk::CommandBuffer beginSingleTimeCommands(bool useTransferPool = false);

            /// <summary>
            /// Helper function to end execution of single time use command buffer
            /// </summary>
            /// <param name="commandBuffer"></param>
            /// <param name="useTransferPool">Was command buffer submitted to the transfer pool. Assumed graphics pool otherwise.</param>
            void endSingleTimeCommands(vk::CommandBuffer commandBuff, bool useTransferPool = false);

            void copyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size);

#pragma endregion
        private:  
            GLFWwindow* glfwWindow; 
            std::vector<common::Handle>* objectHandles; 

#pragma region DebugVars
            size_t numVerticies = 0;
            size_t numIndicies = 0;
#pragma endregion
            
        }; 
    }
}