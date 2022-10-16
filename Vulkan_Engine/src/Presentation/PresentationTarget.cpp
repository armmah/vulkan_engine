#include "pch.h"
#include "Presentation/Device.h"
#include "Presentation/HardwareDevice.h"
#include "VkTypes/VkShader.h"
#include "VkTypes/InitializersUtility.h"
#include "VertexBinding.h"
#include "VkTypes/PushConstantTypes.h"
#include "Engine/Window.h"
#include "VkTypes/VkTexture.h"
#include "VkTypes/VkMaterialVariant.h"
#include "Material.h"
#include "PresentationTarget.h"
#include "Mesh.h"

#include "Camera.h"
#include "vk_types.h"
#include "Profiling/ProfileMarker.h"
#include "Math/Frustum.h"
#include "VkMesh.h"
#include "EngineCore/VertexAttributes.h"
#include "EngineCore/PipelineBinding.h"
#include "BuffersUBO.h"
#include "DescriptorPoolManager.h"

namespace Presentation
{
	bool PresentationTarget::createGraphicsPipeline(VkPipeline& pipeline, const VkPipelineLayout layout, const VkShader& shader, VkDevice device, const VertexBinding& vBinding, VkCullModeFlagBits faceCullingMode, bool depthStencilAttachement) const
	{
		vBinding.runValidations();
		auto vertexInputInfo = vBinding.getVertexInputCreateInfo();

		VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = shader.vertShader;
		vertShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = shader.fragShader;
		fragShaderStageInfo.pName = "main";

		std::vector<VkPipelineShaderStageCreateInfo> shaderStages = { vertShaderStageInfo, fragShaderStageInfo };

		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		VkViewport viewport{};
		VkRect2D scissor{};
		vkinit::Commands::initViewportAndScissor(viewport, scissor, m_swapChainExtent);

		VkPipelineViewportStateCreateInfo viewportState{};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.pViewports = &viewport;
		viewportState.scissorCount = 1;
		viewportState.pScissors = &scissor;

		VkPipelineRasterizationStateCreateInfo rasterizer{};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = faceCullingMode;
		rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		
		rasterizer.depthBiasEnable = VK_FALSE;
		rasterizer.depthBiasConstantFactor = 0.0f;
		rasterizer.depthBiasClamp = 0.0f;
		rasterizer.depthBiasSlopeFactor = 0.0f;

		VkPipelineMultisampleStateCreateInfo multisampling{};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisampling.minSampleShading = 1.0f; // Optional
		multisampling.pSampleMask = nullptr; // Optional
		multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
		multisampling.alphaToOneEnable = VK_FALSE; // Optional

		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;
		colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
		colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
		colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

		VkPipelineColorBlendStateCreateInfo colorBlending{};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY;
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;

		VkPipelineDepthStencilStateCreateInfo depthStencil{};
		if (depthStencilAttachement)
		{
			depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;

			depthStencil.depthTestEnable = VK_TRUE;
			depthStencil.depthWriteEnable = VK_TRUE;
			depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;

			depthStencil.depthBoundsTestEnable = VK_FALSE;
			depthStencil.minDepthBounds = 0.0f;
			depthStencil.maxDepthBounds = 1.0f;

			depthStencil.stencilTestEnable = VK_FALSE;
			depthStencil.front = {};
			depthStencil.back = {};
		}

		// Creating graphics pipeline with its creation descriptor struct
		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;

		// Programmable shader stages
		pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
		pipelineInfo.pStages = shaderStages.data();

		// Fixed function stage
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pDepthStencilState = depthStencilAttachement ? &depthStencil : nullptr;
		pipelineInfo.pColorBlendState = &colorBlending;

		// Dynamic state - currently hardcoding for viewport and scissor rect, to be able to easily recreate the swapchain.
		VkPipelineDynamicStateCreateInfo dynamicStateInfo{};
		dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		VkDynamicState dynamicStates[2] = { 
			VkDynamicState::VK_DYNAMIC_STATE_VIEWPORT, 
			VkDynamicState::VK_DYNAMIC_STATE_SCISSOR 
		};
		dynamicStateInfo.dynamicStateCount = 2;
		dynamicStateInfo.pDynamicStates = dynamicStates;
		pipelineInfo.pDynamicState = &dynamicStateInfo;

		pipelineInfo.layout = layout;

		pipelineInfo.renderPass = m_renderPass;
		pipelineInfo.subpass = 0;

		// Parent pipeline
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
		pipelineInfo.basePipelineIndex = -1;

		if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create graphics pipeline!");
		}

		return true;
	}

	void drawAt(VkCommandBuffer commandBuffer, const VkMeshRenderer& renderer, const Camera& cam, const glm::mat4& model)
	{
		if (renderer.submeshIndex >= renderer.mesh->iAttributes.size())
			return;

		TransformPushConstant pushConstant{};
		pushConstant.model_matrix = model;

		renderer.mesh->vAttributes->bind(commandBuffer);

		auto& indices = renderer.mesh->iAttributes[renderer.submeshIndex];
		{
			indices.bind(commandBuffer);

			vkCmdPushConstants(commandBuffer, renderer.variant->getPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(TransformPushConstant), &pushConstant);
			vkCmdDrawIndexed(commandBuffer, indices.getIndexCount(), 1, 0, 0, 0);
		}
	}

	FrameStats PresentationTarget::renderLoop(const std::vector<VkMeshRenderer>& renderers, Camera& cam, VkCommandBuffer commandBuffer, uint32_t frameNumber)
	{
		FrameStats stats{};
		const auto injectionMarker = ProfileMarkerInjectResult(stats.renderLoop_ms);

		auto cbs = CommandObjectsWrapper::CommandBufferScope(commandBuffer);
		{
			const auto extent = getSwapchainExtent();
			cam.updateWindowExtent(extent);

			vkCmdSetViewport(commandBuffer, 0, 1, &cam.getViewport());
			vkCmdSetScissor(commandBuffer, 0, 1, &cam.getScissorRect());

			auto rps = CommandObjectsWrapper::RenderPassScope(commandBuffer, m_renderPass, getSwapchainFrameBuffers(frameNumber), extent, true);

			renderIndexedMeshes(stats, renderers, cam, commandBuffer, frameNumber);

			if (ImGui::GetDrawData())
			{
				ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
			}
		}

		stats.frameNumber = frameNumber;
		return stats;
	}

	void PresentationTarget::renderIndexedMeshes(FrameStats& stats, const std::vector<VkMeshRenderer>& renderers, const Camera& cam, VkCommandBuffer commandBuffer, uint32_t frameNumber)
	{
		const auto handleConstantsUBO = m_globalPipelineState->fillGlobalConstantsUBO(frameNumber);
		const auto handleViewUBO = m_globalPipelineState->fillCameraUBO(frameNumber, cam);

		auto pipelineLayout = m_globalPipelineState->getPipelineLayout();

		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
			pipelineLayout, 0, 1, &handleConstantsUBO.descriptorSet, 0, nullptr);
		stats.descriptorSetCount += 1;

		// For each camera
		{
			vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
				pipelineLayout, 1, 1, &handleViewUBO.descriptorSet, 0, nullptr);
			stats.descriptorSetCount += 1;

			const auto& cameraFrustum = Frustum(cam);
			const VkMaterialVariant* prevVariant = nullptr;
			// To do - Add sorting to minimize state change
			for (auto& renderer : renderers)
			{
				glm::mat4 model = renderer.transform->localToWorld;
				// To do - transform the AABB from local to world space, before doing frustum check
				if (renderer.bounds != nullptr)
				{
					const auto b = renderer.bounds->getTransformed(model);

					if (!cameraFrustum.isOnFrustum(b))
						continue;
				}

				const auto& variant = *renderer.variant;
				if (prevVariant != renderer.variant)
				{
					auto stateChange = variant.compare(prevVariant);

					if (bitFlagPresent(stateChange, VariantStateChange::Pipeline))
					{
						vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, variant.getPipeline());
						stats.pipelineCount += 1;
					}

					// if (bitFlagPresent(stateChange, VariantStateChange::DescriptorSet))
					{
						vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, variant.getPipelineLayout(), 2, 1, variant.getDescriptorSet(frameNumber), 0, nullptr);
						stats.descriptorSetCount += 1;
					}

					prevVariant = &variant;
				}

				drawAt(commandBuffer, renderer, cam, model);
				stats.drawCallCount += 1;
			}
		}
	}

	PresentationTarget::PresentationTarget(const HardwareDevice& presentationHardware, const Device& presentationDevice, Window const* wnd, bool depthAttachment, uint32_t swapchainCount)
		: m_window(wnd), m_hasDepthAttachment(depthAttachment)
	{
		m_isInitialized = createPresentationTarget(presentationHardware, presentationDevice, swapchainCount);
	}
	PresentationTarget::~PresentationTarget() {}

	bool PresentationTarget::hasDepthAttachement() { return m_depthImage ? true : false; }

	bool PresentationTarget::createPresentationTarget(const HardwareDevice& presentationHardware, const Device& presentationDevice, uint32_t swapchainCount)
	{
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(presentationHardware.getActiveGPU(), presentationDevice.getSurface(), &m_capabilities);
		m_swapChainExtent = chooseSwapExtent(m_window->get());

		auto vkdevice = presentationDevice.getDevice();

		return createSwapChain(swapchainCount, presentationHardware, presentationDevice, m_hasDepthAttachment) &&
			createSwapChainImageViews(vkdevice) &&
			createRenderPass(vkdevice) &&
			createFramebuffers(vkdevice) &&
			tryInitialize(m_globalPipelineState, vkdevice);
	}

	bool PresentationTarget::createGraphicsMaterial(UNQ<VkMaterial>& material, VkDevice device, VkDescriptorPool descPool, const VkShader* shader, const VkTexture2D* texture)
	{
		VkDescriptorSetLayout descriptorSetLayout = m_globalPipelineState->getDescriptorSetLayout(PipelineDescriptor::BindingSlots::Textures);

		std::array<VkDescriptorSet, SWAPCHAIN_IMAGE_COUNT> descriptorSets;
		vkinit::Descriptor::createDescriptorSets(descriptorSets, device, descPool, descriptorSetLayout, *texture);
		
		VkGraphicsPipeline graphicsPipeline;
		if (!m_globalPipelineState->tryGetGraphicsPipelineFor(shader, graphicsPipeline))
		{
			graphicsPipeline.pipelineLayout = m_globalPipelineState->getPipelineLayout();
			if (!createGraphicsPipeline(graphicsPipeline.pipeline, graphicsPipeline.pipelineLayout, *shader, device, Mesh::defaultVertexBinding, VK_CULL_MODE_BACK_BIT, m_hasDepthAttachment))
				return false;

			m_globalPipelineState->insertGraphicsPipelineFor(shader, graphicsPipeline);
		}
		
		material = MAKEUNQ<VkMaterial>(*shader, *texture,
			graphicsPipeline.pipeline, graphicsPipeline.pipelineLayout,
			descriptorSetLayout, descriptorSets);

		return true;
	}

	VkExtent2D PresentationTarget::chooseSwapExtent(const SDL_Window* window)
	{
		if (m_capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
		{
			return m_capabilities.currentExtent;
		}
		else
		{
			int width, height;
			SDL_Vulkan_GetDrawableSize(const_cast<SDL_Window*>(window), &width, &height);

			VkExtent2D actualExtent = {
				static_cast<uint32_t>(width),
				static_cast<uint32_t>(height)
			};

			actualExtent.width = std::clamp(actualExtent.width, m_capabilities.minImageExtent.width, m_capabilities.maxImageExtent.width);
			actualExtent.height = std::clamp(actualExtent.height, m_capabilities.minImageExtent.height, m_capabilities.maxImageExtent.height);

			return actualExtent;
		}
	}

	bool PresentationTarget::createSwapChain(uint32_t imageCount, const HardwareDevice& hardware, const Device& device, bool createDepthAttachement)
	{
		auto surfaceFormat = hardware.chooseSwapSurfaceFormat();
		auto presentationMode = hardware.chooseSwapPresentMode();
		auto extent = chooseSwapExtent(m_window->get());

		imageCount = std::max(imageCount, m_capabilities.minImageCount);
		if (m_capabilities.maxImageCount != 0)
			imageCount = std::min(imageCount, m_capabilities.maxImageCount);

		bool isSuccess = vkinit::Texture::createSwapchain(m_swapchain, device.getDevice(), device.getSurface(),
			imageCount, extent, presentationMode, surfaceFormat, m_capabilities.currentTransform);

		if (createDepthAttachement)
		{
			auto maci = VkMemoryAllocator::getInstance()->createAllocationDescriptor(VMA_MEMORY_USAGE_GPU_ONLY, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
			m_depthImage = MAKEUNQ<VkTexture>(device.getDevice(), maci, VK_FORMAT_D32_SFLOAT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_ASPECT_DEPTH_BIT, extent);
		}
		else m_depthImage = nullptr;

		if (isSuccess)
		{
			m_swapChainExtent = extent;
			m_swapChainImageFormat = surfaceFormat.format;

			vkGetSwapchainImagesKHR(device.getDevice(), m_swapchain, &imageCount, nullptr);
			m_swapChainImages.resize(imageCount);
			vkGetSwapchainImagesKHR(device.getDevice(), m_swapchain, &imageCount, m_swapChainImages.data());
		}

		return isSuccess;
	}

	bool PresentationTarget::createRenderPass(VkDevice device)
	{
		auto attachementCount = 1;

		VkPipelineStageFlags srcStageBit = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		VkPipelineStageFlags dstStageBit = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		VkAccessFlags dstAccessBit = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = attachementCount;

		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = m_swapChainImageFormat;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		subpass.pColorAttachments = &colorAttachmentRef;

		VkAttachmentDescription depthAttachment{};
		if (hasDepthAttachement())
		{
			depthAttachment.format = VK_FORMAT_D32_SFLOAT;
			depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
			depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

			VkAttachmentReference depthAttachmentRef{};
			depthAttachmentRef.attachment = 1;
			depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

			subpass.pDepthStencilAttachment = &depthAttachmentRef;

			srcStageBit |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			dstStageBit |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			dstAccessBit |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

			attachementCount += 1;
		}

		std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };
		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = attachementCount;
		renderPassInfo.pAttachments = attachments.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;

		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = srcStageBit;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = dstStageBit;
		dependency.dstAccessMask = dstAccessBit;

		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		return vkCreateRenderPass(device, &renderPassInfo, nullptr, &m_renderPass) == VK_SUCCESS;
	}

	bool PresentationTarget::createSwapChainImageViews(VkDevice device)
	{
		auto imageCount = m_swapChainImages.size();
		m_swapChainImageViews.resize(imageCount);

		for (int i = 0; i < imageCount; i++)
		{
			if (!vkinit::Texture::createTextureImageView(m_swapChainImageViews[i], device, m_swapChainImages[i],VK_FORMAT_B8G8R8A8_SRGB, 1u))
				return false;
		}

		return true;
	}

	bool PresentationTarget::createFramebuffers(VkDevice device)
	{
		auto imageViewSize = m_swapChainImageViews.size();
		m_swapChainFrameBuffers.resize(imageViewSize);

		std::array<VkImageView, 2> attachments;
		auto attachmentCount = 1;
		if (hasDepthAttachement())
		{
			attachments[1] = m_depthImage->imageView;
			attachmentCount += 1;
		}

		for (size_t i = 0; i < imageViewSize; i++)
		{
			attachments[0] = m_swapChainImageViews[i];

			VkFramebufferCreateInfo framebufferInfo{};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = m_renderPass;
			framebufferInfo.attachmentCount = attachmentCount;
			framebufferInfo.pAttachments = attachments.data();
			framebufferInfo.width = m_swapChainExtent.width;
			framebufferInfo.height = m_swapChainExtent.height;
			framebufferInfo.layers = 1;

			if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &m_swapChainFrameBuffers[i]) != VK_SUCCESS)
				return false;
		}

		return true;
	}

	void PresentationTarget::releaseSwapChain(VkDevice device)
	{
		vkDestroyRenderPass(device, m_renderPass, nullptr);

		if (hasDepthAttachement())
		{
			m_depthImage->release(device);
		}

		int count = static_cast<uint32_t>(m_swapChainImageViews.size());
		assert(m_swapChainImageViews.size() == m_swapChainFrameBuffers.size() && "Frame buffer count should be equal to image view count.");
		for (int i = 0; i < count; i++)
		{
			vkDestroyFramebuffer(device, m_swapChainFrameBuffers[i], nullptr);
			vkDestroyImageView(device, m_swapChainImageViews[i], nullptr);
		}

		vkDestroySwapchainKHR(device, m_swapchain, nullptr);
	}

	void PresentationTarget::releaseAllResources(VkDevice device)
	{
		m_globalPipelineState->release(device);
		releaseSwapChain(device);
	}
}