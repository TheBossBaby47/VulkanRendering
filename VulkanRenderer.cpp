/******************************************************************************
This file is part of the Newcastle Vulkan Tutorial Series

Author:Rich Davison
Contact:richgdavison@gmail.com
License: MIT (see LICENSE file at the top of the source tree)
*//////////////////////////////////////////////////////////////////////////////
#include "VulkanRenderer.h"
#include "VulkanMesh.h"
#include "VulkanTexture.h"
#include "VulkanPipeline.h"
#include "VulkanBuffers.h"
#include "VulkanUtils.h"
#include "VulkanDescriptorSetLayoutBuilder.h"

#include "TextureLoader.h"

#ifdef _WIN32
#include "Win32Window.h"
using namespace NCL::Win32Code;
#endif

#define VMA_IMPLEMENTATION
#include "vma/vk_mem_alloc.h"

using namespace NCL;
using namespace Rendering;

vk::PhysicalDeviceDescriptorIndexingFeatures indexingFeatures;

VulkanRenderer::VulkanRenderer(Window& window) : RendererBase(window) {
	depthBuffer			= nullptr;
	frameBuffers		= nullptr;
	currentSwap			= 0;
	computeQueueIndex	= 0;
	gfxQueueIndex		= 0;
	gfxPresentIndex		= 0;

	majorVersion		= 1;
	minorVersion		= 1;

	deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
	deviceExtensions.push_back("VK_KHR_dynamic_rendering");
	deviceExtensions.push_back("VK_KHR_maintenance4");

	deviceExtensions.push_back("VK_KHR_depth_stencil_resolve");
	deviceExtensions.push_back("VK_KHR_create_renderpass2");
	deviceLayers.push_back("VK_LAYER_LUNARG_standard_validation");


	instanceExtensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
	instanceExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
	instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#ifdef WIN32
	instanceExtensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#endif

	instanceLayers.push_back("VK_LAYER_KHRONOS_validation");
}

VulkanRenderer::~VulkanRenderer() {
	depthBuffer.reset();

	for (auto& i : swapChainList) {
		device.destroyImageView(i->view);
	};

	for (unsigned int i = 0; i < numFrameBuffers; ++i) {
		device.destroyFramebuffer(frameBuffers[i]);
	}
	vmaDestroyAllocator(memoryAllocator);
	device.destroyDescriptorPool(defaultDescriptorPool);
	device.destroySwapchainKHR(swapChain);
	device.destroyCommandPool(commandPool);
	device.destroyCommandPool(computeCommandPool);
	device.destroyRenderPass(defaultRenderPass);
	device.destroyPipelineCache(pipelineCache);
	device.destroy(); //Destroy everything except instance before this gets destroyed!

	delete Vulkan::dispatcher;

	instance.destroySurfaceKHR(surface);
	instance.destroy();

	delete[] frameBuffers;
}

bool VulkanRenderer::Init() {
	InitInstance();

	InitPhysicalDevice();

	InitGPUDevice();
	InitMemoryAllocator();

	InitCommandPools();
	InitDefaultDescriptorPool();

	hostWindow.SetRenderer(this);
	OnWindowResize((int)hostWindow.GetScreenSize().x, (int)hostWindow.GetScreenSize().y);

	pipelineCache = device.createPipelineCache(vk::PipelineCacheCreateInfo());

	defaultCmdBuffer = swapChainList[currentSwap]->frameCmds;

	return true;
}

bool VulkanRenderer::InitInstance() {
	vk::ApplicationInfo appInfo = vk::ApplicationInfo(this->hostWindow.GetTitle().c_str());

	appInfo.apiVersion = VK_MAKE_VERSION(majorVersion, minorVersion, 0);

	vk::InstanceCreateInfo instanceInfo = vk::InstanceCreateInfo(vk::InstanceCreateFlags(), &appInfo)
		.setEnabledExtensionCount((uint32_t)instanceExtensions.size())
		.setPpEnabledExtensionNames(instanceExtensions.data())
		.setEnabledLayerCount((uint32_t)instanceLayers.size())
		.setPpEnabledLayerNames(instanceLayers.data());

	instance = vk::createInstance(instanceInfo);
	Vulkan::dispatcher = new vk::DispatchLoaderDynamic(instance, vkGetInstanceProcAddr); //Instance dispatcher
	return true;
}

bool	VulkanRenderer::InitPhysicalDevice() {
	auto enumResult = instance.enumeratePhysicalDevices();

	if (enumResult.empty()) {
		return false; //Guess there's no Vulkan capable devices?!
	}

	gpu = enumResult[0];
	for (auto& i : enumResult) {
		if (i.getProperties().deviceType == vk::PhysicalDeviceType::eDiscreteGpu) {
			gpu = i; //Prefer a discrete GPU on multi device machines like laptops
		}
	}

	std::cout << __FUNCTION__ << " Vulkan using physical device " << gpu.getProperties().deviceName << std::endl;

	return true;
}

bool VulkanRenderer::InitGPUDevice() {
	InitSurface();
	InitDeviceQueueIndices();

	float queuePriority = 0.0f;
	vk::DeviceQueueCreateInfo queueInfo = vk::DeviceQueueCreateInfo()
		.setQueueCount(1)
		.setQueueFamilyIndex(gfxQueueIndex)
		.setPQueuePriorities(&queuePriority);

	vk::PhysicalDeviceFeatures features = vk::PhysicalDeviceFeatures()
		.setMultiDrawIndirect(true)
		.setDrawIndirectFirstInstance(true)
		.setShaderClipDistance(true)
		.setShaderCullDistance(true);

	vk::PhysicalDeviceFeatures2 deviceFeatures;
	deviceFeatures.setFeatures(features);

	vk::PhysicalDeviceDynamicRenderingFeaturesKHR dynamicRendering(true);
	deviceFeatures.pNext = &dynamicRendering;
	
	vk::DeviceCreateInfo createInfo = vk::DeviceCreateInfo()
		.setQueueCreateInfoCount(1)
		.setPQueueCreateInfos(&queueInfo)
		.setEnabledLayerCount((uint32_t)deviceLayers.size())
		.setPpEnabledLayerNames(deviceLayers.data())
		.setEnabledExtensionCount((uint32_t)deviceExtensions.size())
		.setPpEnabledExtensionNames(deviceExtensions.data());

	SetupDeviceInfo(createInfo);

	dynamicRendering.pNext = (void*)createInfo.pNext;
	createInfo.pNext = &deviceFeatures;

	device = gpu.createDevice(createInfo);

	gfxQueue		= device.getQueue(gfxQueueIndex, 0);
	computeQueue	= device.getQueue(computeQueueIndex, 0);
	copyQueue		= device.getQueue(copyQueueIndex, 0);
	presentQueue	= device.getQueue(gfxPresentIndex, 0);

	deviceMemoryProperties = gpu.getMemoryProperties();

	delete Vulkan::dispatcher;
	Vulkan::dispatcher = new vk::DispatchLoaderDynamic(instance, vkGetInstanceProcAddr , device); //DEVICE dispatcher
	return true;
}

bool VulkanRenderer::InitSurface() {
#ifdef _WIN32
	Win32Window* window = (Win32Window*)&hostWindow;

	vk::Win32SurfaceCreateInfoKHR createInfo;

	createInfo = vk::Win32SurfaceCreateInfoKHR(
		vk::Win32SurfaceCreateFlagsKHR(), window->GetInstance(), window->GetHandle());

	surface = instance.createWin32SurfaceKHR(createInfo);
#endif

	auto formats = gpu.getSurfaceFormatsKHR(surface);

	if (formats.size() == 1 && formats[0].format == vk::Format::eUndefined) {
		surfaceFormat	= vk::Format::eB8G8R8A8Unorm;
		surfaceSpace	= formats[0].colorSpace;
	}
	else {
		surfaceFormat	= formats[0].format;
		surfaceSpace	= formats[0].colorSpace;
	}

	return formats.size() > 0;
}

uint32_t VulkanRenderer::InitBufferChain(vk::CommandBuffer  cmdBuffer) {
	vk::SwapchainKHR oldChain					= swapChain;
	std::vector<SwapChain*> oldSwapChainList	= swapChainList;
	swapChainList.clear();

	vk::SurfaceCapabilitiesKHR surfaceCaps = gpu.getSurfaceCapabilitiesKHR(surface);

	vk::Extent2D swapExtents = vk::Extent2D((int)hostWindow.GetScreenSize().x, (int)hostWindow.GetScreenSize().y);

	auto presentModes = gpu.getSurfacePresentModesKHR(surface); //Type is of vector of PresentModeKHR

	vk::PresentModeKHR idealPresentMode = vk::PresentModeKHR::eFifo;

	for (const auto& i : presentModes) {
		if (i == vk::PresentModeKHR::eMailbox) {
			idealPresentMode = i;
			break;
		}
		else if (i == vk::PresentModeKHR::eImmediate) {
			idealPresentMode = vk::PresentModeKHR::eImmediate; //Might still become mailbox...
		}
	}

	vk::SurfaceTransformFlagBitsKHR idealTransform;

	if (surfaceCaps.supportedTransforms & vk::SurfaceTransformFlagBitsKHR::eIdentity) {
		idealTransform = vk::SurfaceTransformFlagBitsKHR::eIdentity;
	}
	else {
		idealTransform = surfaceCaps.currentTransform;
	}

	int idealImageCount = surfaceCaps.minImageCount + 1;
	if (surfaceCaps.maxImageCount > 0) {
		idealImageCount = std::min(idealImageCount, (int)surfaceCaps.maxImageCount);
	}

	vk::SwapchainCreateInfoKHR swapInfo;

	swapInfo.setPresentMode(idealPresentMode)
		.setPreTransform(idealTransform)
		.setSurface(surface)
		.setImageColorSpace(surfaceSpace)
		.setImageFormat(surfaceFormat)
		.setImageExtent(swapExtents)
		.setMinImageCount(idealImageCount)
		.setOldSwapchain(oldChain)
		.setImageArrayLayers(1)
		.setImageUsage(vk::ImageUsageFlagBits::eColorAttachment);

	swapChain = device.createSwapchainKHR(swapInfo);

	if (!oldSwapChainList.empty()) {
		for (unsigned int i = 0; i < numFrameBuffers; ++i) {
			device.destroyImageView(oldSwapChainList[i]->view);
			delete oldSwapChainList[i];
		}
	}
	if (oldChain) {
		device.destroySwapchainKHR(oldChain);
	}

	auto images = device.getSwapchainImagesKHR(swapChain);

	for (auto& i : images) {
		vk::ImageViewCreateInfo viewCreate = vk::ImageViewCreateInfo()
			.setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1))
			.setFormat(surfaceFormat)
			.setImage(i)
			.setViewType(vk::ImageViewType::e2D);

		SwapChain* chain = new SwapChain();

		chain->image = i;

		Vulkan::ImageTransitionBarrier(cmdBuffer, i, vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal, vk::ImageAspectFlagBits::eColor, vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::PipelineStageFlagBits::eColorAttachmentOutput);

		chain->view = device.createImageView(viewCreate);

		swapChainList.push_back(chain);

		auto buffers = device.allocateCommandBuffers(vk::CommandBufferAllocateInfo(
			commandPool, vk::CommandBufferLevel::ePrimary, 1));

		chain->frameCmds = buffers[0];
	}

	return (int)images.size();
}

void	VulkanRenderer::InitCommandPools() {
	computeCommandPool = device.createCommandPool(vk::CommandPoolCreateInfo(
		vk::CommandPoolCreateFlagBits::eResetCommandBuffer, computeQueueIndex));

	commandPool = device.createCommandPool(vk::CommandPoolCreateInfo(
		vk::CommandPoolCreateFlagBits::eResetCommandBuffer, gfxQueueIndex));
}

void	VulkanRenderer::InitMemoryAllocator() {
	VmaAllocatorCreateInfo allocatorInfo = {};
	allocatorInfo.physicalDevice = gpu;
	allocatorInfo.device = device;
	allocatorInfo.instance = instance;
	vmaCreateAllocator(&allocatorInfo, &memoryAllocator);
}

vk::CommandBuffer VulkanRenderer::BeginComputeCmdBuffer(const std::string& debugName) {
	vk::CommandBufferAllocateInfo bufferInfo = vk::CommandBufferAllocateInfo(computeCommandPool, vk::CommandBufferLevel::ePrimary, 1);

	auto buffers = device.allocateCommandBuffers(bufferInfo); //Func returns a vector!

	vk::CommandBuffer  newBuf = buffers[0];

	vk::CommandBufferBeginInfo beginInfo = vk::CommandBufferBeginInfo();

	if (!debugName.empty()) {
		Vulkan::SetDebugName(device, vk::ObjectType::eCommandBuffer, Vulkan::GetVulkanHandle(newBuf), debugName);
	}

	newBuf.begin(beginInfo);
	return newBuf;
}

vk::CommandBuffer VulkanRenderer::BeginCmdBuffer(const std::string& debugName) {
	vk::CommandBufferAllocateInfo bufferInfo = vk::CommandBufferAllocateInfo(commandPool, vk::CommandBufferLevel::ePrimary, 1);

	auto buffers = device.allocateCommandBuffers(bufferInfo); //Func returns a vector!

	vk::CommandBuffer &newBuf = buffers[0];

	vk::CommandBufferBeginInfo beginInfo = vk::CommandBufferBeginInfo();

	newBuf.begin(beginInfo);
	newBuf.setViewport(0, 1, &defaultViewport);
	newBuf.setScissor( 0, 1, &defaultScissor);

	if (!debugName.empty()) {
		Vulkan::SetDebugName(device, vk::ObjectType::eCommandBuffer, Vulkan::GetVulkanHandle(newBuf), debugName);
	}

	return newBuf;
}

void		VulkanRenderer::SubmitCmdBufferWait(vk::CommandBuffer  buffer) {
	vk::Fence fence = SubmitCmdBufferFence(buffer);

	if (!fence) {
		return;
	}

	if (device.waitForFences(1, &fence, true, UINT64_MAX) != vk::Result::eSuccess) {
		std::cout << __FUNCTION__ << " Device queue submission taking too long?\n";
	};

	device.destroyFence(fence);
}

void 	VulkanRenderer::SubmitCmdBuffer(vk::CommandBuffer  buffer) {
	if (buffer) {	
		buffer.end();
	}
	else {
		std::cout << __FUNCTION__ << " Submitting invalid buffer?\n";
		return;
	}
	vk::SubmitInfo submitInfo = vk::SubmitInfo();
	submitInfo.setCommandBufferCount(1);
	submitInfo.setPCommandBuffers(&buffer);

	gfxQueue.submit(submitInfo, {});
}

vk::Fence 	VulkanRenderer::SubmitCmdBufferFence(vk::CommandBuffer  buffer) {
	vk::Fence fence;
	if (buffer) {
		
		buffer.end();
	}
	else {
		std::cout << __FUNCTION__ << " Submitting invalid buffer?\n";
		return fence;
	}
	fence = device.createFence(vk::FenceCreateInfo());

	vk::SubmitInfo submitInfo = vk::SubmitInfo();
	submitInfo.setCommandBufferCount(1);
	submitInfo.setPCommandBuffers(&buffer);

	gfxQueue.submit(submitInfo, fence);

	return fence;
}

bool VulkanRenderer::InitDeviceQueueIndices() {
	deviceQueueProps = gpu.getQueueFamilyProperties();

	VkBool32 supportsPresent = false;
	gfxQueueIndex		= -1;
	computeQueueIndex	= -1;
	copyQueueIndex		= -1;
	gfxPresentIndex		= -1;

	for (unsigned int i = 0; i < deviceQueueProps.size(); ++i) {
		supportsPresent = gpu.getSurfaceSupportKHR(i, surface);

		if (computeQueueIndex == -1 && deviceQueueProps[i].queueFlags & vk::QueueFlagBits::eCompute) {
			computeQueueIndex = i;
		}
		if (copyQueueIndex == -1 && deviceQueueProps[i].queueFlags & vk::QueueFlagBits::eTransfer) {
			copyQueueIndex = i;
		}

		if (gfxQueueIndex == -1 && deviceQueueProps[i].queueFlags & vk::QueueFlagBits::eGraphics) {
			gfxQueueIndex = i;
			if (supportsPresent && gfxPresentIndex == -1) {
				gfxPresentIndex = i;
			}
		}
	}

	//if (gfxPresentIndex == -1) {
	//	for (unsigned int i = 0; i < deviceQueueProps.size(); ++i) {
	//		supportsPresent = gpu.getSurfaceSupportKHR(i, surface);
	//		if (supportsPresent) {
	//			gfxPresentIndex = i;
	//			break;
	//		}
	//	}
	//}

	if (gfxQueueIndex == -1		|| 
		gfxPresentIndex == -1   || 
		computeQueueIndex == -1 ||
		copyQueueIndex == -1) {
		return false;
	}

	return true;
}

void VulkanRenderer::OnWindowResize(int width, int height) {
	if (!hostWindow.IsMinimised() && width == windowWidth && height == windowHeight) {
		return;
	}
	if (width == 0 && height == 0) {
		return;
	}
	windowWidth		= width;
	windowHeight	= height;

	defaultScreenRect = vk::Rect2D({ 0,0 }, { (uint32_t)windowWidth, (uint32_t)windowHeight });

	defaultViewport = vk::Viewport(0.0f, (float)windowHeight, (float)windowWidth, -(float)windowHeight, 0.0f, 1.0f);
	defaultScissor	= vk::Rect2D(vk::Offset2D(0, 0), vk::Extent2D(windowWidth, windowHeight));

	defaultClearValues[0] = vk::ClearValue(vk::ClearColorValue(std::array<float, 4>{0.2f, 0.2f, 0.2f, 1.0f}));
	defaultClearValues[1] = vk::ClearValue(vk::ClearDepthStencilValue(1.0f, 0));

	vk::CommandBuffer cmds = BeginCmdBuffer();

	std::cout << __FUNCTION__ << " New dimensions: " << windowWidth << " , " << windowHeight << "\n";
	vkDeviceWaitIdle(device);

	//delete depthBuffer;
	depthBuffer = VulkanTexture::CreateDepthTexture(this,(int)hostWindow.GetScreenSize().x, (int)hostWindow.GetScreenSize().y);
	
	numFrameBuffers = InitBufferChain(cmds);

	InitDefaultRenderPass();
	CreateDefaultFrameBuffers();

	vkDeviceWaitIdle(device);

	vk::Semaphore	presentSempaphore = device.createSemaphore(vk::SemaphoreCreateInfo());
	vk::Fence		fence = device.createFence(vk::FenceCreateInfo());

	currentSwap = device.acquireNextImageKHR(swapChain, UINT64_MAX, presentSempaphore, vk::Fence()).value;	//Get swap image

	device.destroySemaphore(presentSempaphore);
	device.destroyFence(fence);

	CompleteResize();

	SubmitCmdBufferWait(cmds);
}

void VulkanRenderer::CompleteResize() {

}

void	VulkanRenderer::BeginFrame() {
	//if (hostWindow.IsMinimised()) {
	//	defaultCmdBuffer = BeginCmdBuffer();
	//}
	defaultCmdBuffer.reset({});

	defaultCmdBuffer.begin(vk::CommandBufferBeginInfo());
	defaultCmdBuffer.setViewport(0, 1, &defaultViewport);
	defaultCmdBuffer.setScissor(0, 1, &defaultScissor);
}

void	VulkanRenderer::EndFrame() {
	if (hostWindow.IsMinimised()) {
		SubmitCmdBufferWait(defaultCmdBuffer);
	}
	else {
		SubmitCmdBuffer(defaultCmdBuffer);
	}
}

void VulkanRenderer::SwapBuffers() {
	vk::UniqueSemaphore	presentSempaphore;
	vk::UniqueFence		presentFence;

	if (!hostWindow.IsMinimised()) {
		vk::CommandBuffer cmds = BeginCmdBuffer();
		TransitionSwapchainForPresenting(cmds);
		SubmitCmdBufferWait(cmds);
		device.freeCommandBuffers(commandPool, cmds);

		vk::Result presentResult = gfxQueue.presentKHR(vk::PresentInfoKHR(0, nullptr, 1, &swapChain, &currentSwap, nullptr));

		presentSempaphore = device.createSemaphoreUnique(vk::SemaphoreCreateInfo());
		presentFence	  = device.createFenceUnique(vk::FenceCreateInfo());
	
		currentSwap = device.acquireNextImageKHR(swapChain, UINT64_MAX, *presentSempaphore, *presentFence).value;	//Get swap image
	}
	defaultBeginInfo = vk::RenderPassBeginInfo()
		.setRenderPass(defaultRenderPass)
		.setFramebuffer(frameBuffers[currentSwap])
		.setRenderArea(defaultScissor)
		.setClearValueCount(sizeof(defaultClearValues) / sizeof(vk::ClearValue))
		.setPClearValues(defaultClearValues);

	defaultCmdBuffer = swapChainList[currentSwap]->frameCmds;

	if (!hostWindow.IsMinimised()) {
		vk::Result waitResult = device.waitForFences(*presentFence, true, ~0);
	}
}

void	VulkanRenderer::InitDefaultRenderPass() {
	if (defaultRenderPass) {
		device.destroyRenderPass(defaultRenderPass);
	}
	vk::AttachmentDescription attachments[] = {
		vk::AttachmentDescription()
			.setInitialLayout(vk::ImageLayout::eColorAttachmentOptimal)
			.setFinalLayout(vk::ImageLayout::eColorAttachmentOptimal)
			.setFormat(surfaceFormat)
			.setLoadOp(vk::AttachmentLoadOp::eClear)
			.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
			.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
	,
		vk::AttachmentDescription()
			.setInitialLayout(vk::ImageLayout::eUndefined)
			.setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal)
			.setFormat(depthBuffer->GetFormat())
			.setLoadOp(vk::AttachmentLoadOp::eClear)
			.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
			.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
	};

	vk::AttachmentReference references[] = {
		vk::AttachmentReference(0, vk::ImageLayout::eColorAttachmentOptimal),
		vk::AttachmentReference(1, vk::ImageLayout::eDepthStencilAttachmentOptimal)
	};

	vk::SubpassDescription subPass = vk::SubpassDescription()
		.setColorAttachmentCount(1)
		.setPColorAttachments(&references[0])
		.setPDepthStencilAttachment(&references[1])
		.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics);

	vk::RenderPassCreateInfo renderPassInfo = vk::RenderPassCreateInfo()
		.setAttachmentCount(2)
		.setPAttachments(attachments)
		.setSubpassCount(1)
		.setPSubpasses(&subPass);

	defaultRenderPass = device.createRenderPass(renderPassInfo);
}

bool VulkanRenderer::CreateDefaultFrameBuffers() {
	if (frameBuffers) {
		for (unsigned int i = 0; i < numFrameBuffers; ++i) {
			device.destroyFramebuffer(frameBuffers[i]);
		}
	}
	else {
		frameBuffers = new vk::Framebuffer[numFrameBuffers];
	}

	vk::ImageView attachments[2];
	
	vk::FramebufferCreateInfo createInfo = vk::FramebufferCreateInfo()
		.setWidth((int)hostWindow.GetScreenSize().x)
		.setHeight((int)hostWindow.GetScreenSize().y)
		.setLayers(1)
		.setAttachmentCount(2)
		.setPAttachments(attachments)
		.setRenderPass(defaultRenderPass);

	for (uint32_t i = 0; i < numFrameBuffers; ++i) {
		attachments[0]	= swapChainList[i]->view;
		attachments[1]	= *depthBuffer->defaultView;
		frameBuffers[i] = device.createFramebuffer(createInfo);
	}

	defaultBeginInfo = vk::RenderPassBeginInfo()
		.setRenderPass(defaultRenderPass)
		.setFramebuffer(frameBuffers[currentSwap])
		.setRenderArea(defaultScissor)
		.setClearValueCount(sizeof(defaultClearValues) / sizeof(vk::ClearValue))
		.setPClearValues(defaultClearValues);

	return true;
}

void	VulkanRenderer::InitDefaultDescriptorPool() {
	int maxSets = 128; //how many times can we ask the pool for a descriptor set?
	vk::DescriptorPoolSize poolSizes[] = {
		vk::DescriptorPoolSize(vk::DescriptorType::eUniformBuffer, 128),
		vk::DescriptorPoolSize(vk::DescriptorType::eCombinedImageSampler, 128),
		vk::DescriptorPoolSize(vk::DescriptorType::eStorageBuffer, 128)
	};

	vk::DescriptorPoolCreateInfo poolCreate;
	poolCreate.setPoolSizeCount(sizeof(poolSizes) / sizeof(vk::DescriptorPoolSize));
	poolCreate.setPPoolSizes(poolSizes);
	poolCreate.setMaxSets(maxSets);
	poolCreate.setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet);

	defaultDescriptorPool = device.createDescriptorPool(poolCreate);
}

void	VulkanRenderer::UpdateImageDescriptor(vk::DescriptorSet set, int bindingNum, int subIndex, vk::ImageView view, vk::Sampler sampler, vk::ImageLayout layout) {
	auto imageInfo = vk::DescriptorImageInfo()
		.setSampler(sampler)
		.setImageView(view)
		.setImageLayout(layout);

	auto descriptorWrite = vk::WriteDescriptorSet()
		.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
		.setDstSet(set)
		.setDstBinding(bindingNum)
		.setDstArrayElement(subIndex)
		.setDescriptorCount(1)
		.setPImageInfo(&imageInfo);

	device.updateDescriptorSets(1, &descriptorWrite, 0, nullptr);
}

void VulkanRenderer::UpdateBufferDescriptor(vk::DescriptorSet set, const VulkanBuffer& data, int bindingSlot, vk::DescriptorType bufferType) {
	auto descriptorInfo = vk::DescriptorBufferInfo()
		.setBuffer(data.buffer)
		.setOffset(0)
		.setRange(data.size);

	auto descriptorWrites = vk::WriteDescriptorSet()
		.setDescriptorType(bufferType)
		.setDstSet(set)
		.setDstBinding(bindingSlot)
		.setDescriptorCount(1)
		.setPBufferInfo(&descriptorInfo);

	device.updateDescriptorSets(1, &descriptorWrites, 0, nullptr);
}

VulkanBuffer VulkanRenderer::CreateBufferInternal(vk::BufferCreateInfo& vkInfo, VmaAllocationCreateInfo& vmaInfo) {
	VulkanBuffer buffer;
	buffer.size			= vkInfo.size;
	buffer.allocator	= memoryAllocator;

	vmaCreateBuffer(memoryAllocator, (VkBufferCreateInfo*)&vkInfo, &vmaInfo, (VkBuffer*)&(buffer.buffer), &buffer.allocationHandle, &buffer.allocationInfo);

	return buffer;
}

VulkanBuffer	VulkanRenderer::CreateStagingBuffer(size_t size) {
	vk::BufferUsageFlags	usage		= vk::BufferUsageFlagBits::eTransferSrc;
	vk::MemoryPropertyFlags properties	= vk::MemoryPropertyFlagBits::eHostVisible;

	vk::BufferCreateInfo bufferInfo(vk::BufferCreateFlags(), size, usage);
	VmaAllocationCreateInfo vmaallocInfo = {};	
	vmaallocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
	vmaallocInfo.usage = VMA_MEMORY_USAGE_AUTO;
	vmaallocInfo.requiredFlags = (VkMemoryPropertyFlags)properties;

	return CreateBufferInternal(bufferInfo, vmaallocInfo);
}

VulkanBuffer	VulkanRenderer::CreatePersistentBuffer(size_t size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags extraProperties) {
	vk::MemoryPropertyFlags properties = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent | extraProperties;

	vk::BufferCreateInfo bufferInfo(vk::BufferCreateFlags(), size, usage);
	VmaAllocationCreateInfo vmaallocInfo = {};
	vmaallocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;// | VMA_ALLOCATION_CREATE_MAPPED_BIT;
	vmaallocInfo.usage = VMA_MEMORY_USAGE_AUTO;
	vmaallocInfo.requiredFlags = (VkMemoryPropertyFlags)properties;

	return CreateBufferInternal(bufferInfo, vmaallocInfo);
}

VulkanBuffer VulkanRenderer::CreateBuffer(size_t size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties) {
	vk::BufferCreateInfo bufferInfo(vk::BufferCreateFlags(), size, usage);

	VmaAllocationCreateInfo vmaallocInfo = {};
	vmaallocInfo.usage = VMA_MEMORY_USAGE_AUTO;
	vmaallocInfo.requiredFlags = (VkMemoryPropertyFlags)properties;

	return CreateBufferInternal(bufferInfo, vmaallocInfo);
}

void VulkanRenderer::UploadBufferData(VulkanBuffer& uniform, void* data, int dataSize) {
	void* mappedData = device.mapMemory(uniform.allocationInfo.deviceMemory, uniform.allocationInfo.offset, uniform.allocationInfo.size);
	memcpy(mappedData, data, dataSize);
	device.unmapMemory(uniform.allocationInfo.deviceMemory);
}

vk::UniqueDescriptorSet VulkanRenderer::BuildUniqueDescriptorSet(vk::DescriptorSetLayout  layout, vk::DescriptorPool pool, uint32_t variableDescriptorCount) {
	if (!pool) {
		pool = defaultDescriptorPool;
	}
	vk::DescriptorSetAllocateInfo allocateInfo = vk::DescriptorSetAllocateInfo()
		.setDescriptorPool(pool)
		.setDescriptorSetCount(1)
		.setPSetLayouts(&layout);

	vk::DescriptorSetVariableDescriptorCountAllocateInfoEXT variableDescriptorInfo;

	if (variableDescriptorCount > 0) {
		variableDescriptorInfo.setDescriptorSetCount(1);
		variableDescriptorInfo.pDescriptorCounts = &variableDescriptorCount;
		allocateInfo.setPNext((const void*)&variableDescriptorInfo);
	}

	return std::move(device.allocateDescriptorSetsUnique(allocateInfo)[0]);
}

void VulkanRenderer::SubmitDrawCallLayer(const VulkanMesh& m, unsigned int layer, vk::CommandBuffer  to, int instanceCount) {
	VkDeviceSize baseOffset = 0;

	const SubMesh* sm = m.GetSubMesh(layer);

	m.BindToCommandBuffer(to);

	if (m.GetIndexCount() > 0) {
		to.drawIndexed(sm->count, instanceCount, sm->start, sm->base, 0);
	}
	else {
		to.draw(sm->count, instanceCount, sm->start, 0);
	}
}

void VulkanRenderer::SubmitDrawCall(const VulkanMesh& m, vk::CommandBuffer  to, int instanceCount) {
	VkDeviceSize baseOffset = 0;

	m.BindToCommandBuffer(to);

	if (m.GetIndexCount() > 0) {
		to.drawIndexed(m.GetIndexCount(), instanceCount, 0, 0, 0);
	}
	else {
		to.draw(m.GetVertexCount(), instanceCount, 0, 0);
	}
}

void	VulkanRenderer::BeginDefaultRenderPass(vk::CommandBuffer  cmds) {
	cmds.beginRenderPass(defaultBeginInfo, vk::SubpassContents::eInline);
	cmds.setViewport(0, 1, &defaultViewport);
	cmds.setScissor(0, 1, &defaultScissor);
}

void VulkanRenderer::TransitionSwapchainForRendering(vk::CommandBuffer buffer) {
	Vulkan::ImageTransitionBarrier(buffer, swapChainList[currentSwap]->image,
		vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal,
		vk::ImageAspectFlagBits::eColor, vk::PipelineStageFlagBits::eColorAttachmentOutput,
		vk::PipelineStageFlagBits::eColorAttachmentOutput);
}

void VulkanRenderer::TransitionSwapchainForPresenting(vk::CommandBuffer buffer) {
	Vulkan::ImageTransitionBarrier(buffer, swapChainList[currentSwap]->image,
		vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::ePresentSrcKHR,
		vk::ImageAspectFlagBits::eColor, vk::PipelineStageFlagBits::eAllCommands,
		vk::PipelineStageFlagBits::eBottomOfPipe);
}

void	VulkanRenderer::BeginDefaultRendering(vk::CommandBuffer  cmds) {
	vk::RenderingInfoKHR renderInfo;
	renderInfo.layerCount = 1;

	vk::RenderingAttachmentInfoKHR colourAttachment;
	colourAttachment.setImageView(swapChainList[currentSwap]->view)
		.setImageLayout(vk::ImageLayout::eColorAttachmentOptimal)
		.setLoadOp(vk::AttachmentLoadOp::eClear)
		.setClearValue(Vulkan::ClearColour(0.2f, 0.2f, 0.2f, 1.0f));

	vk::RenderingAttachmentInfoKHR depthAttachment;
	depthAttachment.setImageView(depthBuffer->GetDefaultView())
		.setImageLayout(vk::ImageLayout::eDepthAttachmentOptimal)
		.setLoadOp(vk::AttachmentLoadOp::eClear)
		.setStoreOp(vk::AttachmentStoreOp::eDontCare);
	depthAttachment.clearValue.setDepthStencil({ 1.0f, ~0U });

	renderInfo.setColorAttachments(colourAttachment)
		.setPDepthAttachment(&depthAttachment)
		.setPStencilAttachment(&depthAttachment);

	renderInfo.setRenderArea(defaultScreenRect);

	cmds.beginRendering(renderInfo, *NCL::Rendering::Vulkan::dispatcher);
}

void	VulkanRenderer::EndRendering(vk::CommandBuffer  cmds) {
	cmds.endRendering(*NCL::Rendering::Vulkan::dispatcher);
}