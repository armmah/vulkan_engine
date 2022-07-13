#include "pch.h"
#include "VkTypes/VkMaterialVariant.h"

#include "Material.h"
#include "Presentation/Device.h"
#include "PresentationTarget.h"
#include <EngineCore/Texture.h>
#include "VkTypes/VkTexture.h"

Material::Material(const Presentation::Device* device, const Presentation::PresentationTarget* presentationTarget, VkDescriptorPool pool, const MaterialSource& materialSource)
	: presentationTarget(presentationTarget), source(materialSource), shader(device->getDevice(), materialSource.shaderSourcesOnDisk)
{
	texture = MAKEUNQ<VkTexture2D>(materialSource.textureOnDisk.path, VkMemoryAllocator::getInstance()->m_allocator, device);

	variant = MAKEUNQ<VkMaterialVariant>(this, device->getDevice(), pool, true);
}

Material::~Material() {}

void Material::release(VkDevice device)
{
	shader.release(device);
	texture->release(device);

	variant->release(device);
}
