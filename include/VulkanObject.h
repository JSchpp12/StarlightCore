#pragma once 

#include "SC/GameObject.hpp"
#include "SC/Handle.hpp"
#include "SC/Enums.h"
#include "SC/Shader.h"
#include "VulkanVertex.hpp"
#include "Star_RenderObject.hpp"
#include "Star_Descriptors.hpp"
#include "Star_Device.hpp"
#include "Star_Pipeline.hpp"

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

			VulkanObject(StarDevice* device, size_t numSwapChainImages) :
				starDevice(device) {};

			void cleanup(); 

			void registerShader(vk::ShaderStageFlagBits stage, common::Shader* newShader, common::Handle newShaderHandle);

			/// <summary>
			/// Add a new rendering object which will be rendered with the pipeline contained in this vulkan object.
			/// </summary>
			/// <param name="newObjectHandle"></param>
			void addObject(common::Handle newObjectHandle, common::GameObject* newObject, size_t numSwapChainImages);

			/// <summary>
			/// Check if the object has a shader for the requestd stage
			/// </summary>
			/// <param name="stage"></param>
			/// <returns></returns>
			bool hasShader(vk::ShaderStageFlagBits stage); 

			common::Handle getBaseShader(vk::ShaderStageFlags stage);


			void setPipelineLayout(vk::PipelineLayout newPipelineLayout);

			vk::PipelineLayout getPipelineLayout(); 

			void addPipeline(std::unique_ptr<StarPipeline> newPipeline);

			size_t getNumRenderObjects(); 

			RenderObject* getRenderObjectAt(size_t index);

			void createPipeline(PipelineConfigSettings& configs);

			vk::Pipeline getPipeline() { return this->starPipeline->getPipeline(); }

		protected:


		private:
			StarDevice* starDevice; 

			//only 10 of these are permitted in general 
			vk::DescriptorPool descriptorPool;

			common::Handle vertShaderHandle; 
			common::Shader* vertShader = nullptr; 
			common::Handle fragShaderHandle; 
			common::Shader* fragShader = nullptr; 

			std::vector<std::unique_ptr<RenderObject>> renderObjects; 
			std::unique_ptr<StarPipeline> starPipeline;
			vk::PipelineLayout pipelineLayout;

		};
	}
}