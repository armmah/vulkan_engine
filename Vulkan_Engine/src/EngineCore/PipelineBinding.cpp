#include "pch.h"
#include "PipelineBinding.h"

#include "Common.h"

#include "Camera.h"
#include "DescriptorPoolManager.h"
#include "VkGraphicsPipeline.h"
#include "UboAllocatorDelegate.h"

VkDescriptorSetLayout PipelineDescriptor::getDescriptorSetLayout(BindingSlots slot) { return m_appendedDescSetLayouts[static_cast<int>(slot)]; }

BufferHandle PipelineDescriptor::fillGlobalConstantsUBO(const glm::mat4& worldToLight, const glm::vec4& bias_ambient)
{
	auto constantsData = ConstantsUBO{};
	constantsData.world_to_light = worldToLight;
	constantsData.light_to_world = glm::inverse(worldToLight);
	constantsData.bias_ambient = bias_ambient;

	const auto handle = m_globalConstantsUBO->getHandle(m_currentFrameNumber);
	handle.CopyData(&constantsData, sizeof(ConstantsUBO));

	return handle;
}

bool PipelineDescriptor::tryCreateDescriptorSetLayouts(VkDevice device)
{
	static_assert( bindingStages.size() == BindingSlots::MAX );

	return vkinit::Descriptor::createDescriptorSetLayout(

		m_appendedDescSetLayouts[BindingSlots::Constants], device, 
		vkinit::BoundBuffer(bindingStages[BindingSlots::Constants])

	) && vkinit::Descriptor::createDescriptorSetLayout(

		m_appendedDescSetLayouts[BindingSlots::View], device, 
		vkinit::BoundBuffer(bindingStages[BindingSlots::View])

	) && vkinit::Descriptor::createDescriptorSetLayout(

		m_appendedDescSetLayouts[BindingSlots::Shadowmap], device, 
		vkinit::BoundTexture(bindingStages[BindingSlots::Shadowmap])

	) && vkinit::Descriptor::createDescriptorSetLayout(

		m_appendedDescSetLayouts[BindingSlots::MaterialTextures], device, 
		vkinit::BoundTexture(bindingStages[BindingSlots::MaterialTextures])

	);
}

bool PipelineDescriptor::tryCreatePipelineLayout(VkPipelineLayout& pipelineLayout, const VkDevice device, uint32_t maxCount)
{
	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

	pipelineLayoutInfo.setLayoutCount = std::min(getSetLayoutsCount(), maxCount);
	pipelineLayoutInfo.pSetLayouts = getAllSetLayouts();

	VkPushConstantRange pushConstantRange{};
	pushConstantRange.offset = 0;
	pushConstantRange.size = sizeof(TransformPushConstant);
	pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
	pipelineLayoutInfo.pushConstantRangeCount = 1;

	return vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) == VK_SUCCESS;
}

bool PipelineDescriptor::allocateConstantsUBO(VkDevice device, VkDescriptorPool pool)
{
	const auto constantsUBOLayout = getDescriptorSetLayout(PipelineDescriptor::BindingSlots::Constants);
	m_globalConstantsUBO = MAKEUNQ<BuffersUBO>(constantsUBOLayout, as_uint32(sizeof(ConstantsUBO)));
	return m_globalConstantsUBO->allocate(device, pool, constantsUBOLayout);
}

bool PipelineDescriptor::allocateViewUBO(VkDevice device, VkDescriptorPool pool)
{
	const auto viewUBOLayout = getDescriptorSetLayout(PipelineDescriptor::BindingSlots::View);
	m_globalViewUBOCollection = BufferUBOPool(UBOAllocatorDelegate(device, pool, viewUBOLayout));
	return true;
}

bool PipelineDescriptor::tryCreateUBOs(VkDevice device)
{
	auto pool = DescriptorPoolManager::getInstance()->createNewPool(SWAPCHAIN_IMAGE_COUNT * 10);

	return allocateConstantsUBO(device, pool) &&
		allocateViewUBO(device, pool);
}

BufferHandle PipelineDescriptor::fillCameraUBO(const Camera& cam)
{
	auto viewData = ViewUBO{};
	viewData.view_matrix = cam.getViewMatrix();
	viewData.persp_matrix = cam.getPerspectiveMatrix();
	viewData.view_persp_matrix = cam.getViewProjectionMatrix();
	
	const auto handle = m_globalViewUBOCollection.claim()->getHandle(m_currentFrameNumber);
	handle.CopyData(&viewData, sizeof(ViewUBO));

	return handle;
}

void PipelineDescriptor::insertGraphicsPipelineFor(const VkShader* shader, VkGraphicsPipeline pipeline) { globalPipelineList[shader] = pipeline; }

bool PipelineDescriptor::tryGetGraphicsPipelineFor(const VkShader* shader, VkGraphicsPipeline& pipeline)
{
	if (globalPipelineList.count(shader) == 0)
		return false;

	pipeline = globalPipelineList[shader];
	return true;
}

const VkDescriptorSetLayout* PipelineDescriptor::getAllSetLayouts() const { return m_appendedDescSetLayouts.data(); }

uint32_t PipelineDescriptor::getSetLayoutsCount() const { return as_uint32(m_appendedDescSetLayouts.size()); }

void PipelineDescriptor::release(VkDevice device)
{
	m_globalConstantsUBO->release();
	m_globalViewUBOCollection.releaseAllResources();

	for (auto& graphicsPipeline : globalPipelineList)
	{
		graphicsPipeline.second.m_pipelineLayout = VK_NULL_HANDLE;
		vkDestroyPipeline(device, graphicsPipeline.second.m_pipeline, nullptr);
	}

	vkDestroyPipelineLayout(device, m_forwardPipelineLayout, nullptr);

	vkDestroyPipelineLayout(device, m_depthOnlyPipelineLayout, nullptr);

	for (auto& setLayout : PipelineDescriptor::m_appendedDescSetLayouts)
	{
		vkDestroyDescriptorSetLayout(device, setLayout, nullptr);
	}

	globalPipelineList.clear();
}
