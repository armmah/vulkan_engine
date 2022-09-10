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

struct MipDesc
{
	uint32_t width, height;
	uint32_t imageByteSize;

	MipDesc(uint32_t w, uint32_t h, uint32_t byteSize)
		: width(w), height(h), imageByteSize(byteSize) { }
};

struct LoadedTexture : ITextureContainer
{
	LoadedTexture(LoadedTexture&& texture) = default;
	LoadedTexture(const LoadedTexture& other) = default;
	LoadedTexture(unsigned char* const data, uint32_t byteSize, uint32_t width, uint32_t height)
		: pixels(data), imageByteSize(byteSize), width(width), height(height) { }

	uint32_t getByteSize() const override { return as_uint32(imageByteSize); }
	MipDesc getDimensions() const { return MipDesc(width, height, imageByteSize); }
	void copyToMappedBuffer(void* destination, size_t offset = 0) const override { copy(destination, offset, pixels, imageByteSize); }

	void release() noexcept
	{
		if (pixels != nullptr && imageByteSize > 0)
		{
			delete[] pixels;
			pixels = nullptr;
		}
	}

private:
	unsigned char* pixels;
	uint32_t imageByteSize;

	uint32_t width, height;
};

struct Texture
{
	Texture() : format(), width(), height(), channels() { }
	Texture(std::vector<LoadedTexture>&& loadedTextures, VkFormat f, uint32_t w, uint32_t h, uint32_t ch)
		: textureMipChain(std::move(loadedTextures)), format(f), width(w), height(h), channels(ch) { }

	void Init(std::vector<LoadedTexture>&& loadedTextures, VkFormat f, uint32_t w, uint32_t h, uint32_t ch)
	{
		textureMipChain = std::move(loadedTextures);
		format = f;
		width = w;
		height = h;
		channels = ch;
	}

	bool hasPixelData() const { return textureMipChain.size() > 0; }
	const std::vector<LoadedTexture>& getMipChain() const { return textureMipChain; }
	void releasePixelData() noexcept
	{
		for (auto& mip : textureMipChain)
		{
			mip.release();
		}
		textureMipChain.clear();
	}

	std::vector<LoadedTexture> textureMipChain;
	VkFormat format;
	uint32_t width;
	uint32_t height;
	uint32_t channels;

	~Texture() { releasePixelData(); }

	constexpr static float c_anisotropySamples = 4.0f;

	static bool tryLoadSupportedFormat(Texture& texture, const std::string& path);

	static void copyBufferToImage(const Presentation::Device* presentationDevice, VkBuffer buffer, VkImage image, const std::vector<MipDesc>& dimensions);
	static void transitionImageLayout(const Presentation::Device* presentationDevice, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipCount);

private:
	static bool stbiLoad(Texture& texture, const std::string& path);
	static bool ddsLoad(Texture& texture, const std::string& path);
};