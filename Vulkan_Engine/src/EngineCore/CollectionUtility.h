#pragma once
#include "pch.h"

constexpr std::size_t operator "" _z(unsigned long long n)
{
	return n;
}

template<typename T>
static size_t vectorsizeof(const typename std::vector<T>& vec)
{
	return sizeof(T) * vec.size();
}

template<typename T>
static size_t normalizedSize(const typename std::vector<T>& vec)
{
	return std::clamp(vec.size(), 0_z, 1_z);
}

template<typename T>
constexpr static size_t vectorElementsizeof(const typename std::vector<T>& vec)
{
	return sizeof(T);
}

template<typename T>
static size_t sumValues(const typename T* arr, size_t length)
{
	size_t total = 0;
	for (size_t i = 0; i < length; i++)
		total += arr[i];
	return total;
}
