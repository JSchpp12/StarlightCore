/*
* Wrapper class for the swapchain to be used in rendering
*/
#pragma once 
#include "Star_Device.hpp"

#include <vulkan/vulkan.hpp>

#include <memory>
#include <vector>

namespace star {
namespace core {
	class StarSwapChain {
	public:
		static const int MAX_FRAMES_IN_FLIGHT = 2;

		StarSwapChain(StarDevice& device, vk::Extent2D windowExtent);
		StarSwapChain(StarDevice& device, vk::Extent2D windowExtent, std::shared_ptr<StarSwapChain> previous);
		~StarSwapChain(); 

		StarSwapChain(const StarSwapChain&) = delete; 
		StarSwapChain& operator=(const StarSwapChain&) = delete; 

		vk::Framebuffer getFrameBuffer(int index){}

	private:
		vk::Format swapChainImageFormat; 
		vk::Format swapChainDepthFormat; 
		vk::Extent2D swapChainExtent; 

		std::vector<vk::Framebuffer> swapChainFramebuffers; 
		vk::RenderPass renderPass; 

		std::vector<vk::Image> depthImages; 
		std::vector<vk::DeviceMemory> depthImageMemory; 
		std::vector<vk::ImageView> depthImageViews; 
		std::vector<vk::Image> swapChainImages; 
		std::vector<vk::ImageView> swapChainImageViews; 

		StarDevice& starDevice; 
		vk::Extent2D windowExtent; 

		vk::SwapchainKHR swapChain; 
		std::shared_ptr<StarSwapChain> oldSwapChain; 

		std::vector<vk::Semaphore> imageAvailableSemaphore; 
		std::vector<vk::Semaphore> renderFinishedSemaphore; 
		std::vector<vk::Fence> inFlightFences; 
		std::vector<vk::Fence> imagesInFlight; 
		size_t currentFrame = 0; 

		void init(); 
		void createSwapChain();
		void createImageViews();
		void createDepthResources();
		void createRenderPass();
		void createFramebuffers();
		void createSyncObjects();
	};
}
}