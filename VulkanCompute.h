/******************************************************************************
This file is part of the Newcastle Vulkan Tutorial Series

Author:Rich Davison
Contact:richgdavison@gmail.com
License: MIT (see LICENSE file at the top of the source tree)
*//////////////////////////////////////////////////////////////////////////////
#pragma once

namespace NCL::Rendering {
	class VulkanCompute	{
	public:
		VulkanCompute(vk::Device sourceDevice, const std::string& filename);
		~VulkanCompute() {}

		uint32_t GetThreadXCount() const;
		uint32_t GetThreadYCount() const;
		uint32_t GetThreadZCount() const;

		void	FillShaderStageCreateInfo(vk::ComputePipelineCreateInfo& info) const;

	protected:
		uint32_t localThreadSize[3];
		vk::PipelineShaderStageCreateInfo info;
		vk::UniqueShaderModule	computeModule;
	};
}