/******************************************************************************
This file is part of the Newcastle Vulkan Tutorial Series

Author:Rich Davison
Contact:richgdavison@gmail.com
License: MIT (see LICENSE file at the top of the source tree)
*//////////////////////////////////////////////////////////////////////////////
#pragma once
#include "VulkanPipelineBuilderBase.h"
#include "VulkanPipeline.h"

namespace NCL::Rendering {
	class VulkanRenderer;
	class VulkanShader;
	using UniqueVulkanShader = std::unique_ptr<VulkanShader>;

	struct VulkanVertexSpecification;

	class VulkanPipelineBuilder	: public VulkanPipelineBuilderBase<VulkanPipelineBuilder, vk::GraphicsPipelineCreateInfo> {
	public:
		VulkanPipelineBuilder(const std::string& debugName = "");
		~VulkanPipelineBuilder() {}

		VulkanPipelineBuilder& WithDepthState(vk::CompareOp op, bool depthEnabled, bool writeEnabled, bool stencilEnabled = false);

		VulkanPipelineBuilder& WithBlendState(vk::BlendFactor srcState, vk::BlendFactor dstState, bool enabled = true);

		VulkanPipelineBuilder& WithRaster(vk::CullModeFlagBits cullMode, vk::PolygonMode polyMode = vk::PolygonMode::eFill);

		VulkanPipelineBuilder& WithVertexInputState(const vk::PipelineVertexInputStateCreateInfo& spec);

		VulkanPipelineBuilder& WithTessellationPatchVertexCount(uint32_t controlPointsPerPatch);

		VulkanPipelineBuilder& WithTopology(vk::PrimitiveTopology topology);

		VulkanPipelineBuilder& WithShader(const UniqueVulkanShader& shader);

		VulkanPipelineBuilder& WithPass(vk::RenderPass& renderPass);

		VulkanPipelineBuilder& WithDepthStencilFormat(vk::Format combinedFormat);
		VulkanPipelineBuilder& WithDepthFormat(vk::Format depthFormat);
		VulkanPipelineBuilder& WithColourFormats(const std::vector<vk::Format>& formats);

		VulkanPipeline	Build(vk::Device device, vk::PipelineCache cache = {});

	protected:
		vk::PipelineCacheCreateInfo					cacheCreate;
		vk::PipelineInputAssemblyStateCreateInfo	inputAsmCreate;
		vk::PipelineRasterizationStateCreateInfo	rasterCreate;
		vk::PipelineColorBlendStateCreateInfo		blendCreate;
		vk::PipelineDepthStencilStateCreateInfo		depthStencilCreate;
		vk::PipelineViewportStateCreateInfo			viewportCreate;
		vk::PipelineMultisampleStateCreateInfo		sampleCreate;
		vk::PipelineDynamicStateCreateInfo			dynamicCreate;
		vk::PipelineVertexInputStateCreateInfo		vertexCreate;
		vk::PipelineTessellationStateCreateInfo		tessellationCreate;

		std::vector< vk::PipelineColorBlendAttachmentState>			blendAttachStates;

		vk::DynamicState dynamicStateEnables[2];

		std::vector<vk::Format> allColourRenderingFormats;
		vk::Format depthRenderingFormat;
		vk::Format stencilRenderingFormat;
	};
}