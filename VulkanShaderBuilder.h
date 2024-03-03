/******************************************************************************
This file is part of the Newcastle Vulkan Tutorial Series

Author:Rich Davison
Contact:richgdavison@gmail.com
License: MIT (see LICENSE file at the top of the source tree)
*//////////////////////////////////////////////////////////////////////////////
#pragma once
#include "../NCLCoreClasses/Shader.h"
#include "SmartTypes.h"

namespace NCL::Rendering::Vulkan {
	class DescriptorSetLayoutBuilder;
	class VulkanShader;
	class ShaderBuilder {
	public:
		ShaderBuilder(vk::Device device) { sourceDevice = device; };
		~ShaderBuilder()	{};

		ShaderBuilder& WithMeshBinary(const std::string& name, const std::string& entry = "main");

		ShaderBuilder& WithVertexBinary(const std::string& name, const std::string& entry = "main");
		ShaderBuilder& WithFragmentBinary(const std::string& name, const std::string& entry = "main");
		ShaderBuilder& WithGeometryBinary(const std::string& name, const std::string& entry = "main");
		ShaderBuilder& WithTessControlBinary(const std::string& name, const std::string& entry = "main");
		ShaderBuilder& WithTessEvalBinary(const std::string& name, const std::string& entry = "main");

		ShaderBuilder& AddBinary(ShaderStages::Type stage, const std::string& name, const std::string& entry = "main");

		UniqueVulkanShader Build(const std::string& debugName = "");
	protected:
		void AddReflectionData(uint32_t dataSize, const void* data, vk::ShaderStageFlags stage, std::vector< std::vector<vk::DescriptorSetLayoutBinding>>& sets, std::vector<vk::PushConstantRange> &constants);
	protected:
		std::string shaderFiles[ShaderStages::MAX_SIZE];
		std::string entryPoints[ShaderStages::MAX_SIZE];
		vk::Device	sourceDevice;
	};
}