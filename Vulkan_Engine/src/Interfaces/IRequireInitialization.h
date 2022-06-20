#pragma once
#include "Common.h"

template <typename T, typename ...Args>
bool tryInitialize(UNQ<T>& toInitialize, Args&&... args)
{
	toInitialize = MAKEUNQ<T>(std::forward<Args>(args)...);
	auto result = toInitialize->isInitialized();
	if (!result)
	{
		printf("Failed to create an instance of type %s\n", typeid(T).name());
	}

	return result;
}

class IRequireInitialization
{
public:
	virtual bool isInitialized() const = 0;
};