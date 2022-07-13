#include "pch.h"
#include "vk_types.h"
#include "EngineCore/VkMesh.h"
#include "Camera.h"
#include "VkTypes/PushConstantTypes.h"
#include "VkMesh.h"
#include "VertexAttributes.h"
#include "IndexAttributes.h"
#include <Presentation/Device.h>
#include <VkTypes/VkMaterialVariant.h>

CommandObjectsWrapper::RenderPassScope::RenderPassScope(VkCommandBuffer commandBuffer, VkRenderPass m_renderPass, VkFramebuffer swapChainFramebuffer, VkExtent2D extent, bool hasDepthAttachment)
{
	this->commandBuffer = commandBuffer;

	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = m_renderPass;
	renderPassInfo.framebuffer = swapChainFramebuffer;

	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = extent;

	std::array<VkClearValue, 2> clearValues{};
	clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
	clearValues[1].depthStencil = { 1.0f, 0 };

	renderPassInfo.clearValueCount = hasDepthAttachment ? 2u : 1u;
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
		auto rps = RenderPassScope(buffer, m_renderPass, frameBuffer, extent, false);

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

void CommandObjectsWrapper::drawAt(VkCommandBuffer commandBuffer, const VkMesh& mesh, VkPipelineLayout layout, const Camera& cam, uint32_t frameNumber, float freq, glm::vec3 pos)
{
	const glm::mat4 model =
		glm::translate(glm::mat4(1.f), pos) *
		glm::rotate(glm::mat4{ 1.0f }, glm::radians(frameNumber * 0.01f * freq), glm::vec3(0, 1, 0));

	TransformPushConstant pushConstant{};
	pushConstant.mvp_matrix = cam.getViewProjectionMatrix() * model;

	mesh.vAttributes->bind(commandBuffer);
	mesh.iAttributes->bind(commandBuffer);

	vkCmdPushConstants(commandBuffer, layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(TransformPushConstant), &pushConstant);
	vkCmdDrawIndexed(commandBuffer, mesh.iCount, 1, 0, 0, 0);
}

void CommandObjectsWrapper::renderIndexedMeshes(VkCommandBuffer commandBuffer, VkRenderPass m_renderPass,
	VkFramebuffer frameBuffer, VkExtent2D extent, Camera& cam, const std::vector<UNQ<VkMesh>>& meshes, const VkMaterialVariant& variant, uint32_t frameNumber)
{
	auto cbs = CommandBufferScope(commandBuffer);
	{
		cam.updateWindowExtent(extent);

		vkCmdSetViewport(commandBuffer, 0, 1, &cam.getViewport());
		vkCmdSetScissor(commandBuffer, 0, 1, &cam.getScissorRect());

		auto rps = RenderPassScope(commandBuffer, m_renderPass, frameBuffer, extent, true);

		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, variant.pipeline);

		for (int i = 0; i < meshes.size(); i++)
		{
			vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, variant.pipelineLayout, 0, 1, &variant.descriptorSets[frameNumber % SWAPCHAIN_IMAGE_COUNT], 0, nullptr);
			drawAt(commandBuffer, *meshes[i], variant.pipelineLayout, cam, frameNumber, 10.0f, glm::vec3(0.0f, 0.0f, 0.0f));
		}

		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
	}
}
