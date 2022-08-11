#pragma once


#ifdef WH_PLATFORM_WINDOWS

extern Whale::Application* Whale::createApplication();

int main(int argc, char** argv)
{
	Whale::Log::init();
	Whale::Log::getCoreLogger()->info("From whale!");
	WH_CRITICAL("From entry Point");
	auto app = Whale::createApplication();
	app->run();
	delete app;
}

#endif