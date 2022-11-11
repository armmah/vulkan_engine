#include "pch.h"
#include "vk_types.h"
#include "EngineCore/VkMesh.h"
#include "Camera.h"
#include "VkTypes/PushConstantTypes.h"
#include "VkMesh.h"
#include "Mesh.h"
#include "VertexAttributes.h"
#include "IndexAttributes.h"
#include <Presentation/Device.h>
#include <VkTypes/VkMaterialVariant.h>
#include "PresentationTarget.h"
#include "Math/Frustum.h"
#include "Profiling/ProfileMarker.h"

CommandObjectsWrapper::RenderPassScope::RenderPassScope(VkCommandBuffer commandBuffer, VkRenderPass renderPass, 
	VkFramebuffer swapChainFramebuffer, VkExtent2D extent, bool hasColorAttachment, bool hasDepthAttachment)
{
	assert(hasColorAttachment || hasDepthAttachment);
	this->commandBuffer = commandBuffer;

	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = renderPass;
	renderPassInfo.framebuffer = swapChainFramebuffer;

	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = extent;

	std::array<VkClearValue, 2> clearValues{ };
	auto attachmentCount = 0;

	if (hasColorAttachment)
	{
		clearValues[attachmentCount] = { {0.0f, 0.0f, 0.0f, 1.0f} };
		attachmentCount += 1;
	}

	if (hasDepthAttachment)
	{
		clearValues[attachmentCount] = { 1.0f, 0 };
		attachmentCount += 1;
	}

	renderPassInfo.clearValueCount = attachmentCount;
	renderPassInfo.pClearValues = clearValues.data();

	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
}

CommandObjectsWrapper::RenderPassScope::~RenderPassScope()
{
	vkCmdEndRenderPass(commandBuffer);
}

CommandObjectsWrapper::CommandBufferScope::CommandBufferScope(VkCommandBuffer commandBuffer, VkCommandBufferUsageFlags flags)
{
	this->commandBuffer = commandBuffer;

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = flags;
	beginInfo.pInheritanceInfo = nullptr;

	if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
		throw std::runtime_error("Failed to begin command buffer");
}

CommandObjectsWrapper::CommandBufferScope::~CommandBufferScope()
{
	if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
		printf("Failed to record command buffer!\n");
}

void CommandObjectsWrapper::HelloTriangleCommand(VkCommandBuffer buffer, VkPipeline m_pipeline, VkRenderPass m_renderPass, VkFramebuffer frameBuffer, VkExtent2D extent, VkBuffer vertexBuffer, uint32_t size)
{
	auto cbs = CommandBufferScope(buffer);
	{
		auto rps = RenderPassScope(buffer, m_renderPass, frameBuffer, extent, true, false);

		vkCmdBindPipeline(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);

		if (vertexBuffer != nullptr)
		{
			VkBuffer vertexBuffers[] = { vertexBuffer };
			const VkDeviceSize offsets[] = { 0 };
			vkCmdBindVertexBuffers(buffer, 0, 1, vertexBuffers, offsets);
		}

		vkCmdDraw(buffer, size, 1, 0, 0);
	}
}

namespace vkinit
{
	bool createDescriptorSets(std::array<VkDescriptorSet, SWAPCHAIN_IMAGE_COUNT>& descriptorSets, VkDevice device, VkDescriptorPool descriptorPool, VkDescriptorSetLayout descriptorSetLayout);
	void updateDescriptorSets(std::array<VkDescriptorSet, SWAPCHAIN_IMAGE_COUNT>& descriptorSets, VkDevice device, BuffersUBO ubo, uint32_t binding);
}
