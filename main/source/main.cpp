#include <whale.h>
#include <core/entry_point.h>

//#include "EditorLayer.h"

namespace Whale {

	class Vesice : public Application
	{
	public:
		Vesice(const ApplicationSpecification& spec)
			: Application(spec)
		{
			//PushLayer(new EditorLayer());
		}
	};

	Application* CreateApplication(ApplicationCommandLineArgs args)
	{
		ApplicationSpecification spec;
		spec.Name = "Vesice";
		spec.CommandLineArgs = args;

		return new Vesice(spec);
	}

}
