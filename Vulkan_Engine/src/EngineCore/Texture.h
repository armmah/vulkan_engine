#pragma once
#include "pch.h"
#include "Common.h"

namespace Presentation
{
	class Device;
}

namespace boost {
	namespace serialization {
		class access;
	}
}

struct ITextureContainer
{
	virtual uint32_t getByteSize() const = 0;
	virtual void copyToMappedBuffer(void* destination, size_t offset = 0) const = 0;

protected:
	static void copy(void* destination, size_t offset, const void* source, size_t byteSize)
	{
		destination = (void*)((char*)destination + offset);
		memcpy(destination, source, byteSize);
	}
};

struct SerializedTexture : ITextureContainer
{
	SerializedTexture(stbi_uc* const data, int byteSize)
	{
		pixels = std::vector<stbi_uc>(data, data + byteSize);
		imageSize = byteSize;
	}

	uint32_t getByteSize() const override { return pixels.size() * sizeof(stbi_uc); }
	void copyToMappedBuffer(void* destination, size_t offset = 0) const override { copy(destination, offset, pixels.data(), getByteSize()); }

private:
	std::vector<stbi_uc> pixels;
	int imageSize;
};

struct LoadedTexture : ITextureContainer
{
	LoadedTexture(LoadedTexture&& texture) = default;
	LoadedTexture(unsigned char* const data, size_t byteSize, int width, int height)
		: pixels(data), imageSize(byteSize), width(static_cast<uint32_t>(width)), height(static_cast<uint32_t>(height)) { }

	uint32_t getByteSize() const override { return imageSize; }
	VkExtent3D getDimensions() const 
	{
		VkExtent3D extent;
		extent.width = width;
		extent.height = height;
		extent.depth = imageSize;
		return extent;
	}
	void copyToMappedBuffer(void* destination, size_t offset = 0) const override { copy(destination, offset, pixels, imageSize); }

	~LoadedTexture() { }

	void release() 
	{
		if (pixels != nullptr && imageSize > 0)
		{
			delete[] pixels;
			pixels = nullptr;
		}
	}

private:
	unsigned char* pixels;
	size_t imageSize;

	uint32_t width, height;
};

struct Texture
{
	constexpr static float c_anisotropySamples = 4.0f;

	static bool tryLoadSupportedFormat(std::vector<LoadedTexture>& textureMipChain, const std::string& path, VkFormat& format, int& width, int& height, int& channels);

	static void copyBufferToImage(const Presentation::Device* presentationDevice, VkBuffer buffer, VkImage image, const std::vector<VkExtent3D>& dimensions);
	static void transitionImageLayout(const Presentation::Device* presentationDevice, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipCount);

private:
	static bool stbiLoad(std::vector<LoadedTexture>& textureMipChain, const std::string& path, int& width, int& height, int& channels);
	static bool ddsLoad(std::vector<LoadedTexture>& textureMipchain, const std::string& path, VkFormat& format, int& width, int& height, int& channels);
};