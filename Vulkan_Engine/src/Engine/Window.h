#pragma once
#include "pch.h"

class Window
{
public:
	Window(SDL_WindowFlags flags, const std::string& appName, uint32_t width, uint32_t height);

	SDL_Window* get() const;

	void release();

private:
	SDL_Window* windowPtr;
};