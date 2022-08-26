#pragma once
#include "pch.h"
#include "ShaderSource.h"
#include "VkTypes/VkMaterialVariant.h"

struct TextureSource
{
	std::string path;
	VkFormat format;
	bool generateTheMips;

	TextureSource(std::string&& path, VkFormat format = VK_FORMAT_R8G8B8A8_SRGB, bool generateMips = true) : path(path), format(format), generateTheMips(generateMips) { }
	TextureSource(const std::string& path, VkFormat format = VK_FORMAT_R8G8B8A8_SRGB, bool generateMips = true) : path(path), format(format), generateTheMips(generateMips) { }

	inline static TextureSource getPlaceholderTexture() { return TextureSource("C:/Git/Vulkan_Engine/Resources/vulkan_tutorial_texture.jpg"); }
	// Texture type
	// Texture format
	// Sampler type
	// Other params

	bool operator ==(const TextureSource& other) const
	{
		return path == other.path && format == other.format && generateTheMips == other.generateTheMips;
	}
};

namespace std {

	template <>
	struct hash<TextureSource>
	{
		std::size_t operator()(const TextureSource& src) const
		{
			return hash<std::string>()(src.path);
		}
	};

}

class MaterialSource
{
public:
	ShaderSource shaderSourcesOnDisk;
	TextureSource textureOnDisk;

	MaterialSource() : shaderSourcesOnDisk(ShaderSource::getDefaultShader()), textureOnDisk(TextureSource::getPlaceholderTexture()) { }
};

namespace Presentation
{
	class Device;
	class PresentationTarget;
}
struct VkTexture2D;
struct VkMaterialVariant;
struct VkShader;

class Material
{
public:
	Material(uint32_t shaderIdentifier, const TextureSource& source) : 
		m_shaderIdentifier(shaderIdentifier), m_textureParameters(source) { }

	const TextureSource& getTextureSource() const { return m_textureParameters; }
	uint32_t getShaderIdentifier() const { return m_shaderIdentifier; }

private:
	uint32_t m_shaderIdentifier;
	TextureSource m_textureParameters;
};

struct VkMaterial
{
	VkMaterial(const VkShader& shader, const VkTexture2D& texture, const VkPipeline pipeline, const VkPipelineLayout pipelineLayout, const VkDescriptorSetLayout descriptorSetLayout, std::array<VkDescriptorSet, SWAPCHAIN_IMAGE_COUNT>& descriptorSets);

	const VkMaterialVariant& getMaterialVariant() const { return variant; }
	void release(VkDevice device);

	const VkShader* shader;
	const VkTexture2D* texture;

	VkMaterialVariant variant;
};