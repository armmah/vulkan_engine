#pragma once
#include "pch.h"
#include "ShaderSource.h"
#include "VkTypes/VkMaterialVariant.h"
#include "FileManager/Path.h"
#include "FileManager/Directories.h"

#include "boost/archive/binary_oarchive.hpp"
#include "boost/archive/binary_iarchive.hpp"

struct TextureSource
{
	Path path;
	VkFormat format;
	bool generateTheMips;
	// Texture type
	// Sampler type
	// Other params

	TextureSource(std::string&& path, VkFormat format = VK_FORMAT_R8G8B8A8_SRGB, bool generateMips = true) : path(std::move(path)), format(format), generateTheMips(generateMips) { }
	// TextureSource(const std::string& path, VkFormat format = VK_FORMAT_R8G8B8A8_SRGB, bool generateMips = true) : path(path), format(format), generateTheMips(generateMips) { }

	bool operator ==(const TextureSource& other) const
	{
		return format == other.format && generateTheMips == other.generateTheMips &&
			(path == other.path || path.getFileName(false) == other.path.getFileName(false));
	}

	template<class Archive>
	void serialize(Archive& ar, const unsigned int version)
	{
		ar& path.value;
		ar& format;
		ar& generateTheMips;
	}

	std::string getTextureName(bool includeExtension) const
	{
		return path.getFileName(includeExtension);
	}

private:
	friend class boost::serialization::access;
	friend class Material;
	TextureSource() : path(), format(VK_FORMAT_R8G8B8A8_SRGB), generateTheMips(true) { }
};

namespace std 
{
	template <>
	struct hash<TextureSource>
	{
		std::size_t operator()(const TextureSource& src) const
		{
			return hash<std::string>()(src.path.value);
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
	Material(uint32_t shaderIdentifier, TextureSource&& source) :
		m_shaderIdentifier(shaderIdentifier), m_textureParameters(source) { }

	const TextureSource& getTextureSource() const { return m_textureParameters; }
	uint32_t getShaderIdentifier() const { return m_shaderIdentifier; }

	// WRITE
	void serialize(boost::archive::binary_oarchive& ar, const unsigned int version)
	{
		ar& m_shaderIdentifier;

		const std::string compressedAlternative = Directories::getWorkingDirectory().combine( m_textureParameters.getTextureName(false) + ".dds" );
		if (std::filesystem::exists(compressedAlternative))
		{
			m_textureParameters.path.value = compressedAlternative;
			m_textureParameters.format = VK_FORMAT_BC1_RGBA_SRGB_BLOCK;
			m_textureParameters.generateTheMips = false;
		}

		// Always serializing relative path
		m_textureParameters.path.removeDirectory( Directories::getWorkingDirectory() );

		ar& m_textureParameters.path.value;
		ar& m_textureParameters.format;
		ar& m_textureParameters.generateTheMips;
	}

	// READ
	void serialize(boost::archive::binary_iarchive& ar, const unsigned int version)
	{
		ar& m_shaderIdentifier;

		// Deserializing always relative path to the working directory
		ar& m_textureParameters.path.value;
		m_textureParameters.path = Directories::getWorkingDirectory().combine( m_textureParameters.path );

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