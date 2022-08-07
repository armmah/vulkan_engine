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
#include "PresentationTarget.h"

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
	const glm::mat4 model = glm::translate(glm::mat4(1.f), pos);// *glm::rotate(glm::mat4{ 1.0f }, glm::radians(frameNumber * 0.01f * freq), glm::vec3(0, 1, 0));

	TransformPushConstant pushConstant{};
	pushConstant.model_matrix = model;
	pushConstant.view_matrix = cam.getViewMatrix();
	pushConstant.persp_matrix = cam.getPerspectiveMatrix();

	mesh.vAttributes->bind(commandBuffer);
	mesh.iAttributes->bind(commandBuffer);

	vkCmdPushConstants(commandBuffer, layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(TransformPushConstant), &pushConstant);
	vkCmdDrawIndexed(commandBuffer, mesh.iCount, 1, 0, 0, 0);
}

struct StateChangeStatistics
{
	uint32_t pipelineCount;
	uint32_t descriptorSetCount;
	uint32_t drawCallCount;

	void print() const
	{
		printf("\t\t\tDraw calls = %i. State change (pipeline = %i) | (descriptors = %i).\n", drawCallCount, pipelineCount, descriptorSetCount);
	}
};

void CommandObjectsWrapper::renderIndexedMeshes(const std::vector<MeshRenderer>& renderers, Camera& cam, 
	VkCommandBuffer commandBuffer, VkRenderPass m_renderPass, VkFramebuffer frameBuffer, VkExtent2D extent, uint32_t frameNumber)
{
	auto cbs = CommandBufferScope(commandBuffer);
	{
		cam.updateWindowExtent(extent);

		vkCmdSetViewport(commandBuffer, 0, 1, &cam.getViewport());
		vkCmdSetScissor(commandBuffer, 0, 1, &cam.getScissorRect());

		auto rps = RenderPassScope(commandBuffer, m_renderPass, frameBuffer, extent, true);

		StateChangeStatistics stats{};
		const VkMaterialVariant* prevVariant = nullptr;
		// To do - Add sorting to minimize state change
		// To do - Frustrum culling
		for (auto & renderer : renderers)
		{
			const auto& variant = *renderer.variant;
			if (prevVariant != renderer.variant)
			{
				auto stateChange = variant.compare(prevVariant);

				if (bitFlagPresent(stateChange, VariantStateChange::Pipeline))
				{
					vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, variant.getPipeline());
					stats.pipelineCount += 1;
				}

				{
					vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, variant.getPipelineLayout(), 0, 1, variant.getDescriptorSet(frameNumber), 0, nullptr);
					stats.descriptorSetCount += 1;
				}

				prevVariant = &variant;
			}

			drawAt(commandBuffer, *renderer.mesh, variant.getPipelineLayout(), cam, frameNumber, 10.0f, glm::vec3(0.0f, 0.0f, 0.0f));
			stats.drawCallCount += 1;
		}
		//stats.print();

		if (ImGui::GetDrawData())
		{
			ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
		}
	}
}
