#include "platform/windows/win_window.h"

#include "events/application_event.h"
#include "events/mouse_event.h"
#include "events/key_event.h"


namespace Whale {

	static uint8_t s_GLFWWindowCount = 0;

	static void GLFWErrorCallback(int error, const char* description)
	{
		WH_CORE_ERROR("GLFW Error ({0}): {1}", error, description);
	}

	WindowsWindow::WindowsWindow(const WindowProps& props)
	{
		WH_PROFILE_FUNCTION();

		Init(props);
	}

	WindowsWindow::~WindowsWindow()
	{
		WH_PROFILE_FUNCTION();

		Shutdown();
	}

	void WindowsWindow::Init(const WindowProps& props)
	{
		WH_PROFILE_FUNCTION();

		m_Data.Title = props.Title;
		m_Data.Width = props.Width;
		m_Data.Height = props.Height;

		WH_CORE_INFO("Creating window {0} ({1}, {2})", props.Title, props.Width, props.Height);

		if (s_GLFWWindowCount == 0)
		{
			WH_PROFILE_SCOPE("glfwInit");
			int success = glfwInit();
			WH_CORE_ASSERT(success, "Could not initialize GLFW!");
			glfwSetErrorCallback(GLFWErrorCallback);
		}

		{
			WH_PROFILE_SCOPE("glfwCreateWindow");
			m_Window = glfwCreateWindow((int)props.Width, (int)props.Height, m_Data.Title.c_str(), nullptr, nullptr);
			++s_GLFWWindowCount;
		}

		glfwMakeContextCurrent(m_Window);
		WH_CORE_INFO("OpenGL Info:");
		WH_CORE_INFO("  Vendor: {0}", glGetString(GL_VENDOR));
		WH_CORE_INFO("  Renderer: {0}", glGetString(GL_RENDERER));
		WH_CORE_INFO("  Version: {0}", glGetString(GL_VERSION));
		glfwSetWindowUserPointer(m_Window, &m_Data);
		SetVSync(true);

		// Set GLFW callbacks
		glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* window, int width, int height)
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
				data.Width = width;
				data.Height = height;

				WindowResizeEvent event(width, height);
				data.EventCallback(event);
			});

		glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* window)
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
				WindowCloseEvent event;
				data.EventCallback(event);
			});

		glfwSetKeyCallback(m_Window, [](GLFWwindow* window, int key, int scancode, int action, int mods)
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

				switch (action)
				{
				case GLFW_PRESS:
				{
					KeyPressedEvent event(key, 0);
					data.EventCallback(event);
					break;
				}
				case GLFW_RELEASE:
				{
					KeyReleasedEvent event(key);
					data.EventCallback(event);
					break;
				}
				case GLFW_REPEAT:
				{
					KeyPressedEvent event(key, true);
					data.EventCallback(event);
					break;
				}
				}
			});

		glfwSetCharCallback(m_Window, [](GLFWwindow* window, unsigned int keycode)
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

				KeyTypedEvent event(keycode);
				data.EventCallback(event);
			});

		glfwSetMouseButtonCallback(m_Window, [](GLFWwindow* window, int button, int action, int mods)
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

				switch (action)
				{
				case GLFW_PRESS:
				{
					MouseButtonPressedEvent event(button);
					data.EventCallback(event);
					break;
				}
				case GLFW_RELEASE:
				{
					MouseButtonReleasedEvent event(button);
					data.EventCallback(event);
					break;
				}
				}
			});

		glfwSetScrollCallback(m_Window, [](GLFWwindow* window, double xOffset, double yOffset)
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

				MouseScrolledEvent event((float)xOffset, (float)yOffset);
				data.EventCallback(event);
			});

		glfwSetCursorPosCallback(m_Window, [](GLFWwindow* window, double xPos, double yPos)
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

				MouseMovedEvent event((float)xPos, (float)yPos);
				data.EventCallback(event);
			});
	}

	void WindowsWindow::Shutdown()
	{
		WH_PROFILE_FUNCTION();

		glfwDestroyWindow(m_Window);
		--s_GLFWWindowCount;

		if (s_GLFWWindowCount == 0)
		{
			glfwTerminate();
		}
	}

	void WindowsWindow::OnUpdate()
	{
		WH_PROFILE_FUNCTION();

		glfwPollEvents();
		glfwSwapBuffers(m_Window);
	}

	void WindowsWindow::SetVSync(bool enabled)
	{
		WH_PROFILE_FUNCTION();

		if (enabled)
			glfwSwapInterval(1);
		else
			glfwSwapInterval(0);

		m_Data.VSync = enabled;
	}

	bool WindowsWindow::IsVSync() const
	{
		return m_Data.VSync;
	}

}
