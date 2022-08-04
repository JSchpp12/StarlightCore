#pragma once 

#include "SC/GameObject.hpp"
#include "SC/Handle.hpp"
#include "SC/Enums.h"
#include "SC/Shader.h"
#include "SC/Light.hpp"
#include "SC/Material.hpp"
#include "VulkanVertex.hpp"
#include "Star_RenderObject.hpp"
#include "Star_Descriptors.hpp"
#include "Star_Device.hpp"
#include "Star_Pipeline.hpp"
#include "Star_Buffer.hpp"
#include "StarSystem_Render.hpp"

#include <vulkan/vulkan.hpp>

#include <memory>
#include <vector>
#include <map>

namespace star::core{
	/// <summary>
	/// Base object which contains the per shader object data for vulkan. 
	/// </summary>
	class RenderSysObj : public RenderSystem{
	public:
		/// <summary>
		/// Object type to be used per render object, updated each frame
		/// </summary>
		struct UniformBufferObject {
			glm::mat4 modelMatrix;
			glm::mat4 normalMatrix;
		};

		RenderSysObj(StarDevice& device, size_t numSwapChainImages, vk::DescriptorSetLayout globalSetLayout, 
			vk::Extent2D swapChainExtent, vk::RenderPass renderPass) 
			: RenderSystem(device), numSwapChainImages(numSwapChainImages), 
			globalSetLayout(globalSetLayout), swapChainExtent(swapChainExtent),
			renderPass(renderPass) {};

		RenderSysObj(const RenderSysObj& baseObject); 

		virtual ~RenderSysObj();

		virtual void setPipelineLayout(vk::PipelineLayout newPipelineLayout);

		virtual void updateBuffers(uint32_t currentImage); 

		virtual void init(std::vector<vk::DescriptorSetLayout> globalDescriptorSets) override; 

		/// <summary>
		/// Render the object
		/// </summary>
		virtual void render(vk::CommandBuffer& commandBuffer, int swapChainImageIndex);

		virtual RenderObject* getRenderObjectAt(size_t index);								//this really needs to be removed
		virtual vk::PipelineLayout getPipelineLayout() { return this->pipelineLayout; }
		virtual StarDescriptorSetLayout* getSetLayout() { return this->descriptorSetLayout.get(); }
		virtual StarDescriptorPool* getDescriptorPool() { return this->descriptorPool.get(); }
		virtual StarBuffer* getBufferAt(int i) { return this->uniformBuffers.at(i).get(); }
		uint32_t getNumVerticies() { return this->totalNumVerticies; }
		virtual size_t getNumRenderObjects();
	protected:
		bool ownerOfSetLayout = true; 
		int numSwapChainImages = 0;

		vk::Extent2D swapChainExtent; 
		vk::RenderPass renderPass; 

		vk::DescriptorSetLayout globalSetLayout; 
		std::unique_ptr<StarDescriptorPool> descriptorPool; 

		std::vector<std::unique_ptr<StarBuffer>> uniformBuffers;

		std::unique_ptr<StarDescriptorSetLayout> staticDescriptorSetLayout;		//descriptor set layout for object data updated once
		std::unique_ptr<StarDescriptorSetLayout> descriptorSetLayout; 

		std::vector<std::vector<vk::DescriptorSet>> staticDescriptorSets;		//descriptor sets for object data updated on init
		std::vector<std::vector<vk::DescriptorSet>> descriptorSets; 

		virtual void createDescriptorPool() override; 
		/// <summary>
		/// Create buffers needed for render operations. Such as those used by descriptors
		/// </summary>
		virtual void createRenderBuffers() override;

		virtual void createDescriptorLayouts() override;
		/// <summary>
		/// Create descriptors for binding render buffers to shaders.
		/// </summary>
		virtual void createDescriptors() override;

		virtual void createPipelineLayout(std::vector<vk::DescriptorSetLayout> globalDescriptorSets) override;

		virtual void createPipeline() override; 

	private:

	};
}