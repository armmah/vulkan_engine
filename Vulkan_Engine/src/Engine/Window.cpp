#include "pch.h"
#include "Window.h"

Window::Window(SDL_WindowFlags flags, const std::string& appName, uint32_t width, uint32_t height)
{
	SDL_Init(SDL_INIT_VIDEO);

	windowPtr = SDL_CreateWindow(
		appName.c_str(),
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		width,
		height,
		flags
	);
}

SDL_Window* Window::get() const { return windowPtr; }

void Window::release()
{
	SDL_DestroyWindow(windowPtr);
}
