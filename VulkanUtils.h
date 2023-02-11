/******************************************************************************
This file is part of the Newcastle Vulkan Tutorial Series

Author:Rich Davison
Contact:richgdavison@gmail.com
License: MIT (see LICENSE file at the top of the source tree)
*//////////////////////////////////////////////////////////////////////////////
#pragma once

namespace NCL::Rendering {
	class VulkanTexture;

	namespace Vulkan {

		template <typename T>
		uint64_t GetVulkanHandle(T const& cppHandle) {
			return uint64_t(static_cast<T::CType>(cppHandle));
		}

		vk::ClearColorValue ClearColour(float r, float g, float b, float a = 1.0f);

		void SetDebugName(vk::Device d, vk::ObjectType t, uint64_t handle, const std::string& debugName);

		void BeginDebugArea(vk::CommandBuffer b, const std::string& name);
		void EndDebugArea(vk::CommandBuffer b);

		void	ImageTransitionBarrier(vk::CommandBuffer  buffer, vk::Image i, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, vk::ImageAspectFlags aspect, vk::PipelineStageFlags srcStage, vk::PipelineStageFlags dstStage, int mipLevel = 0, int layer = 0);

		void TransitionColourToSampler(vk::CommandBuffer  buffer, vk::Image t);
		void TransitionDepthToSampler(vk::CommandBuffer  buffer, vk::Image t, bool doStencil = false);

		void TransitionSamplerToColour(vk::CommandBuffer  buffer, vk::Image t);
		void TransitionSamplerToDepth(vk::CommandBuffer  buffer, vk::Image t, bool doStencil = false);

		void DispatchCompute(vk::CommandBuffer  to, unsigned int xCount, unsigned int yCount = 0, unsigned int zCount = 0);



		extern vk::DispatchLoaderDynamic* dispatcher;
		void SetNullDescriptor(vk::Device device, vk::DescriptorSetLayout layout);
		extern std::map<vk::Device, vk::DescriptorSetLayout > nullDescriptors;
	}
}