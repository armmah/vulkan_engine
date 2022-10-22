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

// WRITE
void Material::serialize(boost::archive::binary_oarchive& ar, const unsigned int version)
{
	ar& m_shaderIdentifier;

	const std::string compressedAlternative = Directories::getWorkingDirectory().combine(m_textureParameters.getTextureName(false) + ".dds");
	if (std::filesystem::exists(compressedAlternative))
	{
		m_textureParameters.path.value = compressedAlternative;
		m_textureParameters.format = VK_FORMAT_BC1_RGBA_SRGB_BLOCK;
		m_textureParameters.generateTheMips = false;
	}

	// Always serializing relative path
	m_textureParameters.path.removeDirectory(Directories::getWorkingDirectory());
	// printf("path: %s\n", Directories::getWorkingDirectory().combine(m_textureParameters.path.value).value.c_str());
	assert(std::filesystem::exists(Directories::getWorkingDirectory().combine(m_textureParameters.path.value).value));

	ar& m_textureParameters.path.value;
	ar& m_textureParameters.format;
	ar& m_textureParameters.generateTheMips;

	ar& m_hash;
}

// READ
void Material::serialize(boost::archive::binary_iarchive& ar, const unsigned int version)
{
	ar& m_shaderIdentifier;

	// Deserializing always relative path to the working directory
	ar& m_textureParameters.path.value;
	m_textureParameters.path = Directories::getWorkingDirectory().combine(m_textureParameters.path);

	ar& m_textureParameters.format;
	ar& m_textureParameters.generateTheMips;

	ar& m_hash;
}
