/******************************************************************************
This file is part of the Newcastle Vulkan Tutorial Series

Author:Rich Davison
Contact:richgdavison@gmail.com
License: MIT (see LICENSE file at the top of the source tree)
*//////////////////////////////////////////////////////////////////////////////
#pragma once
#include "../NCLCoreClasses/Shader.h"

namespace NCL::Rendering::Vulkan {
	class DescriptorSetLayoutBuilder;
	class VulkanShader : public Shader {
	public:
		friend class VulkanRenderer;
		friend class ShaderBuilder;

		void ReloadShader() override;

		void	FillShaderStageCreateInfo(vk::GraphicsPipelineCreateInfo& info) const;

		bool	GetDescriptorSetLayout(uint32_t index, const vk::UniqueDescriptorSetLayout* out);

		void	FillDescriptorSetLayouts(std::vector<vk::DescriptorSetLayout>& layouts);

		void	FillPushConstants(std::vector<vk::PushConstantRange>& constants);

		~VulkanShader();

	protected:
		void AddBinaryShaderModule(ShaderStages::Type stage, vk::UniqueShaderModule& shaderModule, const std::string& entryPoint = "main");
		void AddBinaryShaderModule(const std::string& fromFile, ShaderStages::Type stage, vk::Device device, const std::string& entryPoint = "main");

		void AddDescriptorSetLayoutState(std::vector<std::vector<vk::DescriptorSetLayoutBinding>>& data, std::vector<vk::UniqueDescriptorSetLayout>& layouts);
		void AddPushConstantState(std::vector<vk::PushConstantRange>& data);

		void Init();

	protected:			
		VulkanShader();
		void GetReflection(uint32_t dataSize, const void* data);

		std::vector<std::vector<vk::DescriptorSetLayoutBinding>> allLayoutsBindings;
		std::vector<vk::UniqueDescriptorSetLayout> allLayouts;

		std::vector<vk::PushConstantRange> pushConstants;

		vk::UniqueShaderModule shaderModules[ShaderStages::MAX_SIZE];
		std::string entryPoints[ShaderStages::MAX_SIZE];

		uint32_t stageCount;
		vk::PipelineShaderStageCreateInfo* infos;
	};
}