/******************************************************************************
This file is part of the Newcastle Vulkan Tutorial Series

Author:Rich Davison
Contact:richgdavison@gmail.com
License: MIT (see LICENSE file at the top of the source tree)
*//////////////////////////////////////////////////////////////////////////////
#pragma once
#include "../NCLCoreClasses/Texture.h"
#include "SmartTypes.h"

namespace NCL::Rendering {
	class VulkanRenderer;

	class VulkanTexture : public Texture	{
		friend class VulkanRenderer;
	public:
		~VulkanTexture();

		static UniqueVulkanTexture CubemapFromFiles(
			VulkanRenderer* renderer,
			const std::string& negativeXFile, const std::string& positiveXFile, 
			const std::string& negativeYFile, const std::string& positiveYFile,
			const std::string& negativeZFile, const std::string& positiveZFile,
			const std::string& debugName = "CubeMap");

		static UniqueVulkanTexture TextureFromFile(VulkanRenderer* renderer, const std::string& name);
		static UniqueVulkanTexture CreateDepthTexture(VulkanRenderer* renderer, uint32_t width, uint32_t height, const std::string& debugName = "DefaultDepth", bool hasStencil = true, bool mips = false);
		static UniqueVulkanTexture CreateColourTexture(VulkanRenderer* renderer, 
			uint32_t width, uint32_t height, 
			const std::string& debugName = "DefaultColour", 
			vk::Format format = vk::Format::eB8G8R8A8Unorm, 
			vk::ImageUsageFlags flags = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled, 
			vk::ImageLayout		 initialLayout = vk::ImageLayout::eColorAttachmentOptimal,
			bool mips = false);

		vk::ImageView GetDefaultView() const {
			return *defaultView;
		}

		vk::Format GetFormat() const {
			return format;
		}

		vk::Image GetImage() const {
			return image;
		}

		//Allows us to pass a texture as vk type to various functions
		operator vk::Image() const {
			return image;
		}
		operator vk::ImageView() const {
			return *defaultView;
		}		
		operator vk::Format() const {
			return format;
		}

		void FillImageStorageView(vk::ImageView& view, vk::Device device);

	protected:
		VulkanTexture();
		void GenerateMipMaps(vk::CommandBuffer  buffer, vk::ImageLayout endLayout, vk::PipelineStageFlags endFlags);

		void	GenerateTextureInternal(VulkanRenderer* renderer, uint32_t width, uint32_t height, uint32_t mipcount, bool isCube, const std::string& debugName, vk::Format format, vk::ImageAspectFlags aspect, vk::ImageUsageFlags usage, vk::ImageLayout outLayout, vk::PipelineStageFlags pipeType);
		void	GenerateTextureFromDataInternal(VulkanRenderer* renderer, uint32_t width, uint32_t height, uint32_t channelCount, bool isCube, std::vector<char*>dataSrcs, const std::string& debugName);

		vk::UniqueImageView	defaultView;
		vk::Image			image;

		VmaAllocation		allocationHandle;
		VmaAllocationInfo	allocationInfo;
		VmaAllocator		allocator;

		vk::Format				format;

		vk::ImageCreateInfo		createInfo;
		vk::ImageAspectFlags	aspectType;

		uint32_t width;
		uint32_t height;
		uint32_t mipCount;
		uint32_t layerCount;
	};
}