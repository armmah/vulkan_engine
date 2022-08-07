#include "pch.h"
#include "VkTexture.h"
#include "InitializersUtility.h"
#include <EngineCore/Texture.h>
#include "Presentation/Device.h"

VkTexture2D::VkTexture2D(std::string path, const VmaAllocator& allocator, const Presentation::Device* presentationDevice)
{
	int width, height, channels;
	stbi_uc* const pixels = stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);
	size_t imageSize = static_cast<size_t>(width) * static_cast<size_t>(height) * 4;

	if (width <= 0 || height <= 0 || imageSize <= 0 || !pixels)
	{
		printf("Could not load the image at %s.\n", path.c_str());
	}

	VkBuffer stagingBuffer;
	VmaAllocation stagingAllocation;
	if (!vkinit::MemoryBuffer::allocateBufferAndMemory(stagingBuffer, stagingAllocation, allocator, as_uint32(imageSize), VMA_MEMORY_USAGE_CPU_TO_GPU, VK_BUFFER_USAGE_TRANSFER_SRC_BIT))
		printf("Could not allocate memory buffer for texture");

	void* data;
	vmaMapMemory(allocator, stagingAllocation, &data);
	memcpy(data, pixels, imageSize);
	vmaUnmapMemory(allocator, stagingAllocation);
	stbi_image_free(static_cast<void*>(pixels));

	auto format = VK_FORMAT_R8G8B8A8_SRGB;

	auto imageUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	auto vmaci = VkMemoryAllocator::getInstance()->createAllocationDescriptor(VMA_MEMORY_USAGE_GPU_ONLY);
	vkinit::Texture::createImage(image, memoryRange, vmaci, format, imageUsage, as_uint32(width), as_uint32(height));

	Texture::transitionImageLayout(presentationDevice, image, format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	Texture::copyBufferToImage(presentationDevice, stagingBuffer, image, as_uint32(width), as_uint32(height));

	Texture::transitionImageLayout(presentationDevice, image, format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	vkinit::Texture::createTextureImageView(imageView, presentationDevice->getDevice(), image, format);
	vkinit::Texture::createTextureSampler(sampler, presentationDevice->getDevice(), true, VK_SAMPLER_ADDRESS_MODE_REPEAT, Texture::c_anisotropySamples);

	vmaDestroyBuffer(allocator, stagingBuffer, stagingAllocation);
}

void VkTexture2D::release(VkDevice device)
{
	VkTexture::release(device);

	vkDestroySampler(device, sampler, nullptr);
}

VkTexture::VkTexture(VkDevice device, MemAllocationInfo maci, VkFormat imageFormat, VkImageUsageFlags imageUsage, VkImageAspectFlags imageViewFormat, VkExtent2D extent)
{
	vkinit::Texture::createImage(image, memoryRange, maci, imageFormat, imageUsage, extent.width, extent.height);
	vkinit::Texture::createTextureImageView(imageView, device, image, imageFormat, imageViewFormat);
}

void VkTexture::release(VkDevice device)
{
	vkDestroyImageView(device, imageView, nullptr);
	vmaDestroyImage(VkMemoryAllocator::getInstance()->m_allocator, image, memoryRange);
}
