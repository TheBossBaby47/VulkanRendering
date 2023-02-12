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
		vk::Device  sourceDevice; //HMMMM
		vk::Buffer	buffer;
		size_t		size;
		void*		mappedData;

		VmaAllocation		allocationHandle;
		VmaAllocationInfo	allocationInfo;
		VmaAllocator		allocator;

		VulkanBuffer() {
			size = 0;
			mappedData = nullptr;
		}

		VulkanBuffer(VulkanBuffer&& obj) {
			buffer = obj.buffer;
			allocationHandle = obj.allocationHandle;
			allocationInfo = obj.allocationInfo;
			allocator = obj.allocator;
			size = obj.size;
			mappedData = obj.mappedData;
			sourceDevice = obj.sourceDevice;

			obj.buffer = VK_NULL_HANDLE;
			obj.mappedData = nullptr;
		}

		VulkanBuffer& operator=(VulkanBuffer&& obj) {
			if (this != &obj) {
				buffer = obj.buffer;
				allocationHandle = obj.allocationHandle;
				allocationInfo = obj.allocationInfo;
				allocator = obj.allocator;
				size = obj.size;
				mappedData = obj.mappedData;
				sourceDevice = obj.sourceDevice;

				obj.buffer = VK_NULL_HANDLE;
				obj.mappedData = nullptr;
			}
			return *this;
		}

		~VulkanBuffer() {
			if (buffer) {
				//Unmap();
				vmaDestroyBuffer(allocator, buffer, allocationHandle);
			}
		}

		//A convenience func to help get around vma holding various
		//mapped pointers etc, so us calling mapBuffer can cause
		//validation errors
		void CopyData(void* data, size_t size);

		void*	Map();
		void	Unmap();
	};
};