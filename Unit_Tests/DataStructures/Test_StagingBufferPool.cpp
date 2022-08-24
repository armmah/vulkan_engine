#include <pch.h>
#include "gtest/gtest.h"

#define NO_GRAPHICS_MODE
#define UNIT_TEST
#include "../../Vulkan_Engine/src/EngineCore/StagingBufferPool.h"

TEST(DataStructure_VkBuffers, StagingBufferPool)
{
	auto pool = StagingBufferPool();
	const auto& stats = pool.getStatistics();

	StagingBufferPool::StgBuffer buf{};
	pool.claimAStagingBuffer(buf, 4);
	EXPECT_EQ(stats.createdBuffer, 1);

	pool.releaseAllResources();
}
