/******************************************************************************
This file is part of the Newcastle Vulkan Tutorial Series

Author:Rich Davison
Contact:richgdavison@gmail.com
License: MIT (see LICENSE file at the top of the source tree)
*//////////////////////////////////////////////////////////////////////////////
#include "VulkanTexture.h"
#include "VulkanRenderer.h"
#include "TextureLoader.h"
#include "VulkanUtils.h"
#include "VulkanBuffers.h"
#include "VulkanBufferBuilder.h"

using namespace NCL;
using namespace Rendering;
using namespace Vulkan;

int MipCount(uint32_t width, uint32_t height) {
	return (int)floor(log2(float(std::min(width, height)))) + 1;
}

VulkanTexture::VulkanTexture() {
	mipCount	= 0;
	layerCount	= 0;
	format		= vk::Format::eUndefined;
}

VulkanTexture::~VulkanTexture() {
	if (image) {
		vmaDestroyImage(allocator, image, allocationHandle);
	}
}

//UniqueVulkanTexture VulkanTexture::CreateColourTexture(VulkanRenderer* renderer, uint32_t width, uint32_t height, const std::string& debugName, vk::Format format, vk::ImageUsageFlags usage, vk::ImageLayout		 layout, bool useMips) {
//	vk::ImageAspectFlags aspect = vk::ImageAspectFlagBits::eColor;
//	
//	int numMips = useMips ? MipCount(width, height) : 1;
//
//	VulkanTexture* t = new VulkanTexture();
//	t->GenerateTextureInternal(renderer, width, height, numMips, false, debugName, format, aspect, usage, layout, vk::PipelineStageFlagBits::eColorAttachmentOutput);
//	return UniqueVulkanTexture(t);
//}
//
//UniqueVulkanTexture VulkanTexture::CreateDepthTexture(VulkanRenderer* renderer, uint32_t width, uint32_t height, const std::string& debugName, bool hasStencil, bool useMips) {
//	vk::Format			 format = hasStencil ? vk::Format::eD24UnormS8Uint : vk::Format::eD32Sfloat;
//	vk::ImageAspectFlags aspect = hasStencil ? vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil : vk::ImageAspectFlagBits::eDepth;
//	vk::ImageLayout		 layout = hasStencil ? vk::ImageLayout::eDepthStencilAttachmentOptimal : vk::ImageLayout::eDepthAttachmentOptimal;
//	vk::ImageUsageFlags  usage = vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled;
//
//	int numMips = useMips ? MipCount(width, height) : 1;
//
//	VulkanTexture* t = new VulkanTexture();
//	t->GenerateTextureInternal(renderer, width, height, numMips, false, debugName, format, aspect, usage, layout, vk::PipelineStageFlagBits::eEarlyFragmentTests);
//	return UniqueVulkanTexture(t);
//}

// UniqueVulkanTexture VulkanTexture::TextureFromFile(VulkanRenderer* renderer, const std::string& name) {
//	char* texData	= nullptr;
//	int width		= 0;
//	int height		= 0;
//	int channels	= 0;
//	int flags		= 0;
//	TextureLoader::LoadTexture(name, texData, width, height, channels, flags);
//
//	if (width == 0 || height == 0) {
//		std::cout << __FUNCTION__ << " can't load texture " << name << "\n";
//		return nullptr;
//	}
//	VulkanTexture* texture = new VulkanTexture();
//	texture->GenerateTextureFromDataInternal(renderer, width, height, channels, false, { texData }, name);
//	delete texData;
//	return UniqueVulkanTexture(texture);
//};

//UniqueVulkanTexture VulkanTexture::CubemapFromFiles(
//	VulkanRenderer* renderer,
//	const std::string& negativeXFile, const std::string& positiveXFile,
//	const std::string& negativeYFile, const std::string& positiveYFile,
//	const std::string& negativeZFile, const std::string& positiveZFile,
//	const std::string& debugName) {
//
//	std::vector<const string*> allFiles = { &negativeXFile, &positiveXFile, &negativeYFile, &positiveYFile, &negativeZFile, &positiveZFile };
//
//	std::vector<char*> texData(6, nullptr);
//	int width[6]	 = { 0 };
//	int height[6]	 = { 0 };
//	int channels[6]  = { 0 };
//	int flags[6]	 = { 0 };
//
//	bool loadedData = true;
//	for (int i = 0; i < 6; ++i) {
//		TextureLoader::LoadTexture(*(allFiles[i]), texData[i], width[i], height[i], channels[i], flags[i]);
//		if (i > 0 && (width[i] != width[0] || height[0] != height[0])) {
//			std::cout << __FUNCTION__ << " cubemap input textures don't match in size?\n";
//			loadedData = false;
//			break;
//		}
//		if (width[i] == 0 || height[i] == 0) {
//			std::cout << __FUNCTION__ << " can't load cubemap " << *allFiles[i] << "\n";
//			loadedData = false;
//			break;
//		}
//	}
//	if (!loadedData) {
//		for (int i = 0; i < 6; ++i) {
//			delete texData[i];
//		}
//		return nullptr;
//	}
//	VulkanTexture* cubeTex = new VulkanTexture();
//	cubeTex->GenerateTextureFromDataInternal(renderer, width[0], height[0], channels[0], true, texData, debugName);
//
//	//delete the old texData;
//	for (int i = 0; i < 6; ++i) {
//		delete texData[i];
//	}
//
//	return UniqueVulkanTexture(cubeTex);
//}

//void VulkanTexture::UploadData(vk::Device device, VmaAllocator allocator, vk::CommandBuffer  cmdBuffer, const char* data, uint32_t dataWidth, uint32_t dataHeight, uint32_t dataBpp) {
//	
//	int faceSize = dataWidth * dataHeight * dataBpp;
//
//	int allocationSize = faceSize;// *(int)dataSrcs.size();
//	
//	VulkanBuffer stagingBuffer = BufferBuilder(device, allocator)
//		.WithBufferUsage(vk::BufferUsageFlagBits::eTransferSrc)
//		.WithHostVisibility()
//		.Build(allocationSize, "Staging Buffer");
//
//	//our buffer now has memory! Copy some texture date to it...
//	char* gpuPtr = (char*)stagingBuffer.Map();
//	for (int i = 0; i < dataSrcs.size(); ++i) {
//		memcpy(gpuPtr, data, faceSize);
//		gpuPtr += faceSize;
//		//We'll also set up each layer of the image to accept new transfers
//		ImageTransitionBarrier(cmdBuffer, image, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, aspectType, vk::PipelineStageFlagBits::eHost, vk::PipelineStageFlagBits::eTransfer, 0, i);
//	}
//	stagingBuffer.Unmap();
//
//	vk::BufferImageCopy copyInfo;
//	copyInfo.imageSubresource.setAspectMask(vk::ImageAspectFlagBits::eColor).setMipLevel(0).setLayerCount(1);
//	copyInfo.imageExtent = vk::Extent3D(dataWidth, dataHeight, 1);
//
//	//Copy from staging buffer to image memory...
//	cmdBuffer.copyBufferToImage(stagingBuffer.buffer, image, vk::ImageLayout::eTransferDstOptimal, copyInfo);
//}

//void VulkanTexture::GenerateTextureFromDataInternal(VulkanRenderer* vkRenderer, uint32_t width, uint32_t height, uint32_t channelCount, bool isCube, std::vector<char*>dataSrcs, const std::string& debugName) {
//	vk::Format				format = vk::Format::eR8G8B8A8Unorm;
//	vk::ImageAspectFlags	aspect = vk::ImageAspectFlagBits::eColor;
//	vk::ImageLayout		layout = vk::ImageLayout::eShaderReadOnlyOptimal;
//	vk::ImageUsageFlags	usage = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eTransferSrc;
//
//	int mipCount = (int)floor(log2(float(std::min(width, height)))) + 1;
//
//	GenerateTextureInternal(vkRenderer, width, height, mipCount, isCube, debugName, format, aspect, usage, layout, vk::PipelineStageFlagBits::eFragmentShader);
//
//	//tex is currently empty, need to fill it with our data from stbimage!
//	int faceSize = width * height * channelCount;
//	int allocationSize = faceSize * (int)dataSrcs.size();
//
//	vk::Device device		= vkRenderer->GetDevice();
//	vk::Queue gfxQueue		= vkRenderer->GetQueue(CommandBuffer::Graphics);
//	vk::CommandPool pool	= vkRenderer->GetCommandPool(CommandBuffer::Graphics);
//
//	VulkanBuffer stagingBuffer = BufferBuilder(device, vkRenderer->GetMemoryAllocator())
//		.WithBufferUsage(vk::BufferUsageFlagBits::eTransferSrc)
//		.WithHostVisibility()
//		.Build(allocationSize, "Staging Buffer");
//
//	vk::UniqueCommandBuffer cmdBuffer = CmdBufferBegin(device, pool, "VulkanTexture upload");
//
//	//our buffer now has memory! Copy some texture date to it...
//	char* gpuPtr = (char*)stagingBuffer.Map();
//	for (int i = 0; i < dataSrcs.size(); ++i) {
//		memcpy(gpuPtr, dataSrcs[i], faceSize);
//		gpuPtr += faceSize;
//		//We'll also set up each layer of the image to accept new transfers
//		ImageTransitionBarrier(*cmdBuffer, image, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, aspectType, vk::PipelineStageFlagBits::eHost, vk::PipelineStageFlagBits::eTransfer, 0, 1, i);
//	}
//	stagingBuffer.Unmap();
//
//	vk::BufferImageCopy copyInfo;
//	copyInfo.imageSubresource.setAspectMask(vk::ImageAspectFlagBits::eColor).setMipLevel(0).setLayerCount((uint32_t)dataSrcs.size());
//	copyInfo.imageExtent = vk::Extent3D(width, height, 1);
//
//	//Copy from staging buffer to image memory...
//	cmdBuffer->copyBufferToImage(stagingBuffer.buffer, image, vk::ImageLayout::eTransferDstOptimal, copyInfo);
//
//	if (mipCount > 1) {
//		GenerateMipMaps(*cmdBuffer, vk::ImageLayout::eShaderReadOnlyOptimal, vk::PipelineStageFlagBits::eFragmentShader);
//	}
//	else {
//		ImageTransitionBarrier(*cmdBuffer, image, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal, aspectType, vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader);
//	}
//	CmdBufferEndSubmitWait(*cmdBuffer, device, gfxQueue);
//	//Staging buffer will fall out of scope and be auto destroyed
//}
//
//void VulkanTexture::GenerateTextureInternal(VulkanRenderer* vkRenderer, uint32_t texWidth, uint32_t texHeight, uint32_t texMipCount, bool texIsCubemap, const std::string& debugName, vk::Format texFormat, vk::ImageAspectFlags texAspect, vk::ImageUsageFlags texUsage, vk::ImageLayout outLayout, vk::PipelineStageFlags pipeType) {
//	dimensions.x = texWidth;
//	dimensions.y = texHeight;
//	mipCount	= texMipCount;
//	format		= texFormat;
//	aspectType = texAspect;
//	layerCount = 1;
//
//	createInfo = vk::ImageCreateInfo()
//		.setImageType(vk::ImageType::e2D)
//		.setExtent(vk::Extent3D(texWidth, texHeight, 1))
//		.setFormat(texFormat)
//		.setUsage(texUsage)
//		.setMipLevels(mipCount)
//		.setArrayLayers(1);
//
//	if (texIsCubemap) {
//		createInfo.setArrayLayers(6).setFlags(vk::ImageCreateFlagBits::eCubeCompatible);
//		layerCount = 6;
//	}
//
//	VmaAllocationCreateInfo vmaallocInfo = {};
//	vmaallocInfo.usage = VMA_MEMORY_USAGE_AUTO;
//	allocator = vkRenderer->GetMemoryAllocator();
//	vmaCreateImage(allocator, (VkImageCreateInfo*)&createInfo, &vmaallocInfo, (VkImage*)&image, &allocationHandle, &allocationInfo);
//
//	vk::ImageViewCreateInfo createInfo = vk::ImageViewCreateInfo()
//		.setViewType(texIsCubemap ? vk::ImageViewType::eCube : vk::ImageViewType::e2D)
//		.setFormat(format)
//		.setSubresourceRange(vk::ImageSubresourceRange(aspectType, 0, VK_REMAINING_MIP_LEVELS, 0, VK_REMAINING_ARRAY_LAYERS))
//		.setImage(image);
//
//	defaultView = vkRenderer->GetDevice().createImageViewUnique(createInfo);
//
//	SetDebugName(vkRenderer->GetDevice(), vk::ObjectType::eImage, GetVulkanHandle(image), debugName);
//	SetDebugName(vkRenderer->GetDevice(), vk::ObjectType::eImageView, GetVulkanHandle(*defaultView), debugName);
//
//	vk::Queue gfxQueue		= vkRenderer->GetQueue(CommandBuffer::Graphics);
//	vk::CommandPool pool	= vkRenderer->GetCommandPool(CommandBuffer::Graphics);
//	vk::Device device		= vkRenderer->GetDevice();
//
//	vk::UniqueCommandBuffer cmdBuffer = CmdBufferBegin(device, pool, "VulkanTexture generate");
//	ImageTransitionBarrier(*cmdBuffer, GetImage(), vk::ImageLayout::eUndefined, outLayout, aspectType, vk::PipelineStageFlagBits::eTopOfPipe, pipeType);
//	CmdBufferEndSubmitWait(*cmdBuffer, device, gfxQueue);
//}

void VulkanTexture::GenerateMipMaps(vk::CommandBuffer  buffer, vk::ImageLayout endLayout, vk::PipelineStageFlags endFlags) {
	for (int layer = 0; layer < layerCount; ++layer) {	
		ImageTransitionBarrier(buffer, image, 
			vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferSrcOptimal, 
			aspectType, 
			vk::PipelineStageFlagBits::eAllCommands, vk::PipelineStageFlagBits::eTransfer, 
			0, 1, layer);
		
		for (uint32_t mip = 1; mip < mipCount; ++mip) {
			vk::ImageBlit blitData;
			blitData.srcSubresource.setAspectMask(vk::ImageAspectFlagBits::eColor)
				.setMipLevel(mip - 1)
				.setBaseArrayLayer(layer)
				.setLayerCount(1);
			blitData.srcOffsets[0] = vk::Offset3D(0, 0, 0);
			blitData.srcOffsets[1].x = std::max(dimensions.x >> (mip - 1), 1);
			blitData.srcOffsets[1].y = std::max(dimensions.y >> (mip - 1), 1);
			blitData.srcOffsets[1].z = 1;

			blitData.dstSubresource.setAspectMask(vk::ImageAspectFlagBits::eColor)
				.setMipLevel(mip)
				.setLayerCount(1)
				.setBaseArrayLayer(layer);
			blitData.dstOffsets[0] = vk::Offset3D(0, 0, 0);
			blitData.dstOffsets[1].x = std::max(dimensions.x >> mip, 1);
			blitData.dstOffsets[1].y = std::max(dimensions.y >> mip, 1);
			blitData.dstOffsets[1].z = 1;

			ImageTransitionBarrier(buffer, image, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, aspectType, vk::PipelineStageFlagBits::eHost, vk::PipelineStageFlagBits::eTransfer, mip, 1, layer);
			buffer.blitImage(image, vk::ImageLayout::eTransferSrcOptimal, image, vk::ImageLayout::eTransferDstOptimal, blitData, vk::Filter::eLinear);
			ImageTransitionBarrier(buffer, image, vk::ImageLayout::eTransferSrcOptimal, endLayout, aspectType, vk::PipelineStageFlagBits::eTransfer, endFlags, mip - 1, 1, layer);

			if (mip < this->mipCount - 1) {
				ImageTransitionBarrier(buffer, image, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eTransferSrcOptimal, aspectType, vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eTransfer, mip, 1, layer);
			}
			else {
				ImageTransitionBarrier(buffer, image, vk::ImageLayout::eTransferDstOptimal, endLayout, aspectType, vk::PipelineStageFlagBits::eTransfer, endFlags, mip, 1, layer);
			}
		}
	}
}

//void VulkanTexture::FillImageStorageView(vk::ImageView& view, vk::Device device) {
//	//vk::ImageViewCreateInfo createInfo = vk::ImageViewCreateInfo()
//	//	.setViewType(texIsCubemap ? vk::ImageViewType::eCube : vk::ImageViewType::e2D)
//	//	.setFormat(format)
//	//	.setSubresourceRange(vk::ImageSubresourceRange(aspectType, 0, mipCount, 0, layerCount))
//	//	.setImage(image);
//
//	//view = device.createImageViewUnique(createInfo);
//}