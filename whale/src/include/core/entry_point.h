#pragma once

#include "core/base.h"
#include "core/application.h"


#ifdef WH_PLATFORM_WINDOWS

extern Whale::Application* Whale::CreateApplication(ApplicationCommandLineArgs args);

int main(int argc, char** argv)
{
	Whale::Log::init();

	// WH_PROFILE_BEGIN_SESSION("Startup", "WhaleProfile-Startup.json");
	auto app = Whale::CreateApplication({ argc, argv });
	// WH_PROFILE_END_SESSION();

	// WH_PROFILE_BEGIN_SESSION("Runtime", "WhaleProfile-Runtime.json");
	app->Run();
	// WH_PROFILE_END_SESSION();

	// WH_PROFILE_BEGIN_SESSION("Shutdown", "WhaleProfile-Shutdown.json");
	delete app;
	// WH_PROFILE_END_SESSION();
}

#endif