#pragma once
#include "pch.h"
#include "Interfaces/IRequireInitialization.h"
#include "BuffersUBO.h"
#include "BuffersUBOPool.h"

namespace vkinit { struct ShaderBinding; }
struct VkShader;
struct VkGraphicsPipeline;
class Camera;

struct PipelineDescriptor : IRequireInitialization
{
	static constexpr int DESCRIPTOR_SET_COUNT = 4;
	enum BindingSlots { Constants = 0, View = 1, Shadowmap = 2, MaterialTextures = 3, MAX = 4 };
	static constexpr std::array<VkShaderStageFlags, DESCRIPTOR_SET_COUNT> bindingStages = 
	{
		VK_SHADER_STAGE_VERTEX_BIT,
		VK_SHADER_STAGE_VERTEX_BIT,
		VK_SHADER_STAGE_FRAGMENT_BIT,
		VK_SHADER_STAGE_FRAGMENT_BIT
	};

	PipelineDescriptor(VkDevice device) : m_currentFrameNumber(0)
	{
		m_isInitialized = tryCreateDescriptorSetLayouts(device) &&
			tryCreatePipelineLayout(m_forwardPipelineLayout, device) &&
			tryCreatePipelineLayout(m_depthOnlyPipelineLayout, device, 2u) &&
			tryCreateUBOs(device);
	}
	bool isInitialized() const override { return m_isInitialized; }

	VkDescriptorSetLayout getDescriptorSetLayout(BindingSlots slot);

	BufferHandle fillGlobalConstantsUBO(const glm::mat4& worldToLight, const glm::vec4& bias_ambient);
	BufferHandle fillCameraUBO(const Camera& cam);

	const VkPipelineLayout getForwardPipelineLayout() { return m_forwardPipelineLayout; }
	const VkPipelineLayout getDepthOnlyPipelineLayout() { return m_depthOnlyPipelineLayout; }

	bool hasGraphicsPipelineFor(const VkShader* shader) const { return globalPipelineList.count(shader) != 0; }
	void insertGraphicsPipelineFor(const VkShader* shader, VkGraphicsPipeline pipeline);
	bool tryGetGraphicsPipelineFor(const VkShader* shader, VkGraphicsPipeline& pipeline);

	void release(VkDevice device);

	void StartFrame(uint32_t frameNumber) 
	{
		m_globalViewUBOCollection.freeAllClaimed();
		m_currentFrameNumber = frameNumber % SWAPCHAIN_IMAGE_COUNT;
	}

private:
	VkPipelineLayout m_forwardPipelineLayout,
		m_depthOnlyPipelineLayout;
	UNQ<BuffersUBO> m_globalConstantsUBO;
	BufferUBOPool m_globalViewUBOCollection;

	std::array<VkDescriptorSetLayout, DESCRIPTOR_SET_COUNT> m_appendedDescSetLayouts;
	std::unordered_map<const VkShader*, VkGraphicsPipeline> globalPipelineList;

	bool m_isInitialized;
	uint32_t m_currentFrameNumber;

	bool tryCreateDescriptorSetLayouts(VkDevice device);
	bool tryCreatePipelineLayout(VkPipelineLayout& pipelineLayout, const VkDevice device, uint32_t maxCount = std::numeric_limits<uint32_t>::max());
	bool tryCreateUBOs(VkDevice device);

	bool allocateConstantsUBO(VkDevice device, VkDescriptorPool pool);
	bool allocateViewUBO(VkDevice device, VkDescriptorPool pool);
	
	const VkDescriptorSetLayout* getAllSetLayouts() const;
	uint32_t getSetLayoutsCount() const;
};