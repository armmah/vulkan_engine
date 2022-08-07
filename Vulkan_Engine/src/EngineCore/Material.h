#pragma once
#include "pch.h"
#include "ShaderSource.h"
#include "VkTypes/VkMaterialVariant.h"

struct TextureSource
{
	std::string path;

	TextureSource(std::string&& path) : path(path) { }

	inline static TextureSource getPlaceholderTexture() { return TextureSource("C:/Git/Vulkan_Engine/Resources/vulkan_tutorial_texture.jpg"); }
	// Texture type
	// Texture format
	// Sampler type
	// Other params
};

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
struct Shader;

class Material
{
public:
	Material(const Shader& shader, const VkTexture2D& texture, const VkPipeline pipeline, const VkPipelineLayout pipelineLayout, const VkDescriptorSetLayout descriptorSetLayout, std::array<VkDescriptorSet, SWAPCHAIN_IMAGE_COUNT>& descriptorSets);
	~Material();

	const VkMaterialVariant& getMaterialVariant() const { return variant; }
	void release(VkDevice device);

private:
	const Shader* shader;
	const VkTexture2D* texture;
	VkMaterialVariant variant;

	//MaterialSource source;
};
