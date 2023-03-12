/******************************************************************************
This file is part of the Newcastle Vulkan Tutorial Series

Author:Rich Davison
Contact:richgdavison@gmail.com
License: MIT (see LICENSE file at the top of the source tree)
*//////////////////////////////////////////////////////////////////////////////
#pragma once
#include "../NCLCoreClasses/RendererBase.h"
#include "../NCLCoreClasses/Maths.h"

#include "VulkanPipeline.h"
#include "SmartTypes.h"
#include "vma/vk_mem_alloc.h"
using std::string;

namespace NCL::Rendering {
	class VulkanMesh;
	class VulkanShader;
	class VulkanCompute;
	class VulkanTexture;
	struct VulkanBuffer;

	class VulkanRenderer : public RendererBase {
		friend class VulkanMesh;
		friend class VulkanTexture;
	public:
		VulkanRenderer(Window& window);
		~VulkanRenderer();

		virtual bool Init();
		virtual bool HasInitialised() const { return device; }

	protected:
		void OnWindowResize(int w, int h)	override;
		void BeginFrame()		override;
		void EndFrame()			override;
		void SwapBuffers()		override;

		virtual void	CompleteResize();
		virtual void	InitDefaultRenderPass();
		virtual void	InitDefaultDescriptorPool();

		void SubmitDrawCall(vk::CommandBuffer  to, const VulkanMesh& m, int instanceCount = 1);
		void SubmitDrawCallLayer(const VulkanMesh& m, unsigned int layer, vk::CommandBuffer  to, int instanceCount = 1);

		vk::UniqueDescriptorSet BuildUniqueDescriptorSet(vk::DescriptorSetLayout  layout, vk::DescriptorPool pool = {}, uint32_t variableDescriptorCount = 0);

		void	UpdateBufferDescriptor(vk::DescriptorSet set, const VulkanBuffer& data, int bindingSlot, vk::DescriptorType bufferType);
		void	UpdateImageDescriptor(vk::DescriptorSet set, int bindingNum, int subIndex, vk::ImageView view, vk::Sampler sampler, vk::ImageLayout layout = vk::ImageLayout::eShaderReadOnlyOptimal);

		vk::CommandBuffer	BeginComputeCmdBuffer(const std::string& debugName = "");
		vk::CommandBuffer	BeginCmdBuffer(const std::string& debugName = "");

		void				SubmitCmdBufferWait(vk::CommandBuffer buffer);
		void				SubmitCmdBuffer(vk::CommandBuffer  buffer);
		vk::Fence 			SubmitCmdBufferFence(vk::CommandBuffer  buffer);

		void		BeginDefaultRenderPass(vk::CommandBuffer  cmds);

		void		BeginDefaultRendering(vk::CommandBuffer  cmds);
		void		EndRendering(vk::CommandBuffer  cmds);

		vk::Device GetDevice() const {
			return device;
		}

		VmaAllocator GetMemoryAllocator() const {
			return memoryAllocator;
		}

		vk::Queue GetGraphicsQueue() const {
			return gfxQueue;
		}

		vk::Queue GetCopyQueue() const {
			return copyQueue;
		}

		vk::Queue GetAsyncComputeQueue() const {
			return computeQueue;
		}


	protected:		
		struct SwapChain {
			vk::Image			image;
			vk::ImageView		view;
			vk::CommandBuffer	frameCmds;
		};
		std::vector<SwapChain*> swapChainList;	
		uint32_t				currentSwap		= 0;
		vk::Framebuffer* frameBuffers			= nullptr;

		vk::PipelineCache		pipelineCache;
		vk::Device				device;		//Device handle	
		
		vk::ClearValue			defaultClearValues[2];
		vk::Viewport			defaultViewport;
		vk::Rect2D				defaultScissor;	
		vk::Rect2D				defaultScreenRect;	
		vk::CommandBuffer		defaultCmdBuffer;
		vk::RenderPass			defaultRenderPass;
		vk::RenderPassBeginInfo defaultBeginInfo;
		
		vk::DescriptorPool		defaultDescriptorPool;	//descriptor sets come from here!
		vk::CommandPool			commandPool;			//Source Command Buffers from here
		vk::CommandPool			computeCommandPool;		//Source Command Buffers from here


		//Initialisation Info
		std::vector<const char*> deviceExtensions;
		std::vector<const char*> deviceLayers;
		std::vector<vk::QueueFamilyProperties> deviceQueueProps;

		std::vector<const char*>	instanceExtensions;
		std::vector<const char*>	instanceLayers;

		int majorVersion = 1;
		int minorVersion = 1;

		bool				autoTransitionFrameBuffer = true;
		bool				autoBeginDynamicRendering = true;

		vk::SurfaceKHR		surface;
		vk::Format			surfaceFormat;
		vk::ColorSpaceKHR	surfaceSpace;

		UniqueVulkanTexture depthBuffer;

		vk::SwapchainKHR	swapChain;
		VmaAllocator			memoryAllocator;
		VmaAllocatorCreateInfo	allocatorInfo;

		vk::PhysicalDevice GetPhysicalDevice() const {
			return gpu;
		}

	private: 
		void	InitCommandPools();
		bool	InitInstance();
		bool	InitPhysicalDevice();
		bool	InitGPUDevice();
		bool	InitSurface();
		void	InitMemoryAllocator();
		uint32_t	InitBufferChain(vk::CommandBuffer  cmdBuffer);

		bool	InitDeviceQueueIndices();
		bool	CreateDefaultFrameBuffers();

		virtual void SetupDevice(vk::PhysicalDeviceFeatures2& deviceFeatures) {}

		uint32_t			numFrameBuffers = 0;

		vk::Instance		instance;	//API Instance
		vk::PhysicalDevice	gpu;		//GPU in use

		vk::PhysicalDeviceProperties		deviceProperties;
		vk::PhysicalDeviceMemoryProperties	deviceMemoryProperties;

		vk::Queue			gfxQueue;
		vk::Queue			computeQueue;
		vk::Queue			copyQueue;
		vk::Queue			presentQueue;

		uint32_t			gfxQueueIndex			= 0;
		uint32_t			computeQueueIndex		= 0;
		uint32_t			copyQueueIndex			= 0;
		uint32_t			gfxPresentIndex			= 0;
	};
}