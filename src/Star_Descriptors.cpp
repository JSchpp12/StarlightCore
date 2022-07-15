#include "Star_Descriptors.hpp"
#include "Star_Descriptors.hpp"
#include "Star_Descriptors.hpp"

namespace star::core{
	StarDescriptorSetLayout::Builder& StarDescriptorSetLayout::Builder::addBinding(uint32_t binding,
			vk::DescriptorType descriptorType, 
			vk::ShaderStageFlags stageFlags, uint32_t count) {
		assert(bindings.count(binding) == 0 && "Binding already in use"); 
		vk::DescriptorSetLayoutBinding layoutBinding{}; 
		layoutBinding.binding = binding; 
		layoutBinding.descriptorType = descriptorType; 
		layoutBinding.descriptorCount = count; 
		layoutBinding.stageFlags = stageFlags; 

		this->bindings[binding] = layoutBinding; 
		return *this; 
	}

	std::unique_ptr<StarDescriptorSetLayout> StarDescriptorSetLayout::Builder::build() const {
		return std::make_unique<StarDescriptorSetLayout>(this->starDevice, this->bindings); 
	}

	StarDescriptorSetLayout::StarDescriptorSetLayout(StarDevice& device, std::unordered_map<uint32_t, vk::DescriptorSetLayoutBinding> bindings) :
			starDevice(device),
			bindings{ bindings } {
		std::vector<vk::DescriptorSetLayoutBinding> setLayoutBindings; 

		for (auto& binding : bindings) {
			setLayoutBindings.push_back(binding.second); 
		}

		vk::DescriptorSetLayoutCreateInfo createInfo{}; 
		createInfo.sType = vk::StructureType::eDescriptorSetLayoutCreateInfo; 
		createInfo.bindingCount = static_cast<uint32_t>(setLayoutBindings.size()); 
		createInfo.pBindings = setLayoutBindings.data(); 

		this->descriptorSetLayout = this->starDevice.getDevice().createDescriptorSetLayout(createInfo);
		if (!this->descriptorSetLayout) {
			throw std::runtime_error("failed to create descriptor set layout"); 
		}
	}

	StarDescriptorSetLayout::~StarDescriptorSetLayout() {
		this->starDevice.getDevice().destroyDescriptorSetLayout(this->descriptorSetLayout);
	}


	/* Descriptor Pool */
	StarDescriptorPool::Builder& StarDescriptorPool::Builder::addPoolSize(vk::DescriptorType descriptorType, uint32_t count) {
		this->poolSizes.push_back({ descriptorType, count }); 
		return *this; 
	}

	StarDescriptorPool::Builder& StarDescriptorPool::Builder::setPoolFlags(vk::DescriptorPoolCreateFlags flags)
	{
		this->poolFlags = flags;
		return *this;
	}

	StarDescriptorPool::Builder& StarDescriptorPool::Builder::setMaxSets(uint32_t count)
	{
		this->maxSets = count; 
		return *this; 
	}

	std::unique_ptr<StarDescriptorPool> StarDescriptorPool::Builder::build() const {
		return std::make_unique<StarDescriptorPool>(this->starDevice, this->maxSets, this->poolFlags, this->poolSizes); 
	}

	StarDescriptorPool::StarDescriptorPool(StarDevice& device, 
		uint32_t maxSets, 
		vk::DescriptorPoolCreateFlags poolFlags, 
		const std::vector<vk::DescriptorPoolSize>& poolSizes) :
		starDevice(device)
	{
		vk::DescriptorPoolCreateInfo createInfo{}; 
		createInfo.sType = vk::StructureType::eDescriptorPoolCreateInfo; 
		createInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size()); 
		createInfo.pPoolSizes = poolSizes.data(); 
		createInfo.maxSets = maxSets; 
		createInfo.flags = poolFlags; 

		this->descriptorPool = this->starDevice.getDevice().createDescriptorPool(createInfo);
		if (!this->descriptorPool) {
			throw std::runtime_error("Unable to create descriptor pool"); 
		}
	}

	StarDescriptorPool::~StarDescriptorPool() {
		this->starDevice.getDevice().destroyDescriptorPool(this->descriptorPool); 
	}

	vk::DescriptorPool StarDescriptorPool::getDescriptorPool() {
		return this->descriptorPool; 
	}

	bool StarDescriptorPool::allocateDescriptorSet(const vk::DescriptorSetLayout descriptorSetLayout, vk::DescriptorSet& descriptorSet) {
		vk::DescriptorSetAllocateInfo allocInfo{}; 
		allocInfo.sType = vk::StructureType::eDescriptorSetAllocateInfo; 
		allocInfo.descriptorPool = this->descriptorPool; 
		allocInfo.pSetLayouts = &descriptorSetLayout;
		allocInfo.descriptorSetCount = 1;

		if (this->starDevice.getDevice().allocateDescriptorSets(&allocInfo, &descriptorSet) != vk::Result::eSuccess) {
			return false; 
		}

		return true; 
	}

	void StarDescriptorPool::freeDescriptors(std::vector<vk::DescriptorSet>& descriptors) const {
		this->starDevice.getDevice().freeDescriptorSets(this->descriptorPool, descriptors);
	}

	void StarDescriptorPool::resetPool() {
		this->starDevice.getDevice().resetDescriptorPool(this->descriptorPool);
	}

	/* Descriptor Writer */

	StarDescriptorWriter::StarDescriptorWriter(StarDevice& device, StarDescriptorSetLayout& setLayout, StarDescriptorPool& pool) :
		starDevice(device),
		setLayout{setLayout}, 
		pool{ pool } {}

	StarDescriptorWriter& StarDescriptorWriter::writeBuffer(uint32_t binding, vk::DescriptorBufferInfo* bufferInfos) {
		assert(this->setLayout.bindings.count(binding) == 1 && "Layout does not contain binding specified"); 

		auto& bindingDescription = this->setLayout.bindings[binding]; 
		vk::WriteDescriptorSet writeSet{}; 
		writeSet.sType = vk::StructureType::eWriteDescriptorSet; 
		writeSet.descriptorType = bindingDescription.descriptorType; 
		writeSet.dstBinding = binding; 
		writeSet.pBufferInfo = bufferInfos;
		//writeSet.descriptorCount = static_cast<uint32_t>(bufferInfos->size());
		writeSet.descriptorCount = 1;

		this->writeSets.push_back(writeSet); 
		return *this;
	}

	StarDescriptorWriter& StarDescriptorWriter::writeImage(uint32_t binding, vk::DescriptorImageInfo* imageInfo) {
		assert(this->setLayout.bindings.count(binding) == 1 && "Layout does not contain specified binding");

		auto& bindingDescription = setLayout.bindings[binding]; 
		assert(bindingDescription.descriptorCount == 1 && "Binding single descriptor info, but binding expects multiple"); 

		vk::WriteDescriptorSet writeSet{}; 
		writeSet.sType = vk::StructureType::eWriteDescriptorSet; 
		writeSet.descriptorType = bindingDescription.descriptorType; 
		writeSet.dstBinding = binding; 
		writeSet.pImageInfo = imageInfo; 
		writeSet.descriptorCount = 1; 

		this->writeSets.push_back(writeSet); 
		return *this;
	}

	bool StarDescriptorWriter::build(vk::DescriptorSet& set) {
		bool success = this->pool.allocateDescriptorSet(setLayout.getDescriptorSetLayout(), set);
		if (!success) {
			return false; 
		}
		overwrite(set);
		return success; 
	}

	void StarDescriptorWriter::overwrite(vk::DescriptorSet& set) {
		for (auto& write : this->writeSets) {
			write.dstSet = set;
		}
		this->starDevice.getDevice().updateDescriptorSets(this->writeSets, nullptr);
	}
}