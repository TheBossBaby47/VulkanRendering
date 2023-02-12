/******************************************************************************
This file is part of the Newcastle Vulkan Tutorial Series

Author:Rich Davison
Contact:richgdavison@gmail.com
License: MIT (see LICENSE file at the top of the source tree)
*//////////////////////////////////////////////////////////////////////////////
#pragma once
#include "VulkanBuffers.h"

namespace NCL::Rendering {
	class VulkanBuffer;

	class VulkanBufferBuilder	{
	public:
		VulkanBufferBuilder(size_t byteSize, const std::string& name = "");

		VulkanBufferBuilder& WithBufferUsage(vk::BufferUsageFlags flags);
		VulkanBufferBuilder& WithMemoryProperties(vk::MemoryPropertyFlags flags);
		VulkanBufferBuilder& WithHostVisibility();
		VulkanBufferBuilder& WithDeviceAddresses();
		VulkanBufferBuilder& WithPersistentMapping();

		VulkanBufferBuilder& WithUniqueAllocation();

		~VulkanBufferBuilder() {};

		VulkanBuffer Build(vk::Device device, VmaAllocator allocator);

	protected:
		std::string		debugName;
		VulkanBuffer	outputBuffer;

		VmaAllocationCreateInfo vmaInfo;
		vk::BufferCreateInfo	vkInfo;
	};
}
