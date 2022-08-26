#include "pch.h"
#include "VkTypes/VkMaterialVariant.h"

#include "Material.h"
#include "Presentation/Device.h"
#include "PresentationTarget.h"
#include <EngineCore/Texture.h>
#include "VkTypes/VkTexture.h"
#include "VkTypes/VkShader.h"

VkMaterial::VkMaterial(const VkShader& shader, const VkTexture2D& texture, const VkPipeline pipeline, const VkPipelineLayout pipelineLayout, const VkDescriptorSetLayout descriptorSetLayout, std::array<VkDescriptorSet, SWAPCHAIN_IMAGE_COUNT>& descriptorSets)
	: shader(&shader), texture(&texture), variant(pipeline, pipelineLayout, descriptorSetLayout, descriptorSets)
{
}

void VkMaterial::release(VkDevice device)
{
	shader = nullptr;
	texture = nullptr;

	variant.release(device);
}
