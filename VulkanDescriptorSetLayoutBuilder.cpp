/******************************************************************************
This file is part of the Newcastle Vulkan Tutorial Series

Author:Rich Davison
Contact:richgdavison@gmail.com
License: MIT (see LICENSE file at the top of the source tree)
*//////////////////////////////////////////////////////////////////////////////
#include "VulkanDescriptorSetLayoutBuilder.h"
#include "VulkanMesh.h"
#include "VulkanShader.h"
#include "VulkanRenderer.h"
#include "VulkanUtils.h"

using namespace NCL;
using namespace Rendering;

VulkanDescriptorSetLayoutBuilder& VulkanDescriptorSetLayoutBuilder::WithSamplers(unsigned int count, vk::ShaderStageFlags inShaders, vk::DescriptorBindingFlags bindingFlags) {
	vk::DescriptorSetLayoutBinding binding = vk::DescriptorSetLayoutBinding()
		.setBinding((uint32_t)addedBindings.size())
		.setDescriptorCount(count)
		.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
		.setStageFlags(inShaders);

	addedBindings.emplace_back(binding);
	addedFlags.emplace_back(bindingFlags);

	return *this;
}

VulkanDescriptorSetLayoutBuilder& VulkanDescriptorSetLayoutBuilder::WithUniformBuffers(unsigned int count, vk::ShaderStageFlags inShaders, vk::DescriptorBindingFlags bindingFlags) {
	vk::DescriptorSetLayoutBinding binding = vk::DescriptorSetLayoutBinding()
		.setBinding((uint32_t)addedBindings.size())
		.setDescriptorCount(count)
		.setDescriptorType(vk::DescriptorType::eUniformBuffer)
		.setStageFlags(inShaders);

	addedBindings.emplace_back(binding);
	addedFlags.emplace_back(bindingFlags);

	return *this;
}

VulkanDescriptorSetLayoutBuilder& VulkanDescriptorSetLayoutBuilder::WithStorageBuffers(unsigned int count, vk::ShaderStageFlags inShaders, vk::DescriptorBindingFlags bindingFlags) {
	vk::DescriptorSetLayoutBinding binding = vk::DescriptorSetLayoutBinding()
		.setBinding((uint32_t)addedBindings.size())
		.setDescriptorCount(count)
		.setDescriptorType(vk::DescriptorType::eStorageBuffer)
		.setStageFlags(inShaders);

	addedBindings.emplace_back(binding);
	addedFlags.emplace_back(bindingFlags);

	return *this;
}

VulkanDescriptorSetLayoutBuilder& VulkanDescriptorSetLayoutBuilder::WithCreationFlags(vk::DescriptorSetLayoutCreateFlags flags) {
	createInfo.flags |= flags;
	return *this;
}

vk::UniqueDescriptorSetLayout VulkanDescriptorSetLayoutBuilder::Build(vk::Device device) {
	createInfo.setBindings(addedBindings);
	vk::DescriptorSetLayoutBindingFlagsCreateInfoEXT bindingFlagsInfo;
	
	bindingFlagsInfo.setBindingFlags(addedFlags);

	createInfo.pNext = &bindingFlagsInfo;
	vk::UniqueDescriptorSetLayout layout = std::move(device.createDescriptorSetLayoutUnique(createInfo));
	if (!debugName.empty()) {
		Vulkan::SetDebugName(device, vk::ObjectType::eDescriptorSetLayout, Vulkan::GetVulkanHandle(*layout), debugName);
	}
	return layout;
}