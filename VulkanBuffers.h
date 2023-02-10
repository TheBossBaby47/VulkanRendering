/******************************************************************************
This file is part of the Newcastle Vulkan Tutorial Series

Author:Rich Davison
Contact:richgdavison@gmail.com
License: MIT (see LICENSE file at the top of the source tree)
*//////////////////////////////////////////////////////////////////////////////
#pragma once
#include "vma/vk_mem_alloc.h"

namespace NCL::Rendering {
	//A buffer, backed by memory we have allocated elsewhere
	struct VulkanBuffer {
		vk::Buffer	buffer;

		VmaAllocation		allocationHandle;
		VmaAllocationInfo	allocationInfo;
		VmaAllocator		allocator;

		VulkanBuffer() {

		}

		VulkanBuffer(VulkanBuffer&& obj) {
			buffer = obj.buffer;
			allocationHandle = obj.allocationHandle;
			allocationInfo = obj.allocationInfo;
			allocator = obj.allocator;

			obj.buffer = VK_NULL_HANDLE;
		}

		VulkanBuffer& operator=(VulkanBuffer&& obj) {
			if (this != &obj) {
				buffer = obj.buffer;
				allocationHandle = obj.allocationHandle;
				allocationInfo = obj.allocationInfo;
				allocator = obj.allocator;

				obj.buffer = VK_NULL_HANDLE;
			}
			return *this;
		}

		~VulkanBuffer() {
			if (buffer) {
				vmaDestroyBuffer(allocator, buffer, allocationHandle);
			}
		}
	};

	template<typename T>
	struct UniformBuffer : VulkanBuffer
	{
		T* mappedMemory;
	};
};