/******************************************************************************
This file is part of the Newcastle Vulkan Tutorial Series

Author:Rich Davison
Contact:richgdavison@gmail.com
License: MIT (see LICENSE file at the top of the source tree)
*//////////////////////////////////////////////////////////////////////////////
#pragma once
#include "VulkanPipeline.h"

namespace NCL::Rendering {
	class VulkanRenderer;
	class VulkanShader;

	struct VulkanVertexSpecification;

	template <class T, class P>
	class VulkanPipelineBuilderBase	{
	public:

		T& WithLayout(vk::PipelineLayout pipeLayout) {
			layout = pipeLayout;
			pipelineCreate.setLayout(pipeLayout);
			return (T&)*this;
		}

		T& WithPushConstant(vk::ShaderStageFlags flags, uint32_t offset, uint32_t size) {
			allPushConstants.emplace_back(vk::PushConstantRange(flags, offset, size));
			return (T&)*this;
		}

		T& WithDescriptorSetLayout(uint32_t slot, vk::DescriptorSetLayout layout) {
			assert(slot < 32);
			while (allLayouts.size() <= slot) {
				allLayouts.push_back(vk::DescriptorSetLayout());
			}
			allLayouts[slot] = layout;
			return (T&)*this;
		}

		T& WithDescriptorBuffers() {
			pipelineCreate.flags |= vk::PipelineCreateFlagBits::eDescriptorBufferEXT;
			return (T&)*this;
		}

		P& GetCreateInfo() {
			return pipelineCreate;
		}
	protected:
		VulkanPipelineBuilderBase(const std::string& pipeName = "") : debugName(pipeName) {

		}
		~VulkanPipelineBuilderBase() {}

	protected:
		P pipelineCreate;
		vk::PipelineLayout layout;

		std::vector< vk::DescriptorSetLayout> allLayouts;
		std::vector< vk::PushConstantRange> allPushConstants;

		std::string debugName;
	};
}