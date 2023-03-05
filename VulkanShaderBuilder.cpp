/******************************************************************************
This file is part of the Newcastle Vulkan Tutorial Series

Author:Rich Davison
Contact:richgdavison@gmail.com
License: MIT (see LICENSE file at the top of the source tree)
*//////////////////////////////////////////////////////////////////////////////
#include "VulkanUtils.h"
#include "VulkanShader.h"
#include "VulkanShaderBuilder.h"

using std::string;
using namespace NCL;
using namespace Rendering;

const char* ErrorMessages[(uint32_t)ShaderStages::MAXSIZE] =
{
	"Multiple mesh shaders attached to shader object!",
	"Multiple vertex shaders attached to shader object!",
	"Multiple fragment shaders attached to shader object!",
	"Multiple geometry shaders attached to shader object!",
	"Multiple TCS shaders attached to shader object!",
	"Multiple TES shaders attached to shader object!",

};

VulkanShaderBuilder& VulkanShaderBuilder::AddBinary(ShaderStages stage, const std::string& name, const std::string& entry) {
	const uint32_t index = (uint32_t)stage;
	assert(Vulkan::MessageAssert(shaderFiles[index].empty(), ErrorMessages[index]));
	assert(Vulkan::MessageAssert(index != (uint32_t)ShaderStages::MAXSIZE, "Invalid shader stage!"));
	shaderFiles[index] = name;
	entryPoints[index] = entry;
	return *this;
}

VulkanShaderBuilder& VulkanShaderBuilder::WithMeshBinary(const string& name, const std::string& entry) {
	return AddBinary(ShaderStages::Mesh, name, entry);
}

VulkanShaderBuilder& VulkanShaderBuilder::WithVertexBinary(const string& name, const std::string& entry) {
	return AddBinary(ShaderStages::Vertex, name, entry);
}

VulkanShaderBuilder& VulkanShaderBuilder::WithFragmentBinary(const string& name, const std::string& entry) {
	return AddBinary(ShaderStages::Fragment, name, entry);
}

VulkanShaderBuilder& VulkanShaderBuilder::WithGeometryBinary(const string& name, const std::string& entry) {
	return AddBinary(ShaderStages::Geometry, name, entry);
}

VulkanShaderBuilder& VulkanShaderBuilder::WithTessControlBinary(const string& name, const std::string& entry) {
	return AddBinary(ShaderStages::TessControl, name, entry);
}

VulkanShaderBuilder& VulkanShaderBuilder::WithTessEvalBinary(const string& name, const std::string& entry) {
	return AddBinary(ShaderStages::TessEval, name, entry);
}

UniqueVulkanShader VulkanShaderBuilder::Build(vk::Device device) {
	VulkanShader* newShader = new VulkanShader();
	//mesh and 'traditional' pipeline are mutually exclusive
	assert(Vulkan::MessageAssert(!(!shaderFiles[(int)ShaderStages::Mesh].empty() && !shaderFiles[(int)ShaderStages::Vertex].empty()),
		"Cannot use traditional vertex pipeline with mesh shaders!"));
	
	for (int i = 0; i < (int)ShaderStages::MAXSIZE; ++i) {
		if (!shaderFiles[i].empty()) {
			newShader->AddBinaryShaderModule(shaderFiles[i],(ShaderStages)i, device, entryPoints[i]);

			if (!debugName.empty()) {
				Vulkan::SetDebugName(device, vk::ObjectType::eShaderModule, Vulkan::GetVulkanHandle(*newShader->shaderModules[i]), debugName);
			}
		}
	};
	newShader->Init();
	return UniqueVulkanShader(newShader);
}