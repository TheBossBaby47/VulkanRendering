/******************************************************************************
This file is part of the Newcastle Vulkan Tutorial Series

Author:Rich Davison
Contact:richgdavison@gmail.com
License: MIT (see LICENSE file at the top of the source tree)
*//////////////////////////////////////////////////////////////////////////////
#pragma once
#include "VulkanPipeline.h"
#include "VulkanPipelineBuilderBase.h"
#include "SmartTypes.h"

namespace NCL::Rendering {
	class VulkanCompute;

	class VulkanComputePipelineBuilder : public VulkanPipelineBuilderBase<VulkanComputePipelineBuilder, vk::ComputePipelineCreateInfo>	{
	public:
		VulkanComputePipelineBuilder(const std::string& name = "");
		~VulkanComputePipelineBuilder() {}

		//VulkanComputePipelineBuilder& WithLayout(vk::PipelineLayout layout);

		//VulkanComputePipelineBuilder& WithPushConstant(vk::ShaderStageFlags flags, uint32_t offset, uint32_t size);
		//VulkanComputePipelineBuilder& WithDescriptorSetLayout(uint32_t slot, vk::DescriptorSetLayout layout);
		VulkanComputePipelineBuilder& WithShader(UniqueVulkanCompute& shader);

		VulkanPipeline	Build(vk::Device device, vk::PipelineCache cache = {});

	protected:
	};
};