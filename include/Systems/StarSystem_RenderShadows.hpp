/* 
* This class will contain the render system that will be used to create a basic render pipeline to handle rendering
* shadow maps to the depth map attached to the frame buffers.
*/
#pragma once
#include "StarSystem_Render.hpp"
#include "Star_Buffer.hpp"
#include "Star_Descriptors.hpp"
#include "Star_Pipeline.hpp"
#include "SC/Camera.hpp"

#include <vulkan/vulkan.hpp>
#include <glm/glm.hpp>

namespace star::core {
	class RenderSys_Shadows : public RenderSystem {
	public:
		/// <summary>
		/// Object type to be used per render object, updated each frame
		/// </summary>
		struct UniformBufferObject {
			glm::mat4 modelMatrix	= glm::mat4();
		};

		//Lights are treated as cameras
		struct LightPositionBufferObject {
			glm::mat4 viewMatrix = glm::mat4();
		};

		RenderSys_Shadows(StarDevice& device, size_t numSwapChainImages, vk::DescriptorSetLayout globalSetLayout,
			const vk::Extent2D& swapChainExtent, const vk::RenderPass& renderPass) 
			: RenderSystem(device), numSwapChainImages(numSwapChainImages),
			swapChainExtent(swapChainExtent), renderPass(renderPass) { }

		virtual ~RenderSys_Shadows();

		virtual void render(vk::CommandBuffer& commandBuffer, int swapChainImageIndex) override;
		/// <summary>
		/// End Render Pass
		/// </summary>
		/// <param name="commandBuffer"></param>
		virtual void end(vk::CommandBuffer& commandBuffer) { commandBuffer.endRenderPass(); };

		virtual void updateBuffers(uint32_t currentImage) override;

	protected:
		const vk::Extent2D& swapChainExtent;
		const vk::RenderPass& renderPass; 
		std::vector<std::unique_ptr<StarBuffer>> uniformBuffers; 
		std::vector<std::unique_ptr<StarBuffer>> lightInfoBuffers; 
		std::unique_ptr<StarDescriptorPool> descriptorPool; 
		std::unique_ptr<StarDescriptorSetLayout> descriptorSetLayout; 
		std::vector<vk::DescriptorSet> lightDescriptors; 

		virtual void createRenderBuffers() override; 
		/// <summary>
		/// Creates a buffer to contain the verticies to be passed to the gpu. 
		/// </summary>
		//virtual void createVertexBuffer() override;
		/// <summary>
		/// Function which creates the needed pools to be used when creating descriptors
		/// </summary>
		virtual void createDescriptorPool() override;
		/// <summary>
		/// Creates the definition for layouts of descriptor sets
		/// </summary>
		virtual void createDescriptorLayouts() override;
		/// <summary>
		/// Function which defines and creates needed descriptors for shaders
		/// </summary>
		virtual void createDescriptors() override;
		/// <summary>
		/// Function to create pipeline layout
		/// </summary>
		/// <param name="globalDescriptorSets">Descriptor sets for the core rendering system</param>
		virtual void createPipelineLayout(std::vector<vk::DescriptorSetLayout> globalDescriptorSets) override;
		/// <summary>
		/// Function to create the pipeline needed by the rendering system.
		/// </summary>
		virtual void createPipeline() override;

	private: 
		int numSwapChainImages = 0;



	};
}