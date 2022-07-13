#include "pch.h"
#include "VkMaterialVariant.h"
#include "InitializersUtility.h"
#include "Mesh.h"
#include "PresentationTarget.h"
#include "Material.h"

VkMaterialVariant::VkMaterialVariant(const Material* sourceMat, VkDevice device, VkDescriptorPool descriptorPool, bool hasDepthAttachment)
	: sourceMat(sourceMat)
{
	vkinit::Descriptor::createDescriptorSetLayout(descriptorSetLayout, device);
	sourceMat->presentationTarget->createGraphicsPipeline(pipeline, pipelineLayout, sourceMat->shader, device, Mesh::defaultVertexBinding, descriptorSetLayout, VK_CULL_MODE_BACK_BIT, hasDepthAttachment);
	vkinit::Descriptor::createDescriptorSets(descriptorSets, device, descriptorPool, descriptorSetLayout, *sourceMat->texture);
}

void VkMaterialVariant::release(VkDevice device)
{
	vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
	vkDestroyPipeline(device, pipeline, nullptr);

	vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
}
