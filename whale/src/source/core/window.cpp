#include "core/window.h"

#ifdef WH_PLATFORM_WINDOWS
	#include "platform/windows/win_window.h"
#endif

namespace Whale
{
	Scope<Window> Window::Create(const WindowProps& props)
	{
#ifdef WH_PLATFORM_WINDOWS
		return CreateScope<WindowsWindow>(props);
#else
		WH_CORE_ASSERT(false, "Unknown platform!");
		return nullptr;
#endif
	}

}