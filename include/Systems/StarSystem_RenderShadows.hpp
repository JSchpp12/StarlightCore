/* 
* This class will contain the render system that will be used to create a basic render pipeline to handle rendering
* shadow maps to the depth map attached to the frame buffers.
*/
#pragma once
#include "StarSystem_RenderObj.hpp"
#include "Star_Buffer.hpp"

#include <vulkan/vulkan.hpp>

namespace star::core {
	class RenderSys_Shadows : RenderSysObj {
	public:
		RenderSys_Shadows(StarDevice& device, size_t numSwapChainImages, vk::DescriptorSetLayout globalSetLayout,
			vk::Extent2D swapChainExtent, vk::RenderPass renderPass) 
			: RenderSysObj(device, numSwapChainImages, globalSetLayout, swapChainExtent, renderPass) {}


	protected:

	private: 

	};
}