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
using namespace Vulkan;

vk::DescriptorSetLayoutBinding& DescriptorSetLayoutBuilder::AddDescriptors(uint32_t index, unsigned int count, vk::ShaderStageFlags inShaders, vk::DescriptorBindingFlags bindingFlags) {
	vk::DescriptorSetLayoutBinding binding = vk::DescriptorSetLayoutBinding()
		.setBinding(index)
		.setDescriptorCount(count)
		.setStageFlags(inShaders);

	addedBindings.emplace_back(binding);
	addedFlags.emplace_back(bindingFlags);
	return addedBindings[addedBindings.size() - 1];
}

DescriptorSetLayoutBuilder& DescriptorSetLayoutBuilder::WithSamplers(uint32_t index, unsigned int count, vk::ShaderStageFlags inShaders, vk::DescriptorBindingFlags bindingFlags) {
	AddDescriptors(index, count, inShaders, bindingFlags).setDescriptorType(vk::DescriptorType::eSampler);
	return *this;
}

DescriptorSetLayoutBuilder& DescriptorSetLayoutBuilder::WithUniformTexelBuffers(uint32_t index, unsigned int count, vk::ShaderStageFlags inShaders, vk::DescriptorBindingFlags bindingFlags) {
	AddDescriptors(index, count, inShaders, bindingFlags).setDescriptorType(vk::DescriptorType::eUniformTexelBuffer);
	return *this;
}

DescriptorSetLayoutBuilder& DescriptorSetLayoutBuilder::WithStorageTexelBuffers(uint32_t index, unsigned int count, vk::ShaderStageFlags inShaders, vk::DescriptorBindingFlags bindingFlags) {
	AddDescriptors(index, count, inShaders, bindingFlags).setDescriptorType(vk::DescriptorType::eStorageTexelBuffer);
	return *this;
}

DescriptorSetLayoutBuilder& DescriptorSetLayoutBuilder::WithImageSamplers(uint32_t index, unsigned int count, vk::ShaderStageFlags inShaders, vk::DescriptorBindingFlags bindingFlags) {
	AddDescriptors(index, count, inShaders, bindingFlags).setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
	return *this;
}

DescriptorSetLayoutBuilder& DescriptorSetLayoutBuilder::WithSampledImages(uint32_t index, unsigned int count, vk::ShaderStageFlags inShaders, vk::DescriptorBindingFlags bindingFlags) {
	AddDescriptors(index, count, inShaders, bindingFlags).setDescriptorType(vk::DescriptorType::eSampledImage);
	return *this;
}

DescriptorSetLayoutBuilder& DescriptorSetLayoutBuilder::WithStorageImages(uint32_t index, unsigned int count, vk::ShaderStageFlags inShaders, vk::DescriptorBindingFlags bindingFlags) {
	AddDescriptors(index, count, inShaders, bindingFlags).setDescriptorType(vk::DescriptorType::eStorageImage);
	return *this;
}

DescriptorSetLayoutBuilder& DescriptorSetLayoutBuilder::WithUniformBuffers(uint32_t index, unsigned int count, vk::ShaderStageFlags inShaders, vk::DescriptorBindingFlags bindingFlags) {
	AddDescriptors(index, count, inShaders, bindingFlags).setDescriptorType(vk::DescriptorType::eUniformBuffer);
	return *this;
}

DescriptorSetLayoutBuilder& DescriptorSetLayoutBuilder::WithStorageBuffers(uint32_t index, unsigned int count, vk::ShaderStageFlags inShaders, vk::DescriptorBindingFlags bindingFlags) {
	AddDescriptors(index, count, inShaders, bindingFlags).setDescriptorType(vk::DescriptorType::eStorageBuffer);
	return *this;
}

DescriptorSetLayoutBuilder& DescriptorSetLayoutBuilder::WithDynamicUniformBuffers(uint32_t index, unsigned int count, vk::ShaderStageFlags inShaders, vk::DescriptorBindingFlags bindingFlags) {
	AddDescriptors(index, count, inShaders, bindingFlags).setDescriptorType(vk::DescriptorType::eUniformBufferDynamic);
	return *this;
}

DescriptorSetLayoutBuilder& DescriptorSetLayoutBuilder::WithDynamicStorageBuffers(uint32_t index, unsigned int count, vk::ShaderStageFlags inShaders, vk::DescriptorBindingFlags bindingFlags) {
	AddDescriptors(index, count, inShaders, bindingFlags).setDescriptorType(vk::DescriptorType::eStorageBufferDynamic);
	return *this;
}

DescriptorSetLayoutBuilder& DescriptorSetLayoutBuilder::WithAccelStructures(uint32_t index, unsigned int count, vk::ShaderStageFlags inShaders, vk::DescriptorBindingFlags bindingFlags) {
	AddDescriptors(index, count, inShaders, bindingFlags).setDescriptorType(vk::DescriptorType::eAccelerationStructureKHR);
	return *this;
}

DescriptorSetLayoutBuilder& DescriptorSetLayoutBuilder::WithDescriptors(uint32_t index, vk::DescriptorType type, unsigned int count, vk::ShaderStageFlags inShaders, vk::DescriptorBindingFlags bindingFlags) {
	AddDescriptors(index, count, inShaders, bindingFlags).setDescriptorType(type);
	return *this;
}

DescriptorSetLayoutBuilder& DescriptorSetLayoutBuilder::WithCreationFlags(vk::DescriptorSetLayoutCreateFlags flags) {
	createInfo.flags |= flags;
	return *this;
}

vk::UniqueDescriptorSetLayout DescriptorSetLayoutBuilder::Build(const std::string& debugName) {
	createInfo.setBindings(addedBindings);
	vk::DescriptorSetLayoutBindingFlagsCreateInfoEXT bindingFlagsInfo;
	
	bindingFlagsInfo.setBindingFlags(addedFlags);

	createInfo.pNext = &bindingFlagsInfo;
	vk::UniqueDescriptorSetLayout layout = std::move(sourceDevice.createDescriptorSetLayoutUnique(createInfo));
	if (!debugName.empty()) {
		SetDebugName(sourceDevice, vk::ObjectType::eDescriptorSetLayout, GetVulkanHandle(*layout), debugName);
	}
	return layout;
}