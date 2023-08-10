/******************************************************************************
This file is part of the Newcastle Vulkan Tutorial Series

Author:Rich Davison
Contact:richgdavison@gmail.com
License: MIT (see LICENSE file at the top of the source tree)
*//////////////////////////////////////////////////////////////////////////////
#pragma once
#include "VulkanPipelineBuilderBase.h"
#include "VulkanPipeline.h"

namespace NCL::Rendering::Vulkan {
	class VulkanRenderer;
	class VulkanShader;
	using UniqueVulkanShader = std::unique_ptr<VulkanShader>;

	struct VulkanVertexSpecification;

	class PipelineBuilder	: public PipelineBuilderBase<PipelineBuilder, vk::GraphicsPipelineCreateInfo> {
	public:
		PipelineBuilder(vk::Device device);
		~PipelineBuilder() {}

		PipelineBuilder& WithDepthState(vk::CompareOp op, bool depthEnabled, bool writeEnabled, bool stencilEnabled = false);

		PipelineBuilder& WithBlendState(vk::BlendFactor srcState, vk::BlendFactor dstState, bool enabled = true);

		PipelineBuilder& WithRaster(vk::CullModeFlagBits cullMode, vk::PolygonMode polyMode = vk::PolygonMode::eFill);

		PipelineBuilder& WithVertexInputState(const vk::PipelineVertexInputStateCreateInfo& spec);

		PipelineBuilder& WithTessellationPatchVertexCount(uint32_t controlPointsPerPatch);

		PipelineBuilder& WithTopology(vk::PrimitiveTopology topology);

		PipelineBuilder& WithShader(const UniqueVulkanShader& shader);

		PipelineBuilder& WithPass(vk::RenderPass& renderPass);

		PipelineBuilder& WithDepthStencilFormat(vk::Format combinedFormat);
		PipelineBuilder& WithDepthFormat(vk::Format depthFormat);
		PipelineBuilder& WithColourFormats(const std::vector<vk::Format>& formats);

		VulkanPipeline	Build(const std::string& debugName = "", vk::PipelineCache cache = {});

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