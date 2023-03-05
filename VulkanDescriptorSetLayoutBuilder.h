/******************************************************************************
This file is part of the Newcastle Vulkan Tutorial Series

Author:Rich Davison
Contact:richgdavison@gmail.com
License: MIT (see LICENSE file at the top of the source tree)
*//////////////////////////////////////////////////////////////////////////////
#pragma once

namespace NCL::Rendering {
	class VulkanRenderer;

	class VulkanDescriptorSetLayoutBuilder	{
	public:
		VulkanDescriptorSetLayoutBuilder(const std::string& name = "") { 
			debugName = name; 
			usingBindless = false; 
			usingDescriptorBuffer = false;
		};
		~VulkanDescriptorSetLayoutBuilder() {};

		VulkanDescriptorSetLayoutBuilder& WithSamplers(unsigned int count, vk::ShaderStageFlags inShaders = vk::ShaderStageFlagBits::eAll, vk::DescriptorBindingFlags = (vk::DescriptorBindingFlags)0);
		VulkanDescriptorSetLayoutBuilder& WithUniformBuffers(unsigned int count, vk::ShaderStageFlags inShaders = vk::ShaderStageFlagBits::eAll, vk::DescriptorBindingFlags = (vk::DescriptorBindingFlags)0);
		VulkanDescriptorSetLayoutBuilder& WithStorageBuffers(unsigned int count, vk::ShaderStageFlags inShaders = vk::ShaderStageFlagBits::eAll, vk::DescriptorBindingFlags = (vk::DescriptorBindingFlags)0);

		VulkanDescriptorSetLayoutBuilder& WithCreationFlags(vk::DescriptorSetLayoutCreateFlags flags);

		vk::UniqueDescriptorSetLayout Build(vk::Device device);

	protected:
		std::string	debugName;
		bool usingBindless;
		bool usingDescriptorBuffer;
		std::vector< vk::DescriptorSetLayoutBinding>	addedBindings;
		std::vector< vk::DescriptorBindingFlags>		addedFlags;

		vk::DescriptorSetLayoutCreateInfo createInfo;
	};
}