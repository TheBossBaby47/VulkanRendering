/******************************************************************************
This file is part of the Newcastle Vulkan Tutorial Series

Author:Rich Davison
Contact:richgdavison@gmail.com
License: MIT (see LICENSE file at the top of the source tree)
*//////////////////////////////////////////////////////////////////////////////
#include "VulkanShaderBuilder.h"
#include "VulkanUtils.h"
#include "VulkanShader.h"
#include "VulkanDescriptorSetLayoutBuilder.h"
#include "Assets.h"
extern "C" {
#include "Spirv-reflect/Spirv_reflect.h"
}
using std::string;

using namespace NCL;
using namespace Rendering;
using namespace Vulkan;

const char* ErrorMessages[ShaderStages::MAX_SIZE] =
{
	"Multiple vertex shaders attached to shader object!",
	"Multiple fragment shaders attached to shader object!",
	"Multiple geometry shaders attached to shader object!",
	"Multiple TCS shaders attached to shader object!",
	"Multiple TES shaders attached to shader object!",	
	"Multiple mesh shaders attached to shader object!",
};

vk::ShaderStageFlagBits stageTypeEnum[] = {
	vk::ShaderStageFlagBits::eVertex,
	vk::ShaderStageFlagBits::eFragment,
	vk::ShaderStageFlagBits::eGeometry,
	vk::ShaderStageFlagBits::eTessellationControl,
	vk::ShaderStageFlagBits::eTessellationEvaluation,
	vk::ShaderStageFlagBits::eMeshNV
};

ShaderBuilder& ShaderBuilder::AddBinary(ShaderStages::Type stage, const std::string& name, const std::string& entry) {
	assert(MessageAssert(shaderFiles[stage].empty(), ErrorMessages[stage]));
	assert(MessageAssert(stage != (uint32_t)ShaderStages::MAX_SIZE, "Invalid shader stage!"));
	shaderFiles[stage] = name;
	entryPoints[stage] = entry;
	return *this;
}

ShaderBuilder& ShaderBuilder::WithMeshBinary(const string& name, const std::string& entry) {
	return AddBinary(ShaderStages::Mesh, name, entry);
}

ShaderBuilder& ShaderBuilder::WithVertexBinary(const string& name, const std::string& entry) {
	return AddBinary(ShaderStages::Vertex, name, entry);
}

ShaderBuilder& ShaderBuilder::WithFragmentBinary(const string& name, const std::string& entry) {
	return AddBinary(ShaderStages::Fragment, name, entry);
}

ShaderBuilder& ShaderBuilder::WithGeometryBinary(const string& name, const std::string& entry) {
	return AddBinary(ShaderStages::Geometry, name, entry);
}

ShaderBuilder& ShaderBuilder::WithTessControlBinary(const string& name, const std::string& entry) {
	return AddBinary(ShaderStages::TessControl, name, entry);
}

ShaderBuilder& ShaderBuilder::WithTessEvalBinary(const string& name, const std::string& entry) {
	return AddBinary(ShaderStages::TessEval, name, entry);
}

void ShaderBuilder::AddReflectionData(uint32_t dataSize, const void* data, vk::ShaderStageFlags stage, std::vector< std::vector<vk::DescriptorSetLayoutBinding> >& setLayouts, std::vector<vk::PushConstantRange>& constants) {
	SpvReflectShaderModule module;
	SpvReflectResult result = spvReflectCreateShaderModule(dataSize, data, &module);
	assert(result == SPV_REFLECT_RESULT_SUCCESS);

	uint32_t descriptorCount = 0;
	result = spvReflectEnumerateDescriptorSets(&module, &descriptorCount, NULL);
	assert(result == SPV_REFLECT_RESULT_SUCCESS);

	std::vector<SpvReflectDescriptorSet*> descriptorSetLayouts(descriptorCount);
	result = spvReflectEnumerateDescriptorSets(&module, &descriptorCount, descriptorSetLayouts.data());
	assert(result == SPV_REFLECT_RESULT_SUCCESS);

	for (auto& set : descriptorSetLayouts) {
		if (set->set >= setLayouts.size()) {
			setLayouts.resize(set->set + 1);
		}
		std::vector<vk::DescriptorSetLayoutBinding>& setLayout = setLayouts[set->set];
		setLayout.resize(set->binding_count);
		for (int i = 0; i < set->binding_count; ++i) {
			SpvReflectDescriptorBinding* binding = set->bindings[i];

			uint32_t index = binding->binding;
			uint32_t count = binding->count;
			vk::ShaderStageFlags stages;

			if (setLayout[index].stageFlags != vk::ShaderStageFlags()) {
				//Check that something hasn't gone wrong with the binding combo!
				if(setLayout[index].descriptorType != (vk::DescriptorType)binding->descriptor_type) {

				}
				if (setLayout[index].descriptorCount != binding->count) {

				}
			}
			setLayout[index].binding = index;
			setLayout[index].descriptorCount = binding->count;
			setLayout[index].descriptorType  = (vk::DescriptorType)binding->descriptor_type;

			setLayout[index].stageFlags |= stage; //Combine sets across shader stages
		}
	}

	uint32_t pushConstantCount = 0;
	result = spvReflectEnumeratePushConstantBlocks(&module, &pushConstantCount, NULL);
	assert(result == SPV_REFLECT_RESULT_SUCCESS);

	std::vector<SpvReflectBlockVariable*> pushConstantLayouts(pushConstantCount);
	result = spvReflectEnumeratePushConstantBlocks(&module, &pushConstantCount, pushConstantLayouts.data());
	assert(result == SPV_REFLECT_RESULT_SUCCESS);

	for (auto& constant : pushConstantLayouts) {
		bool found = false;
		for (int i = 0; i < constants.size(); ++i) {
			if (constants[i].offset == constant->offset &&
				constants[i].size == constant->size) {
				constants[i].stageFlags |= stage;
				found = true;
				break;
			}
		}
		vk::PushConstantRange range;
		range.offset		= constant->offset;
		range.size			= constant->size;
		range.stageFlags	= stage;
		constants.push_back(range);
	}

	spvReflectDestroyShaderModule(&module);
}

UniqueVulkanShader ShaderBuilder::Build(const std::string& debugName) {
	VulkanShader* newShader = new VulkanShader();
	//mesh and 'traditional' pipeline are mutually exclusive
	assert(MessageAssert(!(!shaderFiles[ShaderStages::Mesh].empty() && !shaderFiles[ShaderStages::Vertex].empty()),
		"Cannot use traditional vertex pipeline with mesh shaders!"));

	std::vector<std::vector<vk::DescriptorSetLayoutBinding>> setLayoutBindings;
	std::vector<vk::PushConstantRange> pushConstants;

	for (int i = 0; i < ShaderStages::MAX_SIZE; ++i) {
		if (!shaderFiles[i].empty()) {

			char* data;
			size_t dataSize = 0;
			Assets::ReadBinaryFile(Assets::SHADERDIR + "VK/" + shaderFiles[i], &data, dataSize);

			vk::UniqueShaderModule module;

			if (dataSize > 0) {
				module = sourceDevice.createShaderModuleUnique(vk::ShaderModuleCreateInfo(vk::ShaderModuleCreateFlags(), dataSize, (uint32_t*)data));
				AddReflectionData(dataSize, data, stageTypeEnum[i], setLayoutBindings, pushConstants);
			}
			else {
				std::cout << __FUNCTION__ << " Problem loading shader file " << shaderFiles[i] << "!\n";
			}

			newShader->AddBinaryShaderModule(static_cast<ShaderStages::Type>(i), module, entryPoints[i]);

			if (!debugName.empty()) {
				SetDebugName(sourceDevice, vk::ObjectType::eShaderModule, GetVulkanHandle(*newShader->shaderModules[i]), debugName);
			}
		}
	};

	std::vector<vk::UniqueDescriptorSetLayout> setLayouts;
	for (const auto& i : setLayoutBindings) {
		if (i.empty()) {
			setLayouts.push_back(vk::UniqueDescriptorSetLayout());
		}
		else {
			vk::DescriptorSetLayoutCreateInfo createInfo;
			createInfo.setBindings(i);
			setLayouts.push_back(sourceDevice.createDescriptorSetLayoutUnique(createInfo));
		}
	}

	newShader->Init();
	newShader->AddDescriptorSetLayoutState(setLayoutBindings, setLayouts);
	newShader->AddPushConstantState(pushConstants);

	return UniqueVulkanShader(newShader);
}