#pragma once

#include "../core/base.h"

namespace Whale {

	class WH_API Application {
	public:
		Application();
		~Application();

		void run();
	};

	// To be defined in client
	Application* createApplication();
}