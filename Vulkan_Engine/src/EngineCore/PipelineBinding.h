#pragma once
#include "pch.h"
#include "Interfaces/IRequireInitialization.h"
#include "BuffersUBO.h"

namespace vkinit { struct ShaderBinding; }
struct VkShader;
struct VkGraphicsPipeline;
class Camera;

struct VkGraphicsPipeline
{
	VkPipeline pipeline;
	VkPipelineLayout pipelineLayout;

	VkGraphicsPipeline& operator =(VkGraphicsPipeline other)
	{
		std::swap(pipeline, other.pipeline);
		std::swap(pipelineLayout, other.pipelineLayout);
		return *this;
	}
};

struct PipelineDescriptor : IRequireInitialization
{
	static constexpr int DESCRIPTOR_SET_COUNT = 3;
	enum BindingSlots { Constants = 0, View = 1, Textures = 2, MAX = 3 };
	static constexpr std::array<VkShaderStageFlags, DESCRIPTOR_SET_COUNT> bindingStages = 
	{
		VK_SHADER_STAGE_VERTEX_BIT,
		VK_SHADER_STAGE_VERTEX_BIT,
		VK_SHADER_STAGE_FRAGMENT_BIT
	};

	PipelineDescriptor(VkDevice device)
	{
		m_isInitialized = tryCreateDescriptorSetLayouts(device) &&
			tryCreatePipelineLayout(device) &&
			tryCreateUBOs(device);
	}
	bool isInitialized() const override { return m_isInitialized; }

	VkDescriptorSetLayout getDescriptorSetLayout(BindingSlots slot);

	BufferHandle fillGlobalConstantsUBO(uint32_t frameNumber);
	BufferHandle fillCameraUBO(uint32_t frameNumber, const Camera& cam);

	const VkPipelineLayout getPipelineLayout() { return m_standardPipelineLayout; }

	bool hasGraphicsPipelineFor(const VkShader* shader) const { return globalPipelineList.count(shader) != 0; }
	void insertGraphicsPipelineFor(const VkShader* shader, VkGraphicsPipeline pipeline);
	bool tryGetGraphicsPipelineFor(const VkShader* shader, VkGraphicsPipeline& pipeline);

	void release(VkDevice device);

private:
	VkPipelineLayout m_standardPipelineLayout;
	UNQ<BuffersUBO> m_globalConstantsUBO, m_globalViewUBO;

	std::array<VkDescriptorSetLayout, DESCRIPTOR_SET_COUNT> m_appendedDescSetLayouts;
	std::unordered_map<const VkShader*, VkGraphicsPipeline> globalPipelineList;

	bool m_isInitialized;

	bool tryCreateDescriptorSetLayouts(VkDevice device);
	bool tryCreatePipelineLayout(VkDevice device);
	bool tryCreateUBOs(VkDevice device);
	
	const VkDescriptorSetLayout* getAllSetLayouts() const;
	uint32_t getSetLayoutsCount() const;
};