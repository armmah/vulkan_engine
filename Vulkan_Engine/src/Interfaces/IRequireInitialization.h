#pragma once

class IRequireInitialization
{
public:
	virtual bool isInitialized() const = 0;
};