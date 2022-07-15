//This file contains wrapper classes which pertain to vulkan descriptor use and creation

#pragma once 
#include "Star_Device.hpp"

#include <vulkan/vulkan.hpp>

#include <memory>
#include <unordered_map>
namespace star::core{
	class StarDescriptorSetLayout {
	public:
		class Builder {
		public:
			Builder(StarDevice& device) : starDevice(device) {}

			Builder& addBinding(
				uint32_t binding,
				vk::DescriptorType descriptorType,
				vk::ShaderStageFlags stageFlags,
				uint32_t count = 1);
			std::unique_ptr<StarDescriptorSetLayout> build() const; 

		protected:

		private:
			StarDevice& starDevice; 
			std::unordered_map<uint32_t, vk::DescriptorSetLayoutBinding> bindings{};

		};

		StarDescriptorSetLayout(StarDevice& device, std::unordered_map<uint32_t, vk::DescriptorSetLayoutBinding> bindings); 

		~StarDescriptorSetLayout(); 

		vk::DescriptorSetLayout getDescriptorSetLayout() { return this->descriptorSetLayout; }
			
	protected:

	private: 
		StarDevice& starDevice; 
		vk::DescriptorSetLayout descriptorSetLayout; 
		std::unordered_map<uint32_t, vk::DescriptorSetLayoutBinding> bindings; 

		//allow access to the descriptor writer 
		friend class StarDescriptorWriter;
	};

	class StarDescriptorPool {
	public:
		class Builder {
		public:
			Builder(StarDevice& device) : starDevice(device) {};

			Builder& addPoolSize(vk::DescriptorType descriptorType, uint32_t count);
			Builder& setPoolFlags(vk::DescriptorPoolCreateFlags flags); 
			Builder& setMaxSets(uint32_t count); 
			std::unique_ptr<StarDescriptorPool> build() const; 

		protected:
				
		private:
			StarDevice& starDevice; 
			std::vector<vk::DescriptorPoolSize> poolSizes; 
			uint32_t maxSets = 50; 
			vk::DescriptorPoolCreateFlags poolFlags{}; 
		};

		StarDescriptorPool(StarDevice& device, uint32_t maxSets, vk::DescriptorPoolCreateFlags poolFlags, const std::vector<vk::DescriptorPoolSize>& poolSizes);
		~StarDescriptorPool(); 

		vk::DescriptorPool getDescriptorPool(); 

		bool allocateDescriptorSet(const vk::DescriptorSetLayout descriptorSetLayout, vk::DescriptorSet& descriptorSets);

		void freeDescriptors(std::vector<vk::DescriptorSet>& descriptors) const;

		void resetPool();

	protected:

	private: 
		StarDevice& starDevice; 
		vk::DescriptorPool descriptorPool; 

		//allow this class to read the private info of StarDescriptorSetLayout for construction 
		friend class StarDescriptorWriter;
	};

	//allow access to the descriptor writer 
	class StarDescriptorWriter {
	public:
		StarDescriptorWriter(StarDevice& device, StarDescriptorSetLayout& setLayout, StarDescriptorPool& pool); 

		StarDescriptorWriter& writeBuffer(uint32_t binding, vk::DescriptorBufferInfo* bufferInfos); 

		StarDescriptorWriter& writeImage(uint32_t binding, vk::DescriptorImageInfo* imageInfo); 

		bool build(vk::DescriptorSet& set);

		void overwrite(vk::DescriptorSet& set);

	private:
		StarDevice& starDevice; 
		StarDescriptorSetLayout& setLayout;
		StarDescriptorPool& pool; 
		std::vector<vk::WriteDescriptorSet> writeSets; 

	};
}