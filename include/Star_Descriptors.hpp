//This file contains wrapper classes which pertain to vulkan descriptor use and creation

#pragma once 

#include <vulkan/vulkan.hpp>

#include <memory>
#include <unordered_map>
namespace star {
	namespace core{
		class StarDescriptorSetLayout {
		public:
			class Builder {
			public:
				Builder(vk::Device& device) : device(device) {}

				Builder& addBinding(
					uint32_t binding,
					vk::DescriptorType descriptorType,
					vk::ShaderStageFlags stageFlags,
					uint32_t count = 1);
				std::unique_ptr<StarDescriptorSetLayout> build() const; 

			protected:

			private:
				vk::Device& device; 
				std::unordered_map<uint32_t, vk::DescriptorSetLayoutBinding> bindings{};

			};

			StarDescriptorSetLayout(vk::Device& device, std::unordered_map<uint32_t, vk::DescriptorSetLayoutBinding> bindings); 

			~StarDescriptorSetLayout(); 

			vk::DescriptorSetLayout getDescriptorSetLayout() { return this->descriptorSetLayout; }
			
		protected:

		private: 
			vk::Device& device; 
			vk::DescriptorSetLayout descriptorSetLayout; 
			std::unordered_map<uint32_t, vk::DescriptorSetLayoutBinding> bindings; 

			//allow access to the descriptor writer 
			friend class StarDescriptorWriter;
		};

		class StarDescriptorPool {
		public:
			class Builder {
			public:
				Builder(vk::Device& device) : device(device) {};

				Builder& addPoolSize(vk::DescriptorType descriptorType, uint32_t count);
				Builder& setPoolFlags(vk::DescriptorPoolCreateFlags flags); 
				Builder& setMaxSets(uint32_t count); 
				std::unique_ptr<StarDescriptorPool> build() const; 

			protected:
				
			private:
				vk::Device& device; 
				std::vector<vk::DescriptorPoolSize> poolSizes; 
				uint32_t maxSets = 50; 
				vk::DescriptorPoolCreateFlags poolFlags{}; 
			};

			StarDescriptorPool(vk::Device& device, uint32_t maxSets, vk::DescriptorPoolCreateFlags poolFlags, const std::vector<vk::DescriptorPoolSize>& poolSizes);
			~StarDescriptorPool(); 

			bool allocateDescriptorSet(const vk::DescriptorSetLayout descriptorSetLayout, std::vector<vk::DescriptorSet>& descriptor);

			void freeDescriptors(std::vector<vk::DescriptorSet>& descriptors) const;

			void resetPool();

		protected:

		private: 
			vk::Device& device; 
			vk::DescriptorPool descriptorPool; 

			//allow this class to read the private info of StarDescriptorSetLayout for construction 
			friend class StarDescriptorWriter;
		};

		//allow access to the descriptor writer 
		class StarDescriptorWriter {
		public:
			StarDescriptorWriter(vk::Device& device, StarDescriptorSetLayout& setLayout, StarDescriptorPool& pool); 

			StarDescriptorWriter& writeBuffer(uint32_t binding, vk::DescriptorBufferInfo* bufferInfo); 

			StarDescriptorWriter& writeImage(uint32_t binding, vk::DescriptorImageInfo* imageInfo); 

			bool build(std::vector<vk::DescriptorSet>& set, bool callVulkanUpdate = false);

			void overwrite(vk::DescriptorSet& set);

		private:
			vk::Device& device; 
			StarDescriptorSetLayout& setLayout;
			StarDescriptorPool& pool; 
			std::vector<vk::WriteDescriptorSet> writeSets; 

		};
	}
}