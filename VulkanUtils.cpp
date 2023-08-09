/******************************************************************************
This file is part of the Newcastle Vulkan Tutorial Series

Author:Rich Davison
Contact:richgdavison@gmail.com
License: MIT (see LICENSE file at the top of the source tree)
*//////////////////////////////////////////////////////////////////////////////
#include "VulkanUtils.h"
#include "VulkanTexture.h"

using namespace NCL;
using namespace Rendering;

std::map<vk::Device, vk::DescriptorSetLayout > NCL::Rendering::Vulkan::nullDescriptors;

vk::DynamicLoader NCL::Rendering::Vulkan::dynamicLoader;

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

vk::ClearColorValue Vulkan::ClearColour(float r, float g, float b, float a) {
	return vk::ClearColorValue(std::array<float, 4>{r, g, b, a});
}

void Vulkan::SetDebugName(vk::Device device, vk::ObjectType t, uint64_t handle, const std::string& debugName) {
	device.setDebugUtilsObjectNameEXT(
		vk::DebugUtilsObjectNameInfoEXT()
		.setObjectType(t)
		.setObjectHandle(handle)
		.setPObjectName(debugName.c_str())
	);
};

void Vulkan::BeginDebugArea(vk::CommandBuffer b, const std::string& name) {
	vk::DebugUtilsLabelEXT labelInfo;
	labelInfo.pLabelName = name.c_str();

	b.beginDebugUtilsLabelEXT(labelInfo);
}

void Vulkan::EndDebugArea(vk::CommandBuffer b) {
	b.endDebugUtilsLabelEXT();
}

void Vulkan::SetNullDescriptor(vk::Device device, vk::DescriptorSetLayout layout) {
	nullDescriptors.insert({ device, layout });
}

void	Vulkan::ImageTransitionBarrier(vk::CommandBuffer  cmdBuffer, vk::Image image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, vk::ImageAspectFlags aspect, vk::PipelineStageFlags srcStage, vk::PipelineStageFlags dstStage, int mipLevel, int layer) {
	vk::ImageSubresourceRange subRange = vk::ImageSubresourceRange(aspect, mipLevel, 1, layer, 1);

	vk::ImageMemoryBarrier memoryBarrier = vk::ImageMemoryBarrier()
		.setSubresourceRange(subRange)
		.setImage(image)
		.setOldLayout(oldLayout)
		.setNewLayout(newLayout);

	if (newLayout == vk::ImageLayout::eTransferDstOptimal) {
		memoryBarrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
	}
	else if (newLayout == vk::ImageLayout::eTransferSrcOptimal) {
		memoryBarrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;
	}
	else if (newLayout == vk::ImageLayout::eColorAttachmentOptimal) {
		memoryBarrier.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
	}
	else if (newLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal) {
		memoryBarrier.dstAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentWrite;
	}
	else if (newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
		memoryBarrier.dstAccessMask = vk::AccessFlagBits::eShaderRead | vk::AccessFlagBits::eInputAttachmentRead; //added last bit?!?
	}

	cmdBuffer.pipelineBarrier(srcStage, dstStage, vk::DependencyFlags(), 0, nullptr, 0, nullptr, 1, &memoryBarrier);
}

void Vulkan::TransitionPresentToColour(vk::CommandBuffer  buffer, vk::Image t) {
	ImageTransitionBarrier(buffer, t,
		vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal,
		vk::ImageAspectFlagBits::eColor, vk::PipelineStageFlagBits::eColorAttachmentOutput,
		vk::PipelineStageFlagBits::eColorAttachmentOutput);
}

void Vulkan::TransitionColourToPresent(vk::CommandBuffer  buffer, vk::Image t) {
	ImageTransitionBarrier(buffer, t,
		vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::ePresentSrcKHR, vk::ImageAspectFlagBits::eColor,
		vk::PipelineStageFlagBits::eAllCommands, vk::PipelineStageFlagBits::eBottomOfPipe);
}

void Vulkan::TransitionColourToSampler(vk::CommandBuffer  buffer, vk::Image t) {
	ImageTransitionBarrier(buffer, t,
		vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageAspectFlagBits::eColor,
		vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::PipelineStageFlagBits::eFragmentShader);
}

void Vulkan::TransitionSamplerToColour(vk::CommandBuffer  buffer, vk::Image t) {
	ImageTransitionBarrier(buffer, t,
		vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eColorAttachmentOptimal, vk::ImageAspectFlagBits::eColor,
		vk::PipelineStageFlagBits::eFragmentShader, vk::PipelineStageFlagBits::eColorAttachmentOutput);
}

void Vulkan::TransitionDepthToSampler(vk::CommandBuffer  buffer, vk::Image t, bool doStencil) {
	vk::ImageAspectFlags flags = doStencil ? vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil : vk::ImageAspectFlagBits::eDepth;

	ImageTransitionBarrier(buffer, t,
		vk::ImageLayout::eDepthStencilAttachmentOptimal, vk::ImageLayout::eDepthStencilReadOnlyOptimal, flags,
		vk::PipelineStageFlagBits::eEarlyFragmentTests, vk::PipelineStageFlagBits::eFragmentShader);
}

void Vulkan::TransitionSamplerToDepth(vk::CommandBuffer  buffer, vk::Image t, bool doStencil) {
	vk::ImageAspectFlags flags = doStencil ? vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil : vk::ImageAspectFlagBits::eDepth;

	ImageTransitionBarrier(buffer, t,
		vk::ImageLayout::eDepthStencilReadOnlyOptimal, vk::ImageLayout::eDepthStencilAttachmentOptimal, flags,
		vk::PipelineStageFlagBits::eFragmentShader, vk::PipelineStageFlagBits::eEarlyFragmentTests);
}

void Vulkan::DispatchCompute(vk::CommandBuffer  to, unsigned int xCount, unsigned int yCount, unsigned int zCount) {
	to.dispatch(xCount, yCount, zCount);
}

bool Vulkan::MessageAssert(bool condition, const char* msg) {
	if (!condition) {
		std::cerr << msg << "\n";
	}
	return condition;
}

vk::CommandBuffer	Vulkan::BeginCmdBuffer(vk::Device device, vk::CommandPool fromPool, const std::string& debugName) {
	vk::CommandBufferAllocateInfo bufferInfo = vk::CommandBufferAllocateInfo(fromPool, vk::CommandBufferLevel::ePrimary, 1);

	auto buffers = device.allocateCommandBuffers(bufferInfo); //Func returns a vector!

	if (!debugName.empty()) {
		Vulkan::SetDebugName(device, vk::ObjectType::eCommandBuffer, Vulkan::GetVulkanHandle(buffers[0]), debugName);
	}
	vk::CommandBufferBeginInfo beginInfo = vk::CommandBufferBeginInfo();
	buffers[0].begin(beginInfo);
	return buffers[0];
}

void	Vulkan::SubmitCmdBuffer(vk::CommandBuffer  buffer, vk::Queue queue, vk::Fence fence, vk::Semaphore waitSemaphore, vk::Semaphore signalSempahore) {
	if (buffer) {
		buffer.end();
	}
	else {
		std::cout << __FUNCTION__ << " Submitting invalid buffer?\n";
		return;
	}

	vk::SubmitInfo submitInfo = vk::SubmitInfo();
	submitInfo.setCommandBufferCount(1);
	submitInfo.setPCommandBuffers(&buffer);

	vk::PipelineStageFlags waitStage = vk::PipelineStageFlagBits::eTopOfPipe;

	if (waitSemaphore) {
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = &waitSemaphore;
		submitInfo.pWaitDstStageMask = &waitStage;
	}
	if (signalSempahore) {
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &signalSempahore;
	}

	queue.submit(submitInfo, fence);
}

void		Vulkan::SubmitCmdBufferWait(vk::CommandBuffer  buffer, vk::Device device, vk::Queue queue) {
	vk::Fence fence = device.createFence({});

	SubmitCmdBuffer(buffer, queue, fence);

	if (device.waitForFences(1, &fence, true, UINT64_MAX) != vk::Result::eSuccess) {
		std::cout << __FUNCTION__ << " Device queue submission taking too long?\n";
	};

	device.destroyFence(fence);
}