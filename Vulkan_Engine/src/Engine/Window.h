#pragma once
#include "pch.h"

class Window
{
public:
	Window(SDL_WindowFlags flags, const std::string& appName, uint32_t width, uint32_t height);

	SDL_Window* get() const;
	const std::string& getAppName() const;

	void release();

private:
	std::string m_appName;
	SDL_Window* m_windowPtr;
};