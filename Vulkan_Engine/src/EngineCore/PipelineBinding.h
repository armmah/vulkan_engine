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
	VkPipeline m_pipeline;
	VkPipelineLayout m_pipelineLayout;

	VkGraphicsPipeline& operator =(VkGraphicsPipeline other)
	{
		std::swap(m_pipeline, other.m_pipeline);
		std::swap(m_pipelineLayout, other.m_pipelineLayout);
		return *this;
	}
};

#include "VkTypes/PushConstantTypes.h"
struct UBOAllocatorDelegate
{
	UBOAllocatorDelegate(VkDevice device, VkDescriptorPool pool, VkDescriptorSetLayout layout) : m_device(device), m_descPool(pool), m_descriptorSetLayout(layout) { }

	bool invoke(BuffersUBO& buffer) const
	{
		buffer = BuffersUBO(m_descriptorSetLayout, as_uint32(sizeof(ViewUBO)));
		return buffer.allocate(m_device, m_descPool, m_descriptorSetLayout);
	}

private:
	VkDevice m_device;
	VkDescriptorPool m_descPool;
	VkDescriptorSetLayout m_descriptorSetLayout;
};

struct BufferUBOPool
{
	BufferUBOPool() : m_poolUsed(0) { }
	BufferUBOPool(UBOAllocatorDelegate allocDelegate) :
		poolUBO(), m_poolUsed(0), m_allocDelegate(MAKEUNQ<UBOAllocatorDelegate>(allocDelegate)) { }

	BuffersUBO* claim()
	{
		if (poolUBO.size() <= m_poolUsed)
		{
			const auto aDel = *m_allocDelegate;
			BuffersUBO buffer;
			if (m_allocDelegate && m_allocDelegate->invoke(buffer))
			{
				poolUBO.push_back(std::move(buffer));
			}
			else printf("Failed to allocate new UBO from the pool.");
		}

		return &poolUBO[m_poolUsed ++];
	}

	void freeAllClaimed()
	{
		m_poolUsed = 0;
	}

	void releaseAllResources()
	{
		for (auto& ubo : poolUBO)
			ubo.release();
	}

private:
	std::vector<BuffersUBO> poolUBO;
	int m_poolUsed;

	UNQ<UBOAllocatorDelegate> m_allocDelegate;
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

	PipelineDescriptor(VkDevice device) : m_currentFrameNumber(0)
	{
		m_isInitialized = tryCreateDescriptorSetLayouts(device) &&
			tryCreatePipelineLayout(m_forwardPipelineLayout, device) &&
			tryCreatePipelineLayout(m_depthOnlyPipelineLayout, device, 2u) &&
			tryCreateUBOs(device);
	}
	bool isInitialized() const override { return m_isInitialized; }

	VkDescriptorSetLayout getDescriptorSetLayout(BindingSlots slot);

	BufferHandle fillGlobalConstantsUBO();
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