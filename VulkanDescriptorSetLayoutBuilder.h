/******************************************************************************
This file is part of the Newcastle Vulkan Tutorial Series

Author:Rich Davison
Contact:richgdavison@gmail.com
License: MIT (see LICENSE file at the top of the source tree)
*//////////////////////////////////////////////////////////////////////////////
#pragma once

namespace NCL::Rendering::Vulkan {
	class VulkanRenderer;

	class DescriptorSetLayoutBuilder {
	public:
		DescriptorSetLayoutBuilder(vk::Device device) {
			sourceDevice = device;
		};
		~DescriptorSetLayoutBuilder() {};


		DescriptorSetLayoutBuilder& WithSamplers(uint32_t index, unsigned int count, vk::ShaderStageFlags inShaders = vk::ShaderStageFlagBits::eAll, vk::DescriptorBindingFlags = (vk::DescriptorBindingFlags)0);

		DescriptorSetLayoutBuilder& WithUniformTexelBuffers(uint32_t index, unsigned int count, vk::ShaderStageFlags inShaders = vk::ShaderStageFlagBits::eAll, vk::DescriptorBindingFlags = (vk::DescriptorBindingFlags)0);
		DescriptorSetLayoutBuilder& WithStorageTexelBuffers(uint32_t index, unsigned int count, vk::ShaderStageFlags inShaders = vk::ShaderStageFlagBits::eAll, vk::DescriptorBindingFlags = (vk::DescriptorBindingFlags)0);


		DescriptorSetLayoutBuilder& WithImageSamplers(uint32_t index, unsigned int count, vk::ShaderStageFlags inShaders = vk::ShaderStageFlagBits::eAll, vk::DescriptorBindingFlags = (vk::DescriptorBindingFlags)0);
		
		DescriptorSetLayoutBuilder& WithSampledImages(uint32_t index, unsigned int count, vk::ShaderStageFlags inShaders = vk::ShaderStageFlagBits::eAll, vk::DescriptorBindingFlags = (vk::DescriptorBindingFlags)0);
		DescriptorSetLayoutBuilder& WithStorageImages(uint32_t index, unsigned int count, vk::ShaderStageFlags inShaders = vk::ShaderStageFlagBits::eAll, vk::DescriptorBindingFlags = (vk::DescriptorBindingFlags)0);

		DescriptorSetLayoutBuilder& WithUniformBuffers(uint32_t index, unsigned int count, vk::ShaderStageFlags inShaders = vk::ShaderStageFlagBits::eAll, vk::DescriptorBindingFlags = (vk::DescriptorBindingFlags)0);
		DescriptorSetLayoutBuilder& WithStorageBuffers(uint32_t index, unsigned int count, vk::ShaderStageFlags inShaders = vk::ShaderStageFlagBits::eAll, vk::DescriptorBindingFlags = (vk::DescriptorBindingFlags)0);

		DescriptorSetLayoutBuilder& WithDynamicUniformBuffers(uint32_t index, unsigned int count, vk::ShaderStageFlags inShaders = vk::ShaderStageFlagBits::eAll, vk::DescriptorBindingFlags = (vk::DescriptorBindingFlags)0);
		DescriptorSetLayoutBuilder& WithDynamicStorageBuffers(uint32_t index, unsigned int count, vk::ShaderStageFlags inShaders = vk::ShaderStageFlagBits::eAll, vk::DescriptorBindingFlags = (vk::DescriptorBindingFlags)0);

		DescriptorSetLayoutBuilder& WithAccelStructures(uint32_t index, unsigned int count, vk::ShaderStageFlags inShaders = vk::ShaderStageFlagBits::eAll, vk::DescriptorBindingFlags = (vk::DescriptorBindingFlags)0);

		DescriptorSetLayoutBuilder& WithDescriptors(uint32_t index, vk::DescriptorType type, unsigned int count, vk::ShaderStageFlags inShaders = vk::ShaderStageFlagBits::eAll, vk::DescriptorBindingFlags = (vk::DescriptorBindingFlags)0);

		DescriptorSetLayoutBuilder& WithCreationFlags(vk::DescriptorSetLayoutCreateFlags flags);

		vk::UniqueDescriptorSetLayout Build(const std::string& debugName = "");

	protected:
		vk::DescriptorSetLayoutBinding& AddDescriptors(uint32_t index, unsigned int count, vk::ShaderStageFlags inShaders = vk::ShaderStageFlagBits::eAll, vk::DescriptorBindingFlags = (vk::DescriptorBindingFlags)0);

		vk::Device sourceDevice;

		std::vector< vk::DescriptorSetLayoutBinding>	addedBindings;
		std::vector< vk::DescriptorBindingFlags>		addedFlags;

		vk::DescriptorSetLayoutCreateInfo createInfo;
	};
}