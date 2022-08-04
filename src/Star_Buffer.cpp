#include "Star_Buffer.hpp"

namespace star::core {
	vk::DeviceSize StarBuffer::getAlignment(vk::DeviceSize instanceSize, vk::DeviceSize minOffsetAlignment) {
		if (minOffsetAlignment > 0) {
			return  (instanceSize + minOffsetAlignment - 1) & ~(minOffsetAlignment - 1); 
		}
		return instanceSize; 
	}

	StarBuffer::StarBuffer(StarDevice& device, vk::DeviceSize instanceSize, uint32_t instanceCount,
		vk::BufferUsageFlags flags, vk::MemoryPropertyFlags memoryPropertyFlags,
		vk::DeviceSize minOffsetAlignment) : 
			starDevice(device), 
			instanceSize{instanceSize}, 
			instanceCount{instanceCount}, 
			usageFlags{ flags },
			memoryPropertyFlags{memoryPropertyFlags} {
		this->alignmentSize = getAlignment(this->instanceSize, minOffsetAlignment); 
		this->bufferSize = this->alignmentSize * instanceCount; 
		this->starDevice.createBuffer(this->bufferSize, this->usageFlags, this->memoryPropertyFlags, this->buffer, this->memory); 
	}

	StarBuffer::~StarBuffer() {
		unmap(); 
		this->starDevice.getDevice().destroyBuffer(this->buffer); 
		this->starDevice.getDevice().freeMemory(this->memory); 
	}

	void StarBuffer::map(vk::DeviceSize size, vk::DeviceSize offset) {
		assert(this->buffer && this->memory && "Called map on buffer before creation"); 
		this->mapped = this->starDevice.getDevice().mapMemory(this->memory, offset, size, {});
	}

	void StarBuffer::unmap() {
		if (mapped) {
			this->starDevice.getDevice().unmapMemory(this->memory); 
			this->mapped = nullptr; 
		}
	}

	void StarBuffer::writeToBuffer(void* data, vk::DeviceSize size, vk::DeviceSize offset) {
		assert(this->mapped && "Cannot copy to unmapped buffer"); 

		if (size == VK_WHOLE_SIZE) {
			memcpy(this->mapped, data, this->bufferSize); 
		}
		else {
			char* memOffset = (char*)mapped; 
			memOffset += offset; 
			memcpy(memOffset, data, size); 
		}
	}

	vk::Result StarBuffer::flush(vk::DeviceSize size, vk::DeviceSize offset) {
		vk::MappedMemoryRange mappedRange{}; 
		mappedRange.sType = vk::StructureType::eMappedMemoryRange; 
		mappedRange.memory = this->memory; 
		mappedRange.offset = offset; 
		mappedRange.size = size; 
		return this->starDevice.getDevice().flushMappedMemoryRanges(1, &mappedRange); 
	}

	vk::Result StarBuffer::invalidate(vk::DeviceSize size, vk::DeviceSize offset) {
		vk::MappedMemoryRange mappedRange{}; 
		mappedRange.sType = vk::StructureType::eMappedMemoryRange; 
		mappedRange.memory = this->memory; 
		mappedRange.offset = offset; 
		mappedRange.size = size; 
		return this->starDevice.getDevice().invalidateMappedMemoryRanges(1, &mappedRange); 
	}

	vk::DescriptorBufferInfo StarBuffer::descriptorInfo(vk::DeviceSize size, vk::DeviceSize offset) {
		return vk::DescriptorBufferInfo{
			buffer,
			offset,
			size
		}; 
	}

	void StarBuffer::writeToIndex(void* data, int index) {
		writeToBuffer(data, this->instanceSize, index * this->alignmentSize); 
	}

	vk::Result StarBuffer::flushIndex(int index) {
		return flush(this->alignmentSize, index * this->alignmentSize); 
	}

	vk::DescriptorBufferInfo StarBuffer::descriptorInfoForIndex(int index) {
		return descriptorInfo(this->alignmentSize, index * alignmentSize); 
	}

	vk::Result StarBuffer::invalidateIndex(int index) {
		return invalidate(this->alignmentSize, index * this->alignmentSize); 
	}
}