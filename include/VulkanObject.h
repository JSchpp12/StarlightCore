#pragma once 

#include "SC/GameObject.hpp"
#include "SC/Handle.hpp"
#include "SC/Enums.h"
#include "SC/Shader.h"
#include "VulkanVertex.hpp"
#include "Star_RenderObject.hpp"

#include <vulkan/vulkan.hpp>

#include <memory>
#include <vector>
#include <map>

namespace star {
	namespace core {
		/// <summary>
		/// Base object which contains the per shader object data for vulkan. 
		/// </summary>
		class VulkanObject {
		public:
			class Builder {
			public:

			protected:

			private: 

			};
			std::vector<vk::Pipeline> pipelines;

			//vertex buffer
			vk::Buffer vertexBuffer;
			vk::DeviceMemory vertexBufferMemory;

			//index buffer 
			vk::Buffer indexBuffer;
			vk::DeviceMemory indexBufferMemory;

			uint64_t totalNumVerticies = 0; 
			uint64_t totalNumIndicies = 0; 

			//no copy
			VulkanObject(const VulkanObject&) = delete; 

			VulkanObject(vk::Device& device, size_t numSwapChainImages) :
				device(device),
				renderObjects() {};

			void cleanup(); 

			void registerShader(vk::ShaderStageFlagBits stage, common::Handle newShaderHandle); 

			/// <summary>
			/// Add a new rendering object which will be rendered with the pipeline contained in this vulkan object.
			/// </summary>
			/// <param name="newObjectHandle"></param>
			void addObject(common::Handle newObjectHandle, common::GameObject* newObject, size_t numSwapChainImages);

			/// <summary>
			/// Get the handle for one of the base shaders of this object.
			/// </summary>
			/// <param name="stage"></param>
			/// <returns></returns>
			common::Handle getBaseShader(vk::ShaderStageFlagBits stage); 

			/// <summary>
			/// Add a prepared vulkan shader module with this object
			/// </summary>
			/// <param name="stage"></param>
			/// <param name="newShaderModule"></param>
			void registerShaderModule(vk::ShaderStageFlagBits stage, vk::ShaderModule newShaderModule);

			/// <summary>
			/// Get the associated vk::ShaderModule for the provided stage.
			/// </summary>
			/// <param name="stage"></param>
			vk::ShaderModule getShaderModule(vk::ShaderStageFlagBits stage); 

			/// <summary>
			/// Check if the object has a shader for the requestd stage
			/// </summary>
			/// <param name="stage"></param>
			/// <returns></returns>
			bool hasShader(vk::ShaderStageFlagBits stage); 

			void setPipelineLayout(vk::PipelineLayout newPipelineLayout);

			vk::PipelineLayout getPipelineLayout(); 

			void addPipelines(std::vector<vk::Pipeline> newPipelines);

			size_t getNumRenderObjects(); 

			RenderObject* getRenderObjectAt(size_t index);

		protected:


		private:
			vk::Device& device; 

			//only 10 of these are permitted in general 
			vk::DescriptorPool descriptorPool; 

			std::map<vk::ShaderStageFlagBits, common::Handle> shaderContainer; 
			std::map<vk::ShaderStageFlagBits, vk::ShaderModule> shaderModuleContainer; 

			std::vector<std::unique_ptr<RenderObject>> renderObjects; 

			vk::PipelineLayout pipelineLayout;
		};
	}
}