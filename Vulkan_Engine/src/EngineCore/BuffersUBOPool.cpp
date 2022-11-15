#include "pch.h"
#include "BuffersUBO.h"
#include "BuffersUBOPool.h"
#include "UboAllocatorDelegate.h"

BufferUBOPool::BufferUBOPool() : m_poolUsed(0) { }
BufferUBOPool::~BufferUBOPool() { }

BufferUBOPool::BufferUBOPool(UBOAllocatorDelegate&& allocDelegate) :
	poolUBO(), m_poolUsed(0), m_allocDelegate(MAKEUNQ<UBOAllocatorDelegate>(allocDelegate)) { }

BuffersUBO* BufferUBOPool::claim()
{
	if (poolUBO.size() <= m_poolUsed)
	{
		const auto aDel = *m_allocDelegate;
		BuffersUBO buffer;
		if (m_allocDelegate && m_allocDelegate->invoke(buffer))
		{
			poolUBO.push_back(std::move(buffer));
		}
		else printf("Failed to allocate new UBO from the pool.");
	}

	return &poolUBO[m_poolUsed++];
}

void BufferUBOPool::operator=(BufferUBOPool&& other)
{
	poolUBO = std::move(other.poolUBO);
	m_poolUsed = other.m_poolUsed;
	m_allocDelegate = std::move(other.m_allocDelegate);
}

void BufferUBOPool::freeAllClaimed()
{
	m_poolUsed = 0;
}

void BufferUBOPool::releaseAllResources()
{
	for (auto& ubo : poolUBO)
		ubo.release();
}
