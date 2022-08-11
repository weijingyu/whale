#include <whale.h>
#include <entry_point.h>


namespace Whale {

	class Vesice : public Application
	{
	public:
		Vesice()
		{
		}
	};

	Application* CreateApplication()
	{

		return new Vesice();
	}

}
