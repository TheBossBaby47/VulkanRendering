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

		void BeginDebugArea(vk::CommandBuffer buffer, const std::string& name);
		void EndDebugArea(vk::CommandBuffer buffer);

		void ImageTransitionBarrier(vk::CommandBuffer  buffer, vk::Image i, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, vk::ImageAspectFlags aspect, vk::PipelineStageFlags srcStage, vk::PipelineStageFlags dstStage, int mipLevel = 0, int layer = 0);

		void TransitionPresentToColour(vk::CommandBuffer  buffer, vk::Image t);
		void TransitionColourToPresent(vk::CommandBuffer  buffer, vk::Image t);

		void TransitionColourToSampler(vk::CommandBuffer  buffer, vk::Image t);
		void TransitionDepthToSampler(vk::CommandBuffer  buffer, vk::Image t, bool doStencil = false);

		void TransitionSamplerToColour(vk::CommandBuffer  buffer, vk::Image t);
		void TransitionSamplerToDepth(vk::CommandBuffer  buffer, vk::Image t, bool doStencil = false);

		void DispatchCompute(vk::CommandBuffer  buffer, unsigned int xCount, unsigned int yCount = 1, unsigned int zCount = 1);

		bool MessageAssert(bool condition, const char* msg);

		vk::CommandBuffer	BeginCmdBuffer(vk::Device device, vk::CommandPool fromPool, const std::string& debugName = "");
		void	SubmitCmdBuffer(vk::CommandBuffer  buffer, vk::Queue queue, vk::Fence fence = {}, vk::Semaphore waitSemaphore = {}, vk::Semaphore signalSempahore = {});
		void	SubmitCmdBufferWait(vk::CommandBuffer  buffer, vk::Device device, vk::Queue queue);

		extern vk::DynamicLoader dynamicLoader;
		//extern vk::DispatchLoaderDynamic*	dispatcher;
		//extern vk::DispatchLoaderStatic*	staticDispatcher;
		void SetNullDescriptor(vk::Device device, vk::DescriptorSetLayout layout);
		extern std::map<vk::Device, vk::DescriptorSetLayout > nullDescriptors;
	}
}