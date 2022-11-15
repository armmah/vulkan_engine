#include "pch.h"
#include "Common.h"
#include "BuffersUBO.h"
#include "UboAllocatorDelegate.h"

bool UBOAllocatorDelegate::invoke(BuffersUBO& buffer) const
{
	buffer = BuffersUBO(m_descriptorSetLayout, as_uint32(sizeof(ViewUBO)));
	return buffer.allocate(m_device, m_descPool, m_descriptorSetLayout);
}
