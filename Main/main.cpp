#include <vk_engine.h>
#include "FileManager/Directories.h"

std::string getApplicationPath(char commandLineArgument[])
{
	// 1. The command line argument overrides the default fullPath, if its provided by user input and is valid.
	auto appDirectory = Path(std::string(commandLineArgument));
	if (commandLineArgument != nullptr && Directories::isValidWorkingDirectory(appDirectory))
	{
		return Path(commandLineArgument).getFileDirectory();
	}

	// 2. Try to see if the resources exist in the application directory.
	appDirectory = Path(std::move(Directories::syscall_GetApplicationPath().getFileDirectory()));
	if (Directories::isValidWorkingDirectory(appDirectory))
		return appDirectory;

	// 3. Try to exit the path and find the resources in the source directory (step out of /$(Architecture)/$(Configuration)/).
	appDirectory = appDirectory.combine("../../");
	assert(Directories::isValidWorkingDirectory(appDirectory) && "The resources could not be found near the executable, or at solution directory. Please make sure the resources folder is placed beside the executable, or provide an override path to resources via command line argument.");
	return appDirectory;
}

struct ApplicationParameters
{
	Path applicationPath;
	Path applicationDirectory;

	ApplicationParameters(int argc, char* argv[])
	{
		char* commandLineInput = argc > 0 ? argv[0] : nullptr;

		for (int i = 0; i < argc; i++)
		{
			if (strcmp(argv[i], "-resources") == 0 && i + 1 < argc)
			{
				commandLineInput = argv[i + 1];

				i += 1;
			}
		}

		applicationDirectory = Path(getApplicationPath(commandLineInput));
		printf("Application directory '%s'.\n", applicationDirectory.c_str());
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
