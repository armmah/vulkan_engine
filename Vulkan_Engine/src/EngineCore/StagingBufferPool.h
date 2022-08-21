#pragma once
#include "VkTypes/InitializersUtility.h"
#include "VkTypes/VkMemoryAllocator.h"

class StagingBufferPool
{
public:
	struct StgBuffer
	{
		VkBuffer buffer;
		VmaAllocation allocation;

		uint32_t totalByteSize;
	};

	bool claimAStagingBuffer(StgBuffer& buffer, uint32_t byteSize);
	void freeBuffer(const StgBuffer& buffer);
	void releaseAllResources();

private:
	bool createNewBuffer(StgBuffer& buffer, uint32_t size);
	void claimedFromFreePool_impl(size_t index);
	void returnToFreePool_impl(const StgBuffer& buffer);

	void claim(const StgBuffer& buffer);
	void free(const StgBuffer& buffer);

	// The actual buffers
	std::vector<StgBuffer> claimedPool;
	std::vector<StgBuffer> freePool;

	// Storing stats on the buffer performance.
	int freeCount;
	int claimCount;

	int reusedMatchingBuffer;
	int reusedBiggerBuffer;

	int destroyedBuffer;
	int createdBuffer;
};