#include "pch.h"
#include "StagingBufferPool.h"

#define VERBOSITY_ERROR
//#define VERBOSITY_INFO

bool StagingBufferPool::claimAStagingBuffer(StgBuffer& buffer, uint32_t byteSize)
{
	int32_t theNextBestThing = -1;
	for (int32_t i = 0, n = freePool.size(); i < n; i++)
	{
		auto& buf = freePool[i];

		// Check for exact match
		if (byteSize == buf.totalByteSize)
		{
			buffer = buf;
			reusedMatchingBuffer += 1;
#ifdef VERBOSITY_INFO
			printf("Found a buffer (%i) matching the requested size (%i).\n", buf.totalByteSize, byteSize);
#endif

			claimedFromFreePool_impl(i);
			return true;
		}

		// The buffer we are looking at is bigger, so it would work, but lets check if we already have a better option that is closer to desired size.
		if (byteSize < buf.totalByteSize &&
			(theNextBestThing < 0 || freePool[theNextBestThing].totalByteSize > buf.totalByteSize))
		{
			theNextBestThing = i;
		}
	}

	if (theNextBestThing >= 0)
	{
		buffer = freePool[theNextBestThing];
		reusedBiggerBuffer += 1;
#ifdef VERBOSITY_INFO
		printf("Found a buffer (%i) clooose to the requested size (%i).\n", freePool[theNextBestThing].totalByteSize, byteSize);
#endif

		claimedFromFreePool_impl(theNextBestThing);
		return true;
	}

	// Doesn't exist - create
	auto success = createNewBuffer(buffer, byteSize);
	claim(buffer);
	return success;
}

void StagingBufferPool::freeBuffer(const StgBuffer& buffer)
{
	returnToFreePool_impl(buffer);
}

void StagingBufferPool::releaseAllResources()
{
	auto allocator = VkMemoryAllocator::getInstance()->m_allocator;

	for (auto& buffer : freePool)
	{
		vmaDestroyBuffer(allocator, buffer.buffer, buffer.allocation);
	}

	for (auto& buffer : claimedPool)
	{
#ifdef VERBOSITY_ERROR
		printf("[FORCE RELEASE] Warning, the buffer (%p) of size %i is still claimed, but release all resources was called on the pool.\n", buffer.buffer, buffer.totalByteSize);
#endif
		vmaDestroyBuffer(allocator, buffer.buffer, buffer.allocation);
	}

	printf("Staging buffer stats: (Created new %i, reallocated to grow %i), (Reuse matching size %i, bigger size %i), (Claims %i, frees %i).\n", 
		createdBuffer - destroyedBuffer, destroyedBuffer, reusedMatchingBuffer, reusedBiggerBuffer, claimCount, freeCount);
}

bool StagingBufferPool::createNewBuffer(StgBuffer& buffer, uint32_t size)
{
	auto allocator = VkMemoryAllocator::getInstance()->m_allocator;

	// Before creating a new bigger buffer, free all the smaller ones in the free pool.
	for (auto& freeBuf : freePool)
	{
		assert(freeBuf.totalByteSize < size && "Trying to create a new staging buffer while a bigger one was already present in the free pool!");

#ifdef VERBOSITY_INFO
		printf("TEMP\t\t\t\tDestroyed a free buffer (%i), because it was smaller than %i.\n", freeBuf.totalByteSize, size);
#endif
		destroyedBuffer += 1;
		vmaDestroyBuffer(allocator, freeBuf.buffer, freeBuf.allocation);
	}
	freePool.clear();

#ifdef VERBOSITY_INFO
	printf("Trying to allocate a new staging buffer of size %i.\n", size);
#endif
	createdBuffer += 1;

	buffer.totalByteSize = size;
	return vkinit::MemoryBuffer::allocateBufferAndMemory(buffer.buffer, buffer.allocation, allocator, size, VMA_MEMORY_USAGE_CPU_TO_GPU, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
}

void StagingBufferPool::claimedFromFreePool_impl(size_t index)
{
	if (index >= 0 && index < freePool.size())
	{
		claim(freePool[index]);

		freePool[index] = freePool.back();
		freePool.pop_back();
	}
	else
	{
#ifdef VERBOSITY_ERROR
		printf("The index %zu was out of bounds of the free pool [0, %zu).\n", index, freePool.size());
#endif
	}
}

void StagingBufferPool::returnToFreePool_impl(const StgBuffer& buffer)
{
	bool freed = false;
	for (size_t i = 0; i < claimedPool.size(); i++)
	{
		if (claimedPool[i].totalByteSize == buffer.totalByteSize &&
			claimedPool[i].allocation == buffer.allocation)
		{
			claimedPool[i] = claimedPool.back();
			claimedPool.pop_back();

			freed = true;
		}
	}

	if (freed)
	{
		free(buffer);
	}
	else
	{
#ifdef VERBOSITY_ERROR
		printf("The provided buffer is not managed by this staging buffer pool, it was not found in the list of claimed objects.\n");
#endif
	}
}

void StagingBufferPool::claim(const StgBuffer& buffer)
{
	claimedPool.push_back(buffer);
	claimCount += 1;
}

void StagingBufferPool::free(const StgBuffer& buffer)
{
	freePool.push_back(buffer);
	freeCount += 1;
}