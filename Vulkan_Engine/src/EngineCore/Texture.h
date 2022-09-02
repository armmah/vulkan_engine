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
	virtual void copyToMappedBuffer(void*) const = 0;
};

struct SerializedTexture : ITextureContainer
{
	SerializedTexture(stbi_uc* const data, int byteSize)
	{
		pixels = std::vector<stbi_uc>(data, data + byteSize);
		imageSize = byteSize;
	}

	uint32_t getByteSize() const override { return pixels.size() * sizeof(stbi_uc); }
	void copyToMappedBuffer(void* destination) const override { memcpy(destination, pixels.data(), getByteSize()); }

private:
	std::vector<stbi_uc> pixels;
	int imageSize;
};

struct LoadedTexture : ITextureContainer
{
	LoadedTexture(stbi_uc* const data, size_t byteSize)
	{
		pixels = data;
		imageSize = byteSize;
	}

	uint32_t getByteSize() const override { return imageSize; }
	void copyToMappedBuffer(void* destination) const override { memcpy(destination, pixels, imageSize); }

	~LoadedTexture()
	{
		if (pixels && imageSize > 0)
		{
			stbi_image_free(static_cast<void*>(pixels));
		}
	}

private:
	stbi_uc* pixels;
	size_t imageSize;
};

struct Texture
{
	constexpr static float c_anisotropySamples = 4.0f;

/*
	int width, height, channels;

	std::string path;
	VkFormat format;
	bool generateTheMips;

	// Texture type
	// Sampler type
	// Other params
	Texture(std::string&& path, VkFormat format = VK_FORMAT_R8G8B8A8_SRGB, bool generateMips = true);
	template<class Archive>
	void serialize(Archive& ar, const unsigned int version)
	{
		ar& imageSize;
		ar& width& height& channels;

		ar& pixels;
	}

	bool operator ==(const Texture& other) const
	{
		return path == other.path && format == other.format && 
			width == other.width && height == other.height && channels == other.channels &&
			generateTheMips == other.generateTheMips;
	}

	std::size_t operator()(const Texture& src) const
	{
		return std::hash<std::string>()(src.path);
	}
private:
	friend class Material;
	friend class boost::serialization::access;
	Texture() : path(), format(VK_FORMAT_R8G8B8A8_SRGB), generateTheMips(true), width(), height(), channels() { }

	UNQ<LoadedTexture> loadedTexture;
	*/

	static bool stbiLoad(UNQ<LoadedTexture>& texture, const std::string& path, int& width, int& height, int& channels);

	static void copyBufferToImage(const Presentation::Device* presentationDevice, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
	static void transitionImageLayout(const Presentation::Device* presentationDevice, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipCount);


};