/******************************************************************************
This file is part of the Newcastle Vulkan Tutorial Series

Author:Rich Davison
Contact:richgdavison@gmail.com
License: MIT (see LICENSE file at the top of the source tree)
*//////////////////////////////////////////////////////////////////////////////
#include "VulkanBufferBuilder.h"
#include "VulkanBuffers.h"
#include "VulkanUtils.h"

using namespace NCL;
using namespace Rendering;

VulkanBufferBuilder::VulkanBufferBuilder(size_t byteSize, const std::string& name) {
	outputBuffer.size	= byteSize;
	vkInfo.size			= byteSize;
	debugName			= name;
	vmaInfo = {};
	vmaInfo.usage		= VMA_MEMORY_USAGE_AUTO;
}

VulkanBufferBuilder& VulkanBufferBuilder::WithBufferUsage(vk::BufferUsageFlags flags) {
	vkInfo.usage |= flags;
	return *this;
}

VulkanBufferBuilder& VulkanBufferBuilder::WithMemoryProperties(vk::MemoryPropertyFlags flags) {
	vmaInfo.requiredFlags |= (VkMemoryPropertyFlags)flags;
	return *this;
}

VulkanBufferBuilder& VulkanBufferBuilder::WithHostVisibility() {
	vmaInfo.requiredFlags |= (VkMemoryPropertyFlags)vk::MemoryPropertyFlagBits::eHostVisible;
	vmaInfo.flags |= VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;

	return *this;
}

VulkanBufferBuilder& VulkanBufferBuilder::WithDeviceAddresses() {
	vkInfo.usage |= vk::BufferUsageFlagBits::eShaderDeviceAddress;
	return *this;
}

VulkanBufferBuilder& VulkanBufferBuilder::WithPersistentMapping() {
	vmaInfo.requiredFlags |= (VkMemoryPropertyFlags)vk::MemoryPropertyFlagBits::eHostCoherent;

	vmaInfo.flags |= (VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT);
	return *this;
}

VulkanBufferBuilder& VulkanBufferBuilder::WithUniqueAllocation() {
	vmaInfo.flags |= VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
	return *this;
}

VulkanBuffer VulkanBufferBuilder::Build(vk::Device device, VmaAllocator allocator) {
	outputBuffer.allocator = allocator;

	vmaCreateBuffer(allocator, (VkBufferCreateInfo*)&vkInfo, &vmaInfo, (VkBuffer*)&(outputBuffer.buffer), &outputBuffer.allocationHandle, &outputBuffer.allocationInfo);

	if (!debugName.empty()) {
		Vulkan::SetDebugName(device, vk::ObjectType::eBuffer, Vulkan::GetVulkanHandle(outputBuffer.buffer), debugName);
	}

	return std::move(outputBuffer);
}