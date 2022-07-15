#include "Star_Swapchain.hpp"

namespace star::core{
	StarSwapChain::StarSwapChain(StarDevice& device, vk::Extent2D extent) 
		: starDevice{ device }, windowExtent{ extent }{
		init(); 
	}

	StarSwapChain::StarSwapChain(StarDevice& device, vk::Extent2D extent, std::shared_ptr<StarSwapChain> previous)
		: starDevice{ device }, windowExtent{ extent }, oldSwapChain{ previous }{
		init();
	}

	void StarSwapChain::init() {
		createSwapChain();
		createImageViews();
		createRenderPass();
		createDepthResources();
		createFramebuffers();
		createSyncObjects();
	}

	void StarSwapChain::createSwapChain() {
		SwapChainSupportDetails swapChainSupport = this->starDevice.getSwapChainSupportDetails(); 

		vk::SurfaceFormatKHR surfaceFormat;
	}
}