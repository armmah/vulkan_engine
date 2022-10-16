#include "pch.h"
#include "PresentationTarget.h"
#include "vk_types.h"
#include "VkTypes/PushConstantTypes.h"
#include "Mesh.h"
#include "VkMesh.h"
#include "VertexAttributes.h"
#include "PipelineBinding.h"

#include "Camera.h"
#include "Math/Frustum.h"

#include "Profiling/ProfileMarker.h"

namespace Presentation
{
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
}