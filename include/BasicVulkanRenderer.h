#pragma once 
#include <vulkan/vulkan.hpp>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "SC/Renderer.hpp"
#include "SC/ConfigFile.hpp"
#include "SC/Handle.hpp"
#include "SC/Camera.hpp"
#include "SC/Enums.h"
#include "SC/Light.hpp"
#include "SC/MemoryManager.hpp"
#include "SC/RenderOptions.hpp"
//managers
#include "MaterialManager.hpp"
#include "TextureManager.hpp"
#include "MapManager.hpp"
//vulkan wrappers
#include "VulkanVertex.hpp"
#include "Star_Descriptors.hpp"
#include "Star_Device.hpp"
#include "Star_Buffer.hpp"
#include "Star_RenderMesh.hpp"
#include "Star_RenderMaterial.hpp"

//render systems
#include "StarSystem_RenderPointLight.hpp"
#include "StarSystem_RenderObj.hpp"

#include <glm/glm.hpp>

#include <string> 
#include <optional> 
#include <vector>
#include <memory>  
#include <set>
#include <chrono>

namespace star::core{
    class VulkanRenderer : public common::Renderer {
    public:
        VulkanRenderer(common::ConfigFile& configFile, common::RenderOptions& renderOptions, common::FileResourceManager<common::Shader>& shaderManager, 
            common::FileResourceManager<common::GameObject>& objectManager, TextureManager& textureManager, MapManager& mapManager,
            MaterialManager& materialManager, common::Camera& inCamera,
            std::vector<common::Handle>& objectHandleList, std::vector<common::Light*>& inLightList,
            StarWindow& window);

        ~VulkanRenderer();

        void pollEvents();

        void prepare();

        void draw();

        void cleanup();

    protected:
        struct LightBufferObject {
            glm::vec4 position      = glm::vec4(1.0f);
            glm::vec4 direction     = glm::vec4(1.0f);     //direction in which the light is pointing
            glm::vec4 ambient       = glm::vec4(1.0f);
            glm::vec4 diffuse       = glm::vec4(1.0f);
            glm::vec4 specular      = glm::vec4(1.0f);
            //controls.x = inner cutoff diameter 
            //controls.y = outer cutoff diameter
            glm::vec4 controls      = glm::vec4(0.0f);       //container for single float values
            //settings.x = enabled
            //settings.y = type
            glm::uvec4 settings     = glm::uvec4(0);    //container for single uint values
        };
        MaterialManager& materialManager; 
        TextureManager& textureManager; 
        MapManager& mapManager; 
        std::vector<common::Light*>& lightList;

        std::unique_ptr<StarDevice> starDevice{};

        std::vector<std::unique_ptr<RenderSysObj>> RenderSysObjs;
        std::unique_ptr<RenderSysPointLight> lightRenderSys;

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


        //storage for multiple buffers for each swap chain image  
        std::vector<std::unique_ptr<StarBuffer>> globalUniformBuffers;
        std::vector<vk::DescriptorSet> globalDescriptorSets;
        std::vector<std::unique_ptr<StarBuffer>> lightBuffers; 
        std::vector<vk::DescriptorSet> lightDescriptorSets; 

        //pipeline and dependency storage
        vk::RenderPass renderPass;

        //more swapchain info 
        vk::SwapchainKHR swapChain;
        std::vector<vk::Image> swapChainImages;
        vk::Format swapChainImageFormat;
        vk::Extent2D swapChainExtent;

        std::vector<vk::ImageView> swapChainImageViews;
        std::vector<vk::Framebuffer> swapChainFramebuffers;
        std::vector<vk::Fence> inFlightFences;
        std::vector<vk::Fence> imagesInFlight;

        std::unique_ptr<StarDescriptorPool> globalPool{};
        //std::unique_ptr<StarDescriptorPool> perObjectDynamicPool{}; 
        //std::unique_ptr<StarDescriptorPool> perObjectStaticPool{}; 
        std::unique_ptr<StarDescriptorSetLayout> globalSetLayout{};
        //std::unique_ptr<StarDescriptorSetLayout> perObjectStaticLayout{};

        //depth testing storage 
        vk::Image depthImage;
        vk::DeviceMemory depthImageMemory;
        vk::ImageView depthImageView;

        void updateUniformBuffer(uint32_t currentImage);

        void cleanupSwapChain();

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

        vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities);

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
        /// Create framebuffers that will hold representations of the images in the swapchain
        /// </summary>
        void createFramebuffers();

        /// <summary>
        /// Create a buffer to hold the UBO data for each shader. Create a buffer for each swap chain image
        /// </summary>
        void createRenderingBuffers();

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
#pragma endregion
    private:
        StarWindow& starWindow;
        std::vector<common::Handle>* objectHandles;

    };
}