#include "pch.h"
#include "Window.h"

Window::Window(SDL_WindowFlags flags, const std::string& appName, uint32_t width, uint32_t height)
	: m_appName(appName)
{
	SDL_Init(SDL_INIT_VIDEO);

	m_windowPtr = SDL_CreateWindow(
		m_appName.c_str(),
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		width,
		height,
		flags
	);
}

SDL_Window* Window::get() const { return m_windowPtr; }

const std::string& Window::getAppName() const { return m_appName; }

void Window::release()
{
	SDL_DestroyWindow(m_windowPtr);
}
