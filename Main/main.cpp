#include <vk_engine.h>
#include "FileManager/Directories.h"

std::string getApplicationPath(char commandLineArgument[])
{
	std::string path;
	
	if(commandLineArgument == nullptr || !std::filesystem::exists(commandLineArgument))
	{
		path = Directories::syscall_GetApplicationPath().getFileDirectory();
	}
	else
	{
		path = Path(commandLineArgument).getFileDirectory();
	}

	return path;
}

struct ApplicationParameters
{
	Path applicationPath;
	Path applicationDirectory;

	ApplicationParameters(int argc, char* argv[])
	{
		char* applicationPath = argc > 0 ? argv[0] : nullptr;

		for (int i = 0; i < argc; i++)
		{
			if (strcmp(argv[i], "-resources") == 0 && i + 1 < argc)
			{
				applicationPath = argv[i + 1];

				i += 1;
			}
		}

		auto path = getApplicationPath(applicationPath);
		printf("Application directory '%s'.\n", path.c_str());

		applicationDirectory = Path(std::move(path));
		
	}
};

int main(int argc, char* argv[])
{
	ApplicationParameters parameters(argc, argv);
	VulkanEngine engine(parameters.applicationDirectory);

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
