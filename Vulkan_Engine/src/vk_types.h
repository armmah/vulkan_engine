#pragma once

#include "EngineCore/pch.h"
#include "Common.h"
#include "vulkan/vulkan.h"
#include "Presentation/Device.h"
#include "VkTypes/VkMaterialVariant.h"

class Camera;
struct VkMesh;

struct CommandObjectsWrapper
{
	class CommandBufferScope
	{
		VkCommandBuffer commandBuffer;

	public:
		CommandBufferScope(VkCommandBuffer commandBuffer, VkCommandBufferUsageFlags flags = 0);
		~CommandBufferScope();

		CommandBufferScope(const CommandBufferScope&) = delete;
		CommandBufferScope& operator=(const CommandBufferScope&) = delete;
	};

	class RenderPassScope
	{
		VkCommandBuffer commandBuffer;

	public:
		RenderPassScope(VkCommandBuffer commandBuffer, VkRenderPass m_renderPass, VkFramebuffer swapChainFramebuffer, VkExtent2D extent, bool hasDepthAttachment);
		~RenderPassScope();

		RenderPassScope(const RenderPassScope&) = delete;
		RenderPassScope& operator=(const RenderPassScope&) = delete;
	};

	static void HelloTriangleCommand(VkCommandBuffer buffer, VkPipeline m_pipeline, VkRenderPass m_renderPass, VkFramebuffer frameBuffer, VkExtent2D extent, VkBuffer vertexBuffer, uint32_t size);

	static void drawAt(VkCommandBuffer commandBuffer, const VkMesh& mesh, VkPipelineLayout layout,
		const Camera& cam, uint32_t frameNumber, float freq, glm::vec3 pos);

	static void renderIndexedMeshes(VkCommandBuffer commandBuffer, VkRenderPass m_renderPass, 
		VkFramebuffer frameBuffer, VkExtent2D extent, Camera& cam, const std::vector<UNQ<VkMesh>>& meshes, 
		const VkMaterialVariant& variant, uint32_t frameNumber);
};