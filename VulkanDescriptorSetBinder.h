/******************************************************************************
This file is part of the Newcastle Vulkan Tutorial Series

Author:Rich Davison
Contact:richgdavison@gmail.com
License: MIT (see LICENSE file at the top of the source tree)
*//////////////////////////////////////////////////////////////////////////////
#pragma once
namespace NCL::Rendering::Vulkan {
	class DescriptorSetBinder {
	public:
		DescriptorSetBinder(vk::CommandBuffer inBuffer, vk::PipelineLayout inLayout, vk::PipelineBindPoint inBindpoint = vk::PipelineBindPoint::eGraphics) {
			buffer = inBuffer;
			layout = inLayout;
			bindPoint = inBindpoint;
		}
		DescriptorSetBinder() {
			buffer = VK_NULL_HANDLE;
			layout = VK_NULL_HANDLE;
			bindPoint = vk::PipelineBindPoint::eGraphics;
		}
		~DescriptorSetBinder() {
		}

		DescriptorSetBinder& Bind(vk::DescriptorSet set, uint32_t slot){
			buffer.bindDescriptorSets(bindPoint, layout, slot, 1, &set, 0, nullptr);
			return *this;
		}

		//void Commit(vk::CommandBuffer buffer, vk::PipelineLayout layout, vk::PipelineBindPoint bindPoint = vk::PipelineBindPoint::eGraphics) {
		//	//buffer.bindDescriptorSets(bindPoint, layout, 0, 4, sets, 0, nullptr);
		//}

	protected:
		vk::CommandBuffer buffer;
		vk::PipelineLayout layout;
		vk::PipelineBindPoint bindPoint;
	};

	class DescriptorSetMultiBinder {
		const static int MAX_SET_ARRAY = 16;
	public:
		DescriptorSetMultiBinder(int inFirstSlot = 0) {
			slotOffset	= inFirstSlot;
			firstSlot	= INT_MAX;
			lastSlot	= 0;
			slotCount	= 0;
		}
		~DescriptorSetMultiBinder() {
		}

		DescriptorSetMultiBinder& Bind(vk::DescriptorSet set, uint32_t slot) {
			assert(slot < slotOffset + MAX_SET_ARRAY);
			sets[slot - slotOffset] = set;
			firstSlot = std::min(slot, firstSlot);
			lastSlot  = std::max(slot, lastSlot);
			slotCount++;
			return *this;
		}

		void Commit(vk::CommandBuffer buffer, vk::PipelineLayout layout, vk::PipelineBindPoint bindPoint = vk::PipelineBindPoint::eGraphics) {
			if (lastSlot - firstSlot == slotCount-1) {
				//It's a contiguous set list!
				buffer.bindDescriptorSets(bindPoint, layout, firstSlot, slotCount, &sets[firstSlot - slotOffset], 0, nullptr);
				return;
			}
			int startingPoint = firstSlot;
			int subCount = 0;
			for (int i = firstSlot; i < MAX_SET_ARRAY; ++i) {
				if (subCount == 0 && !sets[i]) { //Skipping along a hole in the sets
					startingPoint++;
					continue;
				}
				if (sets[i]) {	//We've found a set!
					if (i == MAX_SET_ARRAY - 1) {
						buffer.bindDescriptorSets(bindPoint, layout, startingPoint, subCount, &sets[startingPoint - slotOffset], 0, nullptr);
						return;
					}
					subCount++;
					continue;
				}
				//Dump previously counted sets
				buffer.bindDescriptorSets(bindPoint, layout, startingPoint, subCount, &sets[startingPoint - slotOffset], 0, nullptr);
				startingPoint = i;
				subCount = 0;
			}
		}

	protected:
		vk::DescriptorSet sets[MAX_SET_ARRAY];
		uint32_t slotOffset;
		uint32_t lastSlot;
		uint32_t firstSlot;
		uint32_t slotCount;
	};
}