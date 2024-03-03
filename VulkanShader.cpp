/******************************************************************************
This file is part of the Newcastle Vulkan Tutorial Series

Author:Rich Davison
Contact:richgdavison@gmail.com
License: MIT (see LICENSE file at the top of the source tree)
*//////////////////////////////////////////////////////////////////////////////
#include "VulkanShader.h"
#include "VulkanDescriptorSetLayoutBuilder.h"
#include "Assets.h"
extern "C" {
#include "Spirv-reflect/Spirv_reflect.h"
}

using std::ifstream;

using namespace NCL;
using namespace Rendering;
using namespace Vulkan;

//These have both been ordered to match the ShaderStages enum for easy lookup!
vk::ShaderStageFlagBits stageTypes[] = {
	vk::ShaderStageFlagBits::eVertex,
	vk::ShaderStageFlagBits::eFragment, 
	vk::ShaderStageFlagBits::eGeometry,
	vk::ShaderStageFlagBits::eTessellationControl,
	vk::ShaderStageFlagBits::eTessellationEvaluation,
	vk::ShaderStageFlagBits::eMeshNV
};

VulkanShader::VulkanShader()	{
	stageCount	= 0;
	infos		= nullptr;
}

VulkanShader::~VulkanShader()	{
	delete infos;
}

void VulkanShader::ReloadShader() {

}

void VulkanShader::AddBinaryShaderModule(const std::string& fromFile, ShaderStages::Type stage, vk::Device device, const std::string& entryPoint) {
	char* data;
	size_t dataSize = 0;
	Assets::ReadBinaryFile(Assets::SHADERDIR + "VK/" + fromFile, &data, dataSize);

	if (dataSize > 0) {
		shaderModules[stage] = device.createShaderModuleUnique(vk::ShaderModuleCreateInfo(vk::ShaderModuleCreateFlags(), dataSize, (uint32_t*)data));
		GetReflection(dataSize, data);
	}
	else {
		std::cout << __FUNCTION__ << " Problem loading shader file " << fromFile << "!\n";
	}

	shaderFiles[stage] = fromFile;
	entryPoints[stage] = entryPoint;
}

void VulkanShader::AddBinaryShaderModule(ShaderStages::Type stage, vk::UniqueShaderModule& shaderModule, const std::string& entryPoint) {
	shaderModule.swap(shaderModules[stage]);
	entryPoints[stage]		= entryPoint;
}

void VulkanShader::Init() {
	stageCount = 0;
	for (int i = 0; i < ShaderStages::MAX_SIZE; ++i) {
		if (shaderModules[i]) {
			stageCount++;
		}
	}
	infos = new vk::PipelineShaderStageCreateInfo[stageCount];

	uint32_t doneCount = 0;
	for (int i = 0; i < ShaderStages::MAX_SIZE; ++i) {
		if (shaderModules[i]) {
			infos[doneCount].stage	= stageTypes[i];
			infos[doneCount].module = *shaderModules[i];
			infos[doneCount].pName	= entryPoints[i].c_str();

			doneCount++;
			if (doneCount >= stageCount) {
				break;
			}
		}
	}
}

void	VulkanShader::FillShaderStageCreateInfo(vk::GraphicsPipelineCreateInfo &info) const {
	info.setStageCount(stageCount); 
	info.setPStages(infos);
}

void VulkanShader::GetReflection(uint32_t dataSize, const void* data) {
	SpvReflectShaderModule module;
	SpvReflectResult result = spvReflectCreateShaderModule(dataSize, data, &module);
	assert(result == SPV_REFLECT_RESULT_SUCCESS);


	uint32_t count = 0;
	result = spvReflectEnumerateDescriptorSets(&module, &count, NULL);
	assert(result == SPV_REFLECT_RESULT_SUCCESS);

	std::vector<SpvReflectDescriptorSet*> sets(count);
	result = spvReflectEnumerateDescriptorSets(&module, &count, sets.data());
	assert(result == SPV_REFLECT_RESULT_SUCCESS);


	spvReflectDestroyShaderModule(&module);
}

bool	VulkanShader::GetDescriptorSetLayout(uint32_t index, const vk::UniqueDescriptorSetLayout* out) {
	if (index >= allLayouts.size())
	{
		return false;
	}
	out = &allLayouts[index];
	return true;
}

void	VulkanShader::FillDescriptorSetLayouts(std::vector<vk::DescriptorSetLayout>& layouts) {
	layouts.resize(allLayouts.size());
	for (int i = 0; i < allLayouts.size(); ++i) {
		layouts[i] = *allLayouts[i];
	}
}

void	VulkanShader::FillPushConstants(std::vector<vk::PushConstantRange>& constants) {
	constants.clear();
	constants = pushConstants;
}

void VulkanShader::AddDescriptorSetLayoutState(std::vector<std::vector<vk::DescriptorSetLayoutBinding>>& data, std::vector<vk::UniqueDescriptorSetLayout>& layouts) {
	allLayoutsBindings	= std::move(data);
	allLayouts			= std::move(layouts);
}

void VulkanShader::AddPushConstantState(std::vector<vk::PushConstantRange>& data) {
	pushConstants = data;
}