#include "pch.h"
#include "ShadowmapPass.h"
#include "VkTypes/InitializersUtility.h"
#include "VkTypes/VkShader.h"
#include "Mesh.h"
#include "VkTypes/PipelineConstructor.h"
#include "EngineCore/DescriptorPoolManager.h"
#include "EngineCore/PipelineBinding.h"
#include "Presentation/PresentationTarget.h"
#include "Material.h"
#include "VkTypes/VkMaterialVariant.h"

#include "StagingBufferPool.h"
#include "Presentation/Device.h"
#include "Texture.h"

namespace Presentation
{
	ShadowMap::ShadowMap(PresentationTarget& target, VkDevice device, VkPipelineLayout depthOnlyPipelineLayout, bool isEnabled, uint32_t dimensionsXY) : Pass(isEnabled),
		m_dimensionsXY(std::clamp(dimensionsXY, MIN_SHADOWMAP_DIMENSION, MAX_SHADOWMAP_DIMENSION)),
		m_renderPass(), m_frameBuffer(), m_isInitialized(false),
		m_shadowMap(VkTexture2D::createTexture(device, m_dimensionsXY, m_dimensionsXY, FORMAT, USAGE_FLAGS, VIEW_IMAGE_ASPECT_FLAGS)),
		m_replacementShader(), m_replacementMaterial()
	{
		m_extent = {};
		m_extent.width = dimensionsXY;
		m_extent.height = dimensionsXY;

		vkinit::Commands::initViewportAndScissor(m_viewport, m_scissorRect, m_extent);

		std::array<VkImageView, SWAPCHAIN_IMAGE_COUNT> imageViews{
			m_shadowMap.imageView
		};

		m_isInitialized =
			m_shadowMap.isValid() &&
			vkinit::Surface::createRenderPass(m_renderPass, device, (VkFormat)0, true);

		for (auto& fb : m_frameBuffer)
		{
			m_isInitialized &= vkinit::Surface::createFrameBuffer(fb, device, m_renderPass, m_extent, imageViews, 1u);
		}

		m_replacementShader = VkShader::findShader(1u);
		if (m_replacementShader && PipelineConstruction::createPipeline(m_replacementMaterial.m_pipeline, depthOnlyPipelineLayout, device,
			getRenderPass(), getExtent(), *m_replacementShader, &Mesh::defaultMeshDescriptor, PipelineConstruction::FaceCulling::Front, true))
		{
			target.m_globalPipelineState->insertGraphicsPipelineFor(m_replacementShader, m_replacementMaterial);
		}
		else m_isInitialized = false;

		{
			auto pool = DescriptorPoolManager::getInstance()->createNewPool(3u);
			m_isInitialized &= target.createGraphicsMaterial(m_shadowMapMaterial, device, pool, VkShader::findShader(0u), &m_shadowMap);
		}

		if (!m_isInitialized)
		{
			printf("Was not able to initialize ShadowMap Pass.\n");
			setActive(false);
		}
	}

	ShadowMap::~ShadowMap() = default;

	bool ShadowMap::isInitialized() const { return m_isInitialized; }
	const VkViewport& ShadowMap::getViewport() const { return m_viewport; }
	const VkRect2D& ShadowMap::getScissorRect() const { return m_scissorRect; }
	const VkExtent2D ShadowMap::getExtent() const { return m_extent; }
	const VkRenderPass ShadowMap::getRenderPass() const { return m_renderPass; }
	const VkFramebuffer ShadowMap::getFrameBuffer(uint32_t frameNumber) const { return m_frameBuffer[frameNumber % SWAPCHAIN_IMAGE_COUNT]; }
	const VkTexture2D& ShadowMap::getTexture2D() const { return m_shadowMap; }
	const VkMaterialVariant& ShadowMap::getMaterialVariant() const { return m_shadowMapMaterial->getMaterialVariant(); }
	VkFormat ShadowMap::getFormat() const { return FORMAT; }

	void ShadowMap::release(VkDevice device)
	{
		m_shadowMap.release(device);

		for (auto& fb : m_frameBuffer)
		{
			vkDestroyFramebuffer(device, fb, nullptr);
		}
		vkDestroyRenderPass(device, m_renderPass, nullptr);
	}
}

namespace Presentation
{	
	EmptyShadowMap::EmptyShadowMap(PresentationTarget& target, const Device& device, const VkShader* shader) : Pass(true)
	{
		VkFormat f = VkFormat::VK_FORMAT_R32_SFLOAT;

		constexpr uint32_t value = 0u;
		constexpr auto w = 2u, h = 2u, ch = 1u;
		unsigned char pixels[sizeof(value) / sizeof(char) * w * h] = { value, value, value, value };

		auto texBytes = LoadedTexture(pixels, sizeof(pixels), w, h);
		auto tex = Texture({ texBytes }, f, w, h, ch);

		StagingBufferPool buffer{};
		VkTexture2D::tryCreateTexture(m_texture, tex, &device, buffer);
		buffer.releaseAllResources();

		auto pool = DescriptorPoolManager::getInstance()->createNewPool(SWAPCHAIN_IMAGE_COUNT);
		target.createGraphicsMaterial(m_shadowMapMaterial, device.getDevice(), pool, shader, m_texture.get());
	}
	EmptyShadowMap::~EmptyShadowMap() = default;
	const VkMaterialVariant& EmptyShadowMap::getMaterialVariant() const { return m_shadowMapMaterial->getMaterialVariant(); }
	bool EmptyShadowMap::isInitialized() const { return true; }
	void EmptyShadowMap::release(VkDevice device)
	{
		m_texture->release(device);
		m_shadowMapMaterial->release(device);
	}
}