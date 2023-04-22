#pragma once
#include "pch.h"
#include "Common.h"

struct UBOAllocatorDelegate;
struct BuffersUBO;

struct BufferUBOPool
{
	BufferUBOPool();
	~BufferUBOPool();
	BufferUBOPool(UBOAllocatorDelegate&& allocDelegate);
	void operator=(BufferUBOPool&& other) noexcept;

	BuffersUBO* claim();
	void freeAllClaimed();

	void releaseAllResources();

private:
	std::vector<BuffersUBO> poolUBO;
	int m_poolUsed;

	UNQ<UBOAllocatorDelegate> m_allocDelegate;
};