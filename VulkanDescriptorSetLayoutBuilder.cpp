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

vk::DescriptorSetLayoutBinding& VulkanDescriptorSetLayoutBuilder::AddDescriptors(unsigned int count, vk::ShaderStageFlags inShaders, vk::DescriptorBindingFlags bindingFlags) {
	vk::DescriptorSetLayoutBinding binding = vk::DescriptorSetLayoutBinding()
		.setBinding((uint32_t)addedBindings.size())
		.setDescriptorCount(count)
		.setStageFlags(inShaders);

	addedBindings.emplace_back(binding);
	addedFlags.emplace_back(bindingFlags);
	return addedBindings[addedBindings.size() - 1];
}

VulkanDescriptorSetLayoutBuilder& VulkanDescriptorSetLayoutBuilder::WithSamplers(unsigned int count, vk::ShaderStageFlags inShaders, vk::DescriptorBindingFlags bindingFlags) {
	AddDescriptors(count, inShaders, bindingFlags).setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
	return *this;
}

VulkanDescriptorSetLayoutBuilder& VulkanDescriptorSetLayoutBuilder::WithSampledImages(unsigned int count, vk::ShaderStageFlags inShaders, vk::DescriptorBindingFlags bindingFlags) {
	AddDescriptors(count, inShaders, bindingFlags).setDescriptorType(vk::DescriptorType::eSampledImage);
	return *this;
}

VulkanDescriptorSetLayoutBuilder& VulkanDescriptorSetLayoutBuilder::WithStoragemages(unsigned int count, vk::ShaderStageFlags inShaders, vk::DescriptorBindingFlags bindingFlags) {
	AddDescriptors(count, inShaders, bindingFlags).setDescriptorType(vk::DescriptorType::eStorageImage);
	return *this;
}

VulkanDescriptorSetLayoutBuilder& VulkanDescriptorSetLayoutBuilder::WithUniformBuffers(unsigned int count, vk::ShaderStageFlags inShaders, vk::DescriptorBindingFlags bindingFlags) {
	AddDescriptors(count, inShaders, bindingFlags).setDescriptorType(vk::DescriptorType::eUniformBuffer);
	return *this;
}

VulkanDescriptorSetLayoutBuilder& VulkanDescriptorSetLayoutBuilder::WithStorageBuffers(unsigned int count, vk::ShaderStageFlags inShaders, vk::DescriptorBindingFlags bindingFlags) {
	AddDescriptors(count, inShaders, bindingFlags).setDescriptorType(vk::DescriptorType::eStorageBuffer);
	return *this;
}

VulkanDescriptorSetLayoutBuilder& VulkanDescriptorSetLayoutBuilder::WithDynamicUniformBuffers(unsigned int count, vk::ShaderStageFlags inShaders, vk::DescriptorBindingFlags bindingFlags) {
	AddDescriptors(count, inShaders, bindingFlags).setDescriptorType(vk::DescriptorType::eUniformBufferDynamic);
	return *this;
}

VulkanDescriptorSetLayoutBuilder& VulkanDescriptorSetLayoutBuilder::WitDynamichStorageBuffers(unsigned int count, vk::ShaderStageFlags inShaders, vk::DescriptorBindingFlags bindingFlags) {
	AddDescriptors(count, inShaders, bindingFlags).setDescriptorType(vk::DescriptorType::eStorageBufferDynamic);
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