#pragma once
#include "pch.h"
#include "ShaderSource.h"
#include "VkTypes/VkMaterialVariant.h"

struct TextureSource
{
	std::string path;
	VkFormat format;
	bool generateTheMips;
	// Texture type
	// Sampler type
	// Other params

	TextureSource(std::string&& path, VkFormat format = VK_FORMAT_R8G8B8A8_SRGB, bool generateMips = true) : path(path), format(format), generateTheMips(generateMips) { }
	TextureSource(const std::string& path, VkFormat format = VK_FORMAT_R8G8B8A8_SRGB, bool generateMips = true) : path(path), format(format), generateTheMips(generateMips) { }

	bool operator ==(const TextureSource& other) const
	{
		return path == other.path && format == other.format && generateTheMips == other.generateTheMips;
	}

	template<class Archive>
	void serialize(Archive& ar, const unsigned int version)
	{
		ar& path;
		ar& format;
		ar& generateTheMips;
	}

private:
	friend class boost::serialization::access;
	friend class Material;
	TextureSource() : path(), format(VK_FORMAT_R8G8B8A8_SRGB), generateTheMips(true) { }
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

namespace Presentation {
	class Device;
	class PresentationTarget;
}
namespace boost {
	namespace serialization {
		class access;
	}
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

	template<class Archive>
	void serialize(Archive& ar, const unsigned int version)
	{
		ar& m_shaderIdentifier;
		ar& m_textureParameters.path;
		ar& m_textureParameters.format;
		ar& m_textureParameters.generateTheMips;
	}

	bool operator ==(const Material& other) const
	{
		return m_shaderIdentifier == other.m_shaderIdentifier &&
			m_textureParameters == other.m_textureParameters;
	}

private:
	friend class boost::serialization::access;
	Material() : m_shaderIdentifier(), m_textureParameters() { }

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