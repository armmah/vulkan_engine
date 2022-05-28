#include "pch.h"
#include <vk_engine.h>


int main(int argc, char* argv[])
{
	VulkanEngine engine;

	// By default disabled
	bool validationLayers = false;

#ifndef NDEBUG
	// Override if debug build
	validationLayers = true;
#endif
	// todo - override validation layers value on command line argument

	engine.init(validationLayers);

	engine.run();

	engine.cleanup();

	return 0;
}
