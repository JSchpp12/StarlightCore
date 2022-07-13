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

#include <vulkan/vulkan.hpp>

#include <memory>
#include <vector>
#include <map>

namespace star::core{
	/// <summary>
	/// Base object which contains the per shader object data for vulkan. 
	/// </summary>
	class RenderSysObj {
	public:
		/// <summary>
		/// Object type containing per render object information updated only once on scene init
		/// </summary>
		struct MaterialBufferObject {
			glm::vec4 surfaceColor; 
			glm::vec4 highlightColor; 
			int shinyCoefficient; 
		};
		/// <summary>
		/// Object type to be used per render object, updated each frame
		/// </summary>
		struct UniformBufferObject {
			alignas(16) glm::mat4 modelMatrix;
			alignas(16) glm::mat4 normalMatrix;
		};

		class Builder {
		public:

		protected:

		private: 

		};
		uint32_t totalNumVerticies = 0; 

		RenderSysObj(StarDevice* device, size_t numSwapChainImages, vk::DescriptorSetLayout globalSetLayout, 
			vk::Extent2D swapChainExtent, vk::RenderPass renderPass) :
			starDevice(device), numSwapChainImages(numSwapChainImages), 
			globalSetLayout(globalSetLayout), swapChainExtent(swapChainExtent),
			renderPass(renderPass) {};

		RenderSysObj(const RenderSysObj& baseObject); 

		virtual ~RenderSysObj();

		virtual void init(std::vector<vk::DescriptorSetLayout> globalDescriptorSets);

		virtual void registerShader(vk::ShaderStageFlagBits stage, common::Shader& newShader, common::Handle newShaderHandle);

		/// <summary>
		/// Add a new rendering object which will be rendered with the pipeline contained in this vulkan object.
		/// </summary>
		/// <param name="newObjectHandle"></param>
		//virtual void addObject(common::Handle newObjectHandle, common::GameObject* newObject, size_t numSwapChainImages);
		/// <summary>
		/// Register a light color and location for rendering light effects on objects
		/// </summary>
		virtual void addObject(std::unique_ptr<RenderObject> newRenderObject);
		virtual void addLight(common::Light* newLight) { this->lights.push_back(newLight); }
		/// <summary>
		/// Check if the object has a shader for the requestd stage
		/// </summary>
		/// <param name="stage"></param>
		/// <returns></returns>
		virtual bool hasShader(vk::ShaderStageFlagBits stage);

		virtual common::Handle getBaseShader(vk::ShaderStageFlags stage);

		virtual void setPipelineLayout(vk::PipelineLayout newPipelineLayout);

		virtual size_t getNumRenderObjects();

		virtual RenderObject* getRenderObjectAt(size_t index);

		virtual void bind(vk::CommandBuffer& commandBuffer);

		virtual void updateBuffers(uint32_t currentImage); 

		//void createPipeline(PipelineConfigSettings& settings); 

		/// <summary>
		/// Render the object
		/// </summary>
		virtual void render(vk::CommandBuffer& commandBuffer, int swapChainImageIndex);

		virtual void setAmbientLight(common::Light* newLight) { this->ambientLight = newLight; }
		virtual bool hasAmbientLight() { return this->ambientLight == nullptr ? false : true; }

		//TODO: remove
		virtual vk::PipelineLayout getPipelineLayout() { return this->pipelineLayout; }
		virtual StarDescriptorSetLayout* getSetLayout() { return this->descriptorSetLayout.get(); }
		virtual StarDescriptorPool* getDescriptorPool() { return this->descriptorPool.get(); }
		virtual StarBuffer* getBufferAt(int i) { return this->uniformBuffers.at(i).get(); }
		uint32_t getNumVerticies() { return this->totalNumVerticies; }
	protected:
		bool ownerOfSetLayout = true; 
		StarDevice* starDevice;
		int numSwapChainImages = 0;

		vk::Extent2D swapChainExtent; 
		vk::RenderPass renderPass; 

		std::unique_ptr<StarBuffer> vertexBuffer;
		std::unique_ptr<StarBuffer> indexBuffer;

		common::Handle vertShaderHandle;
		common::Shader* vertShader = nullptr;
		common::Handle fragShaderHandle;
		common::Shader* fragShader = nullptr;

		common::Light* ambientLight = nullptr;

		vk::DescriptorSetLayout globalSetLayout; 
		std::vector<common::Light*> lights; 
		std::vector<std::unique_ptr<RenderObject>> renderObjects;
		std::unique_ptr<StarDescriptorPool> descriptorPool; 
		std::unique_ptr<StarPipeline> starPipeline;
		vk::PipelineLayout pipelineLayout;
		std::unique_ptr<StarBuffer> objectMaterialBuffer;						//buffer which contains object information updated on scene init
		std::vector<std::unique_ptr<StarBuffer>> uniformBuffers;

		std::unique_ptr<StarDescriptorSetLayout> staticDescriptorSetLayout;		//descriptor set layout for object data updated once
		std::unique_ptr<StarDescriptorSetLayout> descriptorSetLayout; 

		std::vector<std::vector<vk::DescriptorSet>> staticDescriptorSets;		//descriptor sets for object data updated on init
		std::vector<std::vector<vk::DescriptorSet>> descriptorSets; 

		/// <summary>
		/// Concat all verticies of all objects into a single buffer and copy to gpu.
		/// </summary>
		virtual void createVertexBuffer();

		virtual void createIndexBuffer();

		virtual void createDescriptorPool(); 
		/// <summary>
		/// Create buffers needed for render operations. Such as those used by descriptors
		/// </summary>
		virtual void createRenderBuffers();
		virtual void createObjectMaterialBuffer();
		virtual void createStaticDescriptors();
		/// <summary>
		/// Create descriptors for binding render buffers to shaders.
		/// </summary>
		virtual void createDescriptors();

		virtual void createPipelineLayout(std::vector<vk::DescriptorSetLayout> globalDescriptorSets);

		virtual void createPipeline(); 

	private:

	};
}