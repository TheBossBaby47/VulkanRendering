/******************************************************************************
This file is part of the Newcastle Vulkan Tutorial Series

Author:Rich Davison
Contact:richgdavison@gmail.com
License: MIT (see LICENSE file at the top of the source tree)
*//////////////////////////////////////////////////////////////////////////////
#pragma once
#include "../NCLCoreClasses/MeshGeometry.h"
#include "VulkanBuffers.h"

namespace NCL::Rendering {
	class VulkanMesh : public MeshGeometry {
	public:
		friend class VulkanRenderer;
		VulkanMesh();
		VulkanMesh(const std::string& filename);
		~VulkanMesh();

		const vk::PipelineVertexInputStateCreateInfo& GetVertexInputState() const {
			return vertexInputState;
		}

		void BindToCommandBuffer(vk::CommandBuffer  buffer) const;

		void UploadToGPU(RendererBase* renderer) override;
		void UploadToGPU(VulkanRenderer* renderer, VkQueue queue, vk::CommandBuffer buffer, VulkanBuffer& stagingBuffer);

		uint32_t	GetAttributeMask() const;
		size_t		CalculateGPUAllocationSize() const;
		vk::PrimitiveTopology GetVulkanTopology() const;

		operator const vk::PipelineVertexInputStateCreateInfo&() const {
			return vertexInputState;
		}

	protected:
		vk::PipelineVertexInputStateCreateInfo				vertexInputState;
		std::vector<vk::VertexInputAttributeDescription>	attributeDescriptions;
		std::vector<vk::VertexInputBindingDescription>		attributeBindings;		
	
		VulkanBuffer gpuBuffer;
		size_t indexOffset = 0;
	
		uint32_t attributeMask = 0;

		vector<vk::Buffer>			usedBuffers;
		vector<vk::DeviceSize>		usedOffsets;
		vector< VertexAttribute >	usedAttributes;
	};
}