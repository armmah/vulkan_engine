#pragma once
#include "pch.h"
#include "VkTypes/VkShader.h"
#include "ShaderSource.h"

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

class Material
{
public:
	// Keeping the ref to the abstraction over render pas, swapchain, etc to easily create the graphics pipeline, 
	// This is however a bad idea and the material should not be tightly coupled to those resources. Will need a refactor at some point.
	const Presentation::PresentationTarget* presentationTarget;

	Material(const Presentation::Device* device, const Presentation::PresentationTarget* presentationTarget, VkDescriptorPool pool, const MaterialSource& materialSource);
	~Material();

	void release(VkDevice device);

	Shader shader;
	UNQ<VkTexture2D> texture;
	UNQ<VkMaterialVariant> variant;
private:
	MaterialSource source;
};
