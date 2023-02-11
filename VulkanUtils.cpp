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

vk::DispatchLoaderDynamic* NCL::Rendering::Vulkan::dispatcher = nullptr;
std::map<vk::Device, vk::DescriptorSetLayout > NCL::Rendering::Vulkan::nullDescriptors;

vk::ClearColorValue Vulkan::ClearColour(float r, float g, float b, float a) {
	return vk::ClearColorValue(std::array<float, 4>{r, g, b, a});
}

void Vulkan::SetDebugName(vk::Device device, vk::ObjectType t, uint64_t handle, const std::string& debugName) {
	device.setDebugUtilsObjectNameEXT(
		vk::DebugUtilsObjectNameInfoEXT()
		.setObjectType(t)
		.setObjectHandle(handle)
		.setPObjectName(debugName.c_str()), *Vulkan::dispatcher
	);
};

void Vulkan::BeginDebugArea(vk::CommandBuffer b, const std::string& name) {
	vk::DebugUtilsLabelEXT labelInfo;
	labelInfo.pLabelName = name.c_str();

	b.beginDebugUtilsLabelEXT(labelInfo, *Vulkan::dispatcher);
}

void Vulkan::EndDebugArea(vk::CommandBuffer b) {
	b.endDebugUtilsLabelEXT(*Vulkan::dispatcher);
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