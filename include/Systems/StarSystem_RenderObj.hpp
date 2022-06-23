#pragma once 

#include "SC/GameObject.hpp"
#include "SC/Handle.hpp"
#include "SC/Enums.h"
#include "SC/Shader.h"
#include "SC/Light.hpp"
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

namespace star {
	namespace core {
		/// <summary>
		/// Base object which contains the per shader object data for vulkan. 
		/// </summary>
		class RenderSysObj {
		public:
			class Builder {
			public:

			protected:

			private: 

			};
			//vertex buffer

			void init();

			uint32_t totalNumVerticies = 0; 
			uint32_t totalNumIndicies = 0;

			//no copy
			RenderSysObj(const RenderSysObj&) = delete; 

			RenderSysObj(StarDevice* device, size_t numSwapChainImages) :
				starDevice(device), numSwapChainImages(numSwapChainImages) {};
			~RenderSysObj(); 

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

			size_t getNumRenderObjects(); 

			RenderObject* getRenderObjectAt(size_t index);

			void createPipeline(PipelineConfigSettings& configs);

			/// <summary>
			/// Render the object
			/// </summary>
			void render(vk::CommandBuffer& commandBuffer); 

			void setPointLight(common::Light* newLight) { this->pointLight = newLight; }
			common::Light* getPointLight() { return this->pointLight; }
			bool hasPointLight() { return this->pointLight == nullptr ? false : true; }
			void setAmbientLight(common::Light* newLight) { this->ambientLight = newLight; }
			bool hasAmbientLight() { return this->ambientLight == nullptr ? false : true; }

		private:
			StarDevice* starDevice; 
			int numSwapChainImages = 0; 

			std::unique_ptr<StarBuffer> vertexBuffer;
			std::unique_ptr<StarBuffer> indexBuffer; 

			//only 10 of these are permitted in general 
			vk::DescriptorPool descriptorPool;

			common::Handle vertShaderHandle; 
			common::Shader* vertShader = nullptr; 
			common::Handle fragShaderHandle; 
			common::Shader* fragShader = nullptr; 

			common::Light* ambientLight = nullptr;
			common::Light* pointLight = nullptr; 

			std::vector<std::unique_ptr<RenderObject>> renderObjects; 
			std::unique_ptr<StarPipeline> starPipeline;
			vk::PipelineLayout pipelineLayout;

 
			/// <summary>
			/// Concat all verticies of all objects into a single buffer and copy to gpu.
			/// </summary>
			void createVertexBuffer(); 

			void createIndexBuffer();
		};
	}
}