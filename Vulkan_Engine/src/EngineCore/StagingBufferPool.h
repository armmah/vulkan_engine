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

	// Storing stats on the buffer performance.
	struct Stats
	{
		int freeCount;
		int claimCount;

		int reusedMatchingBuffer;
		int reusedBiggerBuffer;

		int destroyedBuffer;
		int createdBuffer;

		Stats() : freeCount(0), claimCount(0),
			reusedMatchingBuffer(0), reusedBiggerBuffer(0),
			createdBuffer(0), destroyedBuffer(0) { }

		void print()
		{
			printf("Staging buffer stats: (Created new %i, reallocated to grow %i), (Reuse matching size %i, bigger size %i), (Claims %i, frees %i).\n",
				createdBuffer - destroyedBuffer, destroyedBuffer, reusedMatchingBuffer, reusedBiggerBuffer, claimCount, freeCount);
		}
	};

	bool claimAStagingBuffer(StgBuffer& buffer, uint32_t byteSize);
	void freeBuffer(const StgBuffer& buffer);
	void releaseAllResources();

	const Stats& getStatistics() const { return m_stats; }

private:
	bool indirection_allocateBuffer(StgBuffer& buffer, uint32_t size);
	void indirection_destroyBuffer(StgBuffer& buffer, bool increaseCounter = true);

	bool createNewBuffer(StgBuffer& buffer, uint32_t size);
	void claimedFromFreePool_impl(size_t index);
	void returnToFreePool_impl(const StgBuffer& buffer);

	void claim(const StgBuffer& buffer);
	void free(const StgBuffer& buffer);

	// The actual buffers
	std::vector<StgBuffer> claimedPool;
	std::vector<StgBuffer> freePool;

	Stats m_stats{};
};
