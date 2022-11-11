#include "pch.h"
#include "PresentationTarget.h"
#include "Mesh.h"
#include "VertexBinding.h"
#include "Material.h"
#include "VkTypes/VkShader.h"

namespace PipelineConstruction
{
	struct ComponentCreateInfoAbstract
	{
		ComponentCreateInfoAbstract() { }
		~ComponentCreateInfoAbstract() { }

		virtual bool isValid() const = 0;
		virtual void submit(VkGraphicsPipelineCreateInfo& pipelineCI) = 0;

		void submitIfValid(VkGraphicsPipelineCreateInfo& pipelineCI)
		{
			if (isValid())
			{
				submit(pipelineCI);
			}
		}
	};

	template <typename T>
	struct ComponentCI : ComponentCreateInfoAbstract
	{
	protected:
		T m_createInfo{};
	};

	struct GraphicsPipelineCI : ComponentCreateInfoAbstract
	{
		static constexpr VkStructureType STRUCTURE_TYPE = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;

		GraphicsPipelineCI(VkPipelineLayout pipelineLayout) : m_pipelineLayout(pipelineLayout) { }

		bool isValid() const override { return true; }
		void submit(VkGraphicsPipelineCreateInfo& pipelineCI) override
		{
			pipelineCI.sType = STRUCTURE_TYPE;
			pipelineCI.layout = m_pipelineLayout;

			// Parent pipeline
			pipelineCI.basePipelineHandle = VK_NULL_HANDLE;
			pipelineCI.basePipelineIndex = -1;
		}

		VkPipelineLayout m_pipelineLayout;
	};

	struct RenderPass : ComponentCreateInfoAbstract
	{
		RenderPass(VkRenderPass renderPass) : m_renderPass(renderPass) { }

		bool isValid() const override { return true; }
		void submit(VkGraphicsPipelineCreateInfo& pipelineCI) override
		{
			pipelineCI.renderPass = m_renderPass;
		}

	private:
		VkRenderPass m_renderPass;
	};

	struct ShaderStage : ComponentCI<VkPipelineShaderStageCreateInfo>
	{
		enum class SupportedStages
		{
			Vertex = VK_SHADER_STAGE_VERTEX_BIT,
			Fragment = VK_SHADER_STAGE_FRAGMENT_BIT
		};

		ShaderStage(VkShaderModule shader, SupportedStages stage, const char* fnName = nullptr)
		{
			m_createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			m_createInfo.stage = static_cast<VkShaderStageFlagBits>( stage );
			m_createInfo.module = shader;
			m_createInfo.pName = fnName ? fnName : "main";
		}

		bool isValid() const override { return m_createInfo.module != VK_NULL_HANDLE; }
		void submit(VkGraphicsPipelineCreateInfo& pipelineCI) override { throw std::exception("Should not call submit on ShaderStage."); }
		const VkPipelineShaderStageCreateInfo getCreateInfo() const { return m_createInfo; }
	};

	struct PipelineStageCollection : ComponentCreateInfoAbstract
	{
		PipelineStageCollection(ShaderStage* vertStage, ShaderStage* fragStage) 
			: m_allStages(), m_stageCount(0)
		{
			if (vertStage && vertStage->isValid())
			{
				m_allStages[m_stageCount] = vertStage->getCreateInfo();
				m_stageCount += 1;
			}

			if (fragStage && fragStage->isValid())
			{
				m_allStages[m_stageCount] = fragStage->getCreateInfo();
				m_stageCount += 1;
			}
		}

		bool isValid() const override { return m_stageCount > 0; }
		void submit(VkGraphicsPipelineCreateInfo& pipelineCI) override
		{
			assert(m_stageCount <= m_allStages.size());

			pipelineCI.stageCount = m_stageCount;
			pipelineCI.pStages = m_allStages.data();
		}

		std::array<VkPipelineShaderStageCreateInfo, 2> m_allStages;
		uint32_t m_stageCount;
	};

	struct InputAssembly : ComponentCI<VkPipelineInputAssemblyStateCreateInfo>
	{
		InputAssembly()
		{
			m_createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
			m_createInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
			m_createInfo.primitiveRestartEnable = VK_FALSE;
		}

		bool isValid() const override { return true; }
		void submit(VkGraphicsPipelineCreateInfo& pipelineCI) override
		{
			pipelineCI.pInputAssemblyState = &m_createInfo;
		}
	};

	enum FaceCulling
	{
		None = VK_CULL_MODE_NONE,
		Front = VK_CULL_MODE_FRONT_BIT,
		Back = VK_CULL_MODE_BACK_BIT,
		Both = VK_CULL_MODE_FRONT_AND_BACK,
	};

	enum TriangleWinding
	{
		CCW = VK_FRONT_FACE_COUNTER_CLOCKWISE,
		CW = VK_FRONT_FACE_CLOCKWISE
	};

	enum PolygonMode
	{
		Fill = VK_POLYGON_MODE_FILL,
		Line = VK_POLYGON_MODE_LINE,
		Point = VK_POLYGON_MODE_POINT
	};
	struct RasterizationState : ComponentCI<VkPipelineRasterizationStateCreateInfo>
	{
		RasterizationState(FaceCulling faceCullingMode, TriangleWinding winding = TriangleWinding::CCW, PolygonMode polygoneMode = PolygonMode::Fill)
		{
			m_createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
			m_createInfo.depthClampEnable = VK_FALSE;
			m_createInfo.rasterizerDiscardEnable = VK_FALSE;
			m_createInfo.polygonMode = static_cast<VkPolygonMode>( polygoneMode );
			m_createInfo.lineWidth = 1.0f;
			m_createInfo.cullMode = static_cast<VkCullModeFlagBits>( faceCullingMode );
			m_createInfo.frontFace = static_cast<VkFrontFace>( winding );

			m_createInfo.depthBiasEnable = VK_FALSE;
			m_createInfo.depthBiasConstantFactor = 0.0f;
			m_createInfo.depthBiasClamp = 0.0f;
			m_createInfo.depthBiasSlopeFactor = 0.0f;
		}

		bool isValid() const override { return true; }
		void submit(VkGraphicsPipelineCreateInfo& pipelineCI) override
		{
			pipelineCI.pRasterizationState = &m_createInfo;
		}
	};

	struct MultisampleState : ComponentCI<VkPipelineMultisampleStateCreateInfo>
	{
		MultisampleState()
		{
			m_createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
			m_createInfo.sampleShadingEnable = VK_FALSE;
			m_createInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
			m_createInfo.minSampleShading = 1.0f;
			m_createInfo.pSampleMask = nullptr;
			m_createInfo.alphaToCoverageEnable = VK_FALSE;
			m_createInfo.alphaToOneEnable = VK_FALSE;
		}

		bool isValid() const override { return true; }
		void submit(VkGraphicsPipelineCreateInfo& pipelineCI) override
		{
			pipelineCI.pMultisampleState = &m_createInfo;
		}
	};

	struct ColorBlendState : ComponentCI<VkPipelineColorBlendStateCreateInfo>
	{
		ColorBlendState()
		{
			m_attachmentState = {};
			m_attachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
			m_attachmentState.blendEnable = VK_FALSE;
			m_attachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
			m_attachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
			m_attachmentState.colorBlendOp = VK_BLEND_OP_ADD;
			m_attachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
			m_attachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
			m_attachmentState.alphaBlendOp = VK_BLEND_OP_ADD;

			m_createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
			m_createInfo.logicOpEnable = VK_FALSE;
			m_createInfo.logicOp = VK_LOGIC_OP_COPY;
			m_createInfo.attachmentCount = 1;
			m_createInfo.pAttachments = &m_attachmentState;
		}

		bool isValid() const override { return true; }
		void submit(VkGraphicsPipelineCreateInfo& pipelineCI) override
		{
			pipelineCI.pColorBlendState = &m_createInfo;
		}

	private:
		VkPipelineColorBlendAttachmentState m_attachmentState;
	};

	struct DepthStencilState : ComponentCI<VkPipelineDepthStencilStateCreateInfo>
	{
		DepthStencilState(bool enabled)
		{
			if (enabled)
			{
				m_createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;

				m_createInfo.depthTestEnable = VK_TRUE;
				m_createInfo.depthWriteEnable = VK_TRUE;
				m_createInfo.depthCompareOp = VK_COMPARE_OP_LESS;

				m_createInfo.depthBoundsTestEnable = VK_FALSE;
				m_createInfo.minDepthBounds = 0.0f;
				m_createInfo.maxDepthBounds = 1.0f;

				m_createInfo.stencilTestEnable = VK_FALSE;
				m_createInfo.front = {};
				m_createInfo.back = {};
			}
		}

		bool isValid() const override { return m_createInfo.sType == VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO; }
		void submit(VkGraphicsPipelineCreateInfo& pipelineCI) override
		{
			pipelineCI.pDepthStencilState = &m_createInfo;
		}
	};

	struct ViewportState : ComponentCI<VkPipelineViewportStateCreateInfo>
	{
		ViewportState(VkExtent2D extent)
		{
			vkinit::Commands::initViewportAndScissor(m_viewport, m_scissor, extent);

			m_createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
			m_createInfo.viewportCount = 1;
			m_createInfo.pViewports = &m_viewport;
			m_createInfo.scissorCount = 1;
			m_createInfo.pScissors = &m_scissor;
		}

		bool isValid() const override { return true; }
		void submit(VkGraphicsPipelineCreateInfo& pipelineCI) override
		{
			pipelineCI.pViewportState = &m_createInfo;
		}

	private:
		VkViewport m_viewport;
		VkRect2D m_scissor;
	};

	struct DynamicState : ComponentCI<VkPipelineDynamicStateCreateInfo>
	{
		DynamicState()
		{
			m_createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
			
			m_dynamicStates[0] = VkDynamicState::VK_DYNAMIC_STATE_VIEWPORT;
			m_dynamicStates[1] = VkDynamicState::VK_DYNAMIC_STATE_SCISSOR;
			
			m_createInfo.dynamicStateCount = 2;
			m_createInfo.pDynamicStates = m_dynamicStates;
		}

		bool isValid() const override { return true; }
		void submit(VkGraphicsPipelineCreateInfo& pipelineCI) override
		{
			pipelineCI.pDynamicState = &m_createInfo;
		}

	private:
		VkDynamicState m_dynamicStates[2];
	};

#include "CollectionUtility.h"
	struct VertexInputState : ComponentCI<VkPipelineVertexInputStateCreateInfo>
	{
		VertexInputState(const MeshDescriptor& meshDescriptor)
		{
			auto descriptorCount = meshDescriptor.descriptorCount;
			// Vertex Attribute Descriptions
			attributeDescriptions.resize(descriptorCount);

			size_t offset = 0;
			for (uint32_t i = 0; i < descriptorCount; i++)
			{
				auto size = meshDescriptor.elementByteSizes[i];

				attributeDescriptions[i].binding = 0;
				attributeDescriptions[i].location = i;
				attributeDescriptions[i].format = pickDataFormat(size);
				attributeDescriptions[i].offset = as_uint32(offset);

				offset += size * std::clamp(meshDescriptor.lengths[i], 0_z, 1_z);
			}

			bindingDescription = {};
			bindingDescription.binding = 0;
			bindingDescription.stride = as_uint32(offset);
			bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

			m_createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

			m_createInfo.vertexBindingDescriptionCount = 1;
			m_createInfo.pVertexBindingDescriptions = &bindingDescription;

			m_createInfo.vertexAttributeDescriptionCount = as_uint32(attributeDescriptions.size());
			m_createInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
		}

		bool isValid() const override { return VertexBinding::validateAttributeAndBindingDescriptions({ bindingDescription }, attributeDescriptions); }
		void submit(VkGraphicsPipelineCreateInfo& pipelineCI) override
		{
			pipelineCI.pVertexInputState = &m_createInfo;
		}

		static VkFormat pickDataFormat(size_t size)
		{
			static const VkFormat formats[4]{
				VkFormat::VK_FORMAT_R32_SFLOAT,
				VkFormat::VK_FORMAT_R32G32_SFLOAT,
				VkFormat::VK_FORMAT_R32G32B32_SFLOAT,
				VkFormat::VK_FORMAT_R32G32B32A32_SFLOAT,
			};

			auto index = size / sizeof(float) - 1;

			assert((size % sizeof(float) == 0) && index >= 0 && index < 4 && "The vertex attribute data type is not supported.");

			return formats[index];
		}

	private:
		VkVertexInputBindingDescription bindingDescription;
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
	};

	static bool createPipeline(VkPipeline& pipelineInstance, const VkPipelineLayout pipelineLayout, const VkDevice device, const VkRenderPass renderPass,
		VkExtent2D swapchainExtent, const VkShader& shader, const MeshDescriptor& descriptor, FaceCulling faceCullingMode, bool depthStencilAttachement)
	{
		VkGraphicsPipelineCreateInfo pipelineCreateInfo{};

		auto pipeline = GraphicsPipelineCI(pipelineLayout);
		auto renderPassState = RenderPass(renderPass);
			
		auto vertStage = ShaderStage(shader.vertShader, ShaderStage::SupportedStages::Vertex);
		auto fragStage = ShaderStage(shader.fragShader, ShaderStage::SupportedStages::Fragment);
		PipelineStageCollection allStages( &vertStage, &fragStage );

		auto vertexInputState = VertexInputState(descriptor);
		auto inputAssembly = InputAssembly();
		auto viewportState = ViewportState(swapchainExtent);
		auto rasterizationState = RasterizationState(faceCullingMode);
		auto multisampleState = MultisampleState();
		auto depthStencilState = DepthStencilState(depthStencilAttachement);
		auto colorBlendState = ColorBlendState();

		auto dynamicState = DynamicState();

		ComponentCreateInfoAbstract* allComponents[] {
			&pipeline,
			&renderPassState,

			// Programmable shader stages
			&allStages,

			// Fixed function stages
			&vertexInputState,
			&inputAssembly,
			&viewportState,
			&rasterizationState,
			&multisampleState,
			&depthStencilState,
			&colorBlendState,

			&dynamicState
		};

		for (auto& createInfo : allComponents)
		{
			createInfo->submitIfValid(pipelineCreateInfo);
		}

		return vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &pipelineInstance) == VK_SUCCESS;
	}
}

namespace Presentation
{
	bool PresentationTarget::createPipelineIfNotExist(VkGraphicsPipeline& graphicsPipeline, const VkPipelineLayout pipelineLayout,
		const VkDevice device, const VkShader* shader, const VkRenderPass renderPass, VkExtent2D extent)
	{
		if (!m_globalPipelineState->tryGetGraphicsPipelineFor(shader, graphicsPipeline))
		{
			graphicsPipeline.m_pipelineLayout = pipelineLayout;
			if (!PipelineConstruction::createPipeline(graphicsPipeline.m_pipeline, pipelineLayout, device,
				renderPass, extent, *shader, Mesh::defaultMeshDescriptor,
				PipelineConstruction::FaceCulling::Back, hasDepthAttachement()))
				return false;

			m_globalPipelineState->insertGraphicsPipelineFor(shader, graphicsPipeline);
		}

		return true;
	}

	bool PresentationTarget::createGraphicsMaterial(UNQ<VkMaterial>& material, const VkDevice device, const VkDescriptorPool descPool, const VkShader* shader, const VkTexture2D* texture)
	{
		VkDescriptorSetLayout descriptorSetLayout = m_globalPipelineState->getDescriptorSetLayout(PipelineDescriptor::BindingSlots::Textures);

		std::array<VkDescriptorSet, SWAPCHAIN_IMAGE_COUNT> descriptorSets;
		vkinit::Descriptor::createDescriptorSets(descriptorSets, device, descPool, descriptorSetLayout, *texture);

		VkGraphicsPipeline graphicsPipeline;
		if (!createPipelineIfNotExist(graphicsPipeline, m_globalPipelineState->getForwardPipelineLayout(), device, shader, getRenderPass(), getSwapchainExtent()))
			return false;

		material = MAKEUNQ<VkMaterial>(*shader, *texture,
			graphicsPipeline.m_pipeline, graphicsPipeline.m_pipelineLayout,
			descriptorSetLayout, descriptorSets);

		return true;
	}
}