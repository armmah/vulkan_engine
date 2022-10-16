#include "pch.h"
#include "PipelineBinding.h"

#include "Common.h"
#include "VkTypes/InitializersUtility.h"

#include "Camera.h"
#include "DescriptorPoolManager.h"
#include "VkTypes/PushConstantTypes.h"

VkDescriptorSetLayout PipelineDescriptor::getDescriptorSetLayout(BindingSlots slot) { return m_appendedDescSetLayouts[static_cast<int>(slot)]; }

BufferHandle PipelineDescriptor::fillGlobalConstantsUBO(uint32_t frameNumber)
{
	auto constantsData = ConstantsUBO{};

	const auto handle = m_globalConstantsUBO->getHandle(frameNumber);
	handle.CopyData(&constantsData, sizeof(ConstantsUBO));

	return handle;
}

bool PipelineDescriptor::tryCreateDescriptorSetLayouts(VkDevice device)
{
	static_assert( bindingStages.size() == BindingSlots::MAX );

	return vkinit::Descriptor::createDescriptorSetLayout(m_appendedDescSetLayouts[BindingSlots::Constants], device, vkinit::BindedBuffer(bindingStages[BindingSlots::Constants])) &&
		vkinit::Descriptor::createDescriptorSetLayout(m_appendedDescSetLayouts[BindingSlots::View], device, vkinit::BindedBuffer(bindingStages[BindingSlots::View])) &&
		vkinit::Descriptor::createDescriptorSetLayout(m_appendedDescSetLayouts[BindingSlots::Textures], device, vkinit::BindedTexture(bindingStages[BindingSlots::Textures]));
}

bool PipelineDescriptor::tryCreatePipelineLayout(VkDevice device)
{
	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

	pipelineLayoutInfo.setLayoutCount = getSetLayoutsCount();
	pipelineLayoutInfo.pSetLayouts = getAllSetLayouts();

	VkPushConstantRange pushConstantRange{};
	pushConstantRange.offset = 0;
	pushConstantRange.size = sizeof(TransformPushConstant);
	pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
	pipelineLayoutInfo.pushConstantRangeCount = 1;

	return vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &m_standardPipelineLayout) == VK_SUCCESS;
}

bool PipelineDescriptor::tryCreateUBOs(VkDevice device)
{
	const auto constantsUBOLayout = getDescriptorSetLayout(PipelineDescriptor::BindingSlots::Constants);
	m_globalConstantsUBO = MAKEUNQ<BuffersUBO>(constantsUBOLayout, as_uint32(sizeof(ConstantsUBO)));

	const auto viewUBOLayout = getDescriptorSetLayout(PipelineDescriptor::BindingSlots::View);
	m_globalViewUBO = MAKEUNQ<BuffersUBO>(viewUBOLayout, as_uint32(sizeof(ViewUBO)));

	auto pool = DescriptorPoolManager::getInstance()->createNewPool(SWAPCHAIN_IMAGE_COUNT * 2);
	return m_globalConstantsUBO->allocate(device, pool, constantsUBOLayout) &&
		m_globalViewUBO->allocate(device, pool, viewUBOLayout);
}

BufferHandle PipelineDescriptor::fillCameraUBO(uint32_t frameNumber, const Camera& cam)
{
	auto viewData = ViewUBO{};
	viewData.view_matrix = cam.getViewMatrix();
	viewData.persp_matrix = cam.getPerspectiveMatrix();
	viewData.view_persp_matrix = cam.getViewProjectionMatrix();

	const auto handle = m_globalViewUBO->getHandle(frameNumber);
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
	m_globalViewUBO->release();

	for (auto& graphicsPipeline : globalPipelineList)
	{
		// vkDestroyPipelineLayout(device, graphicsPipeline.second.pipelineLayout, nullptr);
		graphicsPipeline.second.pipelineLayout = VK_NULL_HANDLE;
		vkDestroyPipeline(device, graphicsPipeline.second.pipeline, nullptr);
	}

	vkDestroyPipelineLayout(device, m_standardPipelineLayout, nullptr);

	for (auto& setLayout : PipelineDescriptor::m_appendedDescSetLayouts)
	{
		vkDestroyDescriptorSetLayout(device, setLayout, nullptr);
	}

	globalPipelineList.clear();
}
