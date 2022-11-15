#include "pch.h"
#include "PresentationTarget.h"
#include "vk_types.h"
#include "VkTypes/InitializersUtility.h"
#include "VkTypes/PushConstantTypes.h"
#include "VkMesh.h"
#include "VertexAttributes.h"
#include "PipelineBinding.h"

#include "Mesh.h"
#include "Camera.h"
#include "Texture.h"
#include "Material.h"
#include "Math/Frustum.h"

#include "Profiling/ProfileMarker.h"

namespace Presentation
{
	void drawAt(VkCommandBuffer commandBuffer, const VkMeshRenderer& renderer, const glm::mat4& model)
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

	FrameStats PresentationTarget::renderLoop(const std::vector<VkMeshRenderer>& renderers, Camera& cam, DirectionalLightParams& lightTr, VkCommandBuffer commandBuffer, uint32_t frameNumber)
	{
		FrameStats stats{};
		const auto injectionMarker = ProfileMarkerInjectResult(stats.renderLoop_ms);

		auto cbs = CommandObjectsWrapper::CommandBufferScope(commandBuffer);
		{
			m_globalPipelineState->StartFrame(frameNumber);

			const auto pipelineLayout = m_globalPipelineState->getForwardPipelineLayout();

			VkExtent2D extent{};
			extent.width = 45u;
			extent.height = 45u;
			auto lightCam = Camera(extent);
			lightCam.centerAround(lightTr.pitch, lightTr.yaw, lightTr.distance);
			const auto lightViewUBO = m_globalPipelineState->fillCameraUBO(lightCam);

			const auto handleConstantsUBO = m_globalPipelineState->fillGlobalConstantsUBO(lightCam.getViewProjectionMatrix(), lightTr.getBiasAmbient());

			vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
				pipelineLayout, PipelineDescriptor::BindingSlots::Constants, 1, &handleConstantsUBO.descriptorSet, 0, nullptr);
			stats.descriptorSetCount += 1;

			renderIndexedMeshes(stats, renderers, cam, lightViewUBO, commandBuffer, frameNumber);
		}

		stats.frameNumber = frameNumber;
		return stats;
	}

	void PresentationTarget::renderIndexedMeshes(FrameStats& stats, const std::vector<VkMeshRenderer>& renderers, Camera& cam, const BufferHandle& lightViewUBO, VkCommandBuffer commandBuffer, uint32_t frameNumber)
	{
		const auto pipelineLayout = m_globalPipelineState->getForwardPipelineLayout();

		std::vector<VkMeshRenderer> sortedList(renderers.begin(), renderers.end());

		// ShadowMap - pass
		if(m_shadowMapModule && m_shadowMapModule->getActive())
		{
			auto scopeShadowMapRenderPass = CommandObjectsWrapper::RenderPassScope(commandBuffer, m_shadowMapModule->getRenderPass(), 
				m_shadowMapModule->getFrameBuffer(frameNumber), m_shadowMapModule->getExtent(), false, true);

			vkCmdSetViewport(commandBuffer, 0, 1, &m_shadowMapModule->getViewport());
			vkCmdSetScissor(commandBuffer, 0, 1, &m_shadowMapModule->getScissorRect());

			vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
				pipelineLayout, PipelineDescriptor::BindingSlots::View, 1, &lightViewUBO.descriptorSet, 0, nullptr);
			stats.descriptorSetCount += 1;

			const auto depthOnly = m_shadowMapModule->m_replacementMaterial;
			vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, depthOnly.m_pipeline);
			stats.pipelineCount += 1;

			for (auto& renderer : sortedList)
			{
				drawAt(commandBuffer, renderer, renderer.transform->localToWorld);
				stats.drawCallCount += 1;
			}
		}

		const auto& variant = m_debugModule->getMaterialVariant();
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, variant.getPipelineLayout(), 
			PipelineDescriptor::BindingSlots::Shadowmap, 1, variant.getDescriptorSet(frameNumber), 0, nullptr);
		stats.descriptorSetCount += 1;

		auto scopeForwardRenderPass = CommandObjectsWrapper::RenderPassScope(commandBuffer, m_renderPass, getSwapchainFrameBuffers(frameNumber), getSwapchainExtent(), true, hasDepthAttachement());
		
		// For each camera - Forward pass
		{
			auto extent = getSwapchainExtent();
			cam.updateWindowExtent(extent);

			vkinit::Commands::initViewportAndScissor(m_viewport, m_scissorRect, extent);
			vkCmdSetViewport(commandBuffer, 0, 1, &m_viewport);
			vkCmdSetScissor(commandBuffer, 0, 1, &m_scissorRect);

			const auto handleViewUBO = m_globalPipelineState->fillCameraUBO(cam);

			vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
				pipelineLayout, 1, 1, &handleViewUBO.descriptorSet, 0, nullptr);
			stats.descriptorSetCount += 1;

			const auto& cameraFrustum = Frustum(cam);
			// Move the objects that are out of frustum to the end of the vector.
			auto partition = std::partition(sortedList.begin(), sortedList.end(), [&cameraFrustum](auto& renderer) 
				{
					const auto& model = renderer.transform->localToWorld;
					return renderer.bounds != nullptr && cameraFrustum.isOnFrustum(renderer.bounds->getTransformed(model));
				}
			);
			// Cut the end of the vector, preserving only the objects that are in frustum.
			sortedList.resize(sortedList.size() - static_cast<size_t>(std::distance(partition, sortedList.end())));

			auto cameraPosition = cam.getPosition();
			// Sort the objects in frustum to minimize texture state change.
			std::sort(sortedList.begin(), sortedList.end(), [cameraPosition](const VkMeshRenderer& a, const VkMeshRenderer& b)
				{
					return a.material->getHash() < b.material->getHash();
				}
			);

			const VkMaterialVariant* prevVariant = nullptr;
			for (auto& renderer : sortedList)
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

					// if (bitFlagPresent(stateChange, VariantStateChange::DescriptorSet))
					{
						vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, variant.getPipelineLayout(),
							PipelineDescriptor::BindingSlots::MaterialTextures, 1, variant.getDescriptorSet(frameNumber), 0, nullptr);
						stats.descriptorSetCount += 1;
					}

					prevVariant = &variant;
				}

				drawAt(commandBuffer, renderer, renderer.transform->localToWorld);
				stats.drawCallCount += 1;
			}
		}

		if(m_debugModule && m_debugModule->getActive())
		{
			auto swExtent = getSwapchainExtent();
			auto extent = swExtent;
			extent.width = static_cast<int32_t>( extent.width * 0.25f );
			extent.height = static_cast<int32_t>( extent.height * 0.25f );

			vkinit::Commands::initViewportAndScissor(m_viewport, m_scissorRect, extent, 
				static_cast<int32_t>( swExtent.width * 0.75f ), static_cast<int32_t>( swExtent.height * 0.0f ));
			vkCmdSetViewport(commandBuffer, 0, 1, &m_viewport);
			vkCmdSetScissor(commandBuffer, 0, 1, &m_scissorRect);

			vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_debugModule->getPipeline());
			stats.pipelineCount += 1;

			vkCmdDraw(commandBuffer, 3, 1, 0, 0);
		}

		if (ImGui::GetDrawData())
		{
			ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
		}
	}
}