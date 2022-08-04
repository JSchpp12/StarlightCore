/*
* This is the pure virtual class that will define what a basic render system in the starlight engine should implement 
* and be the owner of. 
*/
#pragma once 
#include "SC/Shader.h"
#include "SC/Handle.hpp"
#include "SC/GameObject.hpp"
#include "SC/Light.hpp"
#include "Star_Device.hpp"
#include "Star_RenderObject.hpp"

#include <vector>
#include <vulkan/vulkan.hpp>

#include <memory>

namespace star::core {
	class RenderSystem {
	public:
		RenderSystem(StarDevice& device);

		virtual ~RenderSystem(); 
		/// <summary>
		/// Setup needed buffers and descriptors needed for rendering operations
		/// </summary>
		/// <param name="globalDescriptorSets"></param>
		virtual void init(std::vector<vk::DescriptorSetLayout> globalDescriptorSets);
		/// <summary>
		/// Bind this rendering system's pipeline 
		/// </summary>
		/// <param name="commandBuffer"></param>
		virtual void bind(vk::CommandBuffer& commandBuffer);

		virtual void registerShader(vk::ShaderStageFlagBits stage, common::Shader& newShader, common::Handle newShaderHandle);
		/// <summary>
		/// Add a new rendering object which will be rendered with the pipeline contained in this vulkan object.
		/// </summary>
		virtual void addObject(std::unique_ptr<RenderObject> newRenderObject);

		virtual void addLight(common::Light* newLight) { this->lights.push_back(newLight); }
		/// <summary>
		/// Called during draw operations. This function is responsible for updating all needed buffers before drawing each frame.
		/// </summary>
		/// <param name="currentImage">Index of current swap chain image being drawn to (important for buffers which are created for each image)</param>
		virtual void updateBuffers(uint32_t currentImage) = 0;
		/// <summary>
		/// Called during creation of command buffers. This function should define what operations are needed in order to draw contained images.
		/// Such as binding descriptors or actually making the call to draw the object. 
		/// </summary>
		/// <param name="commandBUffer"></param>
		/// <param name="swapChainImageIndex"></param>
		virtual void render(vk::CommandBuffer& commandBuffer, int swapChainImageIndex) = 0; 
		//TODO: combine these and remove one 
		virtual bool hasShader(vk::ShaderStageFlagBits stage); 
		virtual common::Handle getBaseShader(vk::ShaderStageFlags stage); 
		uint32_t getNumVerticies() { return this->totalNumVerticies; }
	protected:
		StarDevice& starDevice; 
		int numMeshes = 0;
		uint32_t totalNumVerticies = 0;
		common::Handle vertShaderHandle;
		common::Shader* vertShader = nullptr;
		common::Handle fragShaderHandle;
		common::Shader* fragShader = nullptr;

		//rendering objects
		std::vector<std::unique_ptr<RenderObject>> renderObjects;		//objects registered to be rendered by this system
		std::vector<common::Light*> lights;

		std::unique_ptr<StarPipeline> starPipeline;

		std::unique_ptr<StarBuffer> vertexBuffer;
		std::unique_ptr<StarBuffer> indexBuffer;

		vk::PipelineLayout pipelineLayout;

		/// <summary>
		/// Creates a buffer to contain the verticies to be passed to the gpu. 
		/// </summary>
		virtual void createVertexBuffer(); 
		/// <summary>
		/// Create the index buffer to contain the indicies to be used during indexed draw operations. 
		/// </summary>
		virtual void createIndexBuffer(); 
		/// <summary>
		/// Create buffers which will contain mapped data to be used in shaders. Such as uniform data.
		/// </summary>
		virtual void createRenderBuffers() = 0; 
		/// <summary>
		/// Function which creates the needed pools to be used when creating descriptors
		/// </summary>
		virtual void createDescriptorPool() = 0;
		/// <summary>
		/// Creates the definition for layouts of descriptor sets
		/// </summary>
		virtual void createDescriptorLayouts() = 0; 
		/// <summary>
		/// Function which defines and creates needed descriptors for shaders
		/// </summary>
		virtual void createDescriptors() = 0; 
		/// <summary>
		/// Function to create pipeline layout
		/// </summary>
		/// <param name="globalDescriptorSets">Descriptor sets for the core rendering system</param>
		virtual void createPipelineLayout(std::vector<vk::DescriptorSetLayout> globalDescriptorSets) = 0;
		/// <summary>
		/// Function to create the pipeline needed by the rendering system.
		/// </summary>
		virtual void createPipeline() = 0; 

	private:
		/// <summary>
		/// Function which ensures that needed rendering objects were created by child class
		/// </summary>
		void checkRenderRequirenments(); 

	};
}