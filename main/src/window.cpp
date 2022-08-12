#include <window.h>

#include <whale.h>
#include <core/log.h>
#include <core/api.h>

#include <chrono>
#include <csignal>
#include <iostream>
#include <set>
#include <thread>
#include <cassert>


#include <imgui.h>
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui_internal.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <GLFW/glfw3.h>

namespace Whale {

    using namespace std::literals::chrono_literals;

    void ShowAboutWindow(bool* p_open)
    {
        if (!ImGui::Begin("About", p_open, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::End();
            return;
        }
        ImGui::Text("VeSiCE %s", ImGui::GetVersion());
        ImGui::Separator();
        ImGui::Text("By VSC-E-5");
        ImGui::Text("A Very Simple CAN Examiner.");

        ImGui::End();
    }

  /*  std::string openFileBrowser(const std::vector<nfdfilteritem_t>& validExtensions)
    {
        NFD::Init();

        nfdchar_t* outPath = nullptr;
        nfdresult_t result;
        result = NFD::OpenDialog(outPath, validExtensions.data(), validExtensions.size(), NULL);

        if (result == NFD_OKAY && outPath != nullptr)
        {
            return reinterpret_cast<char*>(outPath);
            NFD::FreePath(outPath);
        }

        NFD::Quit();

        return "";
    }*/

    

    static void signalHandler(int signalNumber) {
        WH_CRITICAL("Terminating with signal {}", signalNumber);

        if (std::uncaught_exceptions() > 0) {
            WH_CRITICAL("Uncaught exception thrown!");
        }

        // Let's not loop on this...
        std::signal(signalNumber, nullptr);

#if defined(WHALE_DEBUG)
        assert(false);
#else
        std::raise(signalNumber);
#endif
    };

    Window::Window() {

        this->initGLFW();
        this->initImGui();
        //this->setupNativeWindow();


        for (auto signal = 0; signal < NSIG; signal++)
            std::signal(signal, signalHandler);
        std::set_terminate([] { signalHandler(SIGTERM); });
    }

    Window::~Window() {

        this->exitImGui();
        this->exitGLFW();
    }

    void Window::loop() {
        this->m_lastFrameTime = glfwGetTime();
        while (!glfwWindowShouldClose(this->m_window)) {
            if (!glfwGetWindowAttrib(this->m_window, GLFW_VISIBLE) || glfwGetWindowAttrib(this->m_window, GLFW_ICONIFIED)) {
                glfwWaitEvents();
            }
            else {
                glfwPollEvents();

                bool frameRateUnlocked = ImGui::IsPopupOpen(ImGuiID(0), ImGuiPopupFlags_AnyPopupId) || this->m_mouseButtonDown || this->m_hadEvent || !this->m_pressedKeys.empty();
                const double timeout = std::max(0.0, (1.0 / 5.0) - (glfwGetTime() - this->m_lastFrameTime));

                if ((this->m_lastFrameTime - this->m_frameRateUnlockTime) > 5 && this->m_frameRateTemporarilyUnlocked && !frameRateUnlocked) {
                    this->m_frameRateTemporarilyUnlocked = false;
                }

                if (frameRateUnlocked || this->m_frameRateTemporarilyUnlocked) {
                    if (!this->m_frameRateTemporarilyUnlocked) {
                        this->m_frameRateTemporarilyUnlocked = true;
                        this->m_frameRateUnlockTime = this->m_lastFrameTime;
                    }
                }
                else {
                    glfwWaitEventsTimeout(timeout);
                }
            }


            this->frameBegin();
            this->frame();
            this->frameEnd();

            this->m_lastFrameTime = glfwGetTime();

            this->m_hadEvent = false;
        }
    }

    void Window::frameBegin() {

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

        ImGuiWindowFlags windowFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;

        ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

        if (ImGui::Begin("WhaleDockSpace", nullptr, windowFlags)) {
            auto drawList = ImGui::GetWindowDrawList();
            ImGui::PopStyleVar();
            auto sidebarPos = ImGui::GetCursorPos();
            auto sidebarWidth = 0;

            ImGui::SetCursorPosX(sidebarWidth);

            auto footerHeight = ImGui::GetTextLineHeightWithSpacing() + ImGui::GetStyle().FramePadding.y * 2 + 2;
            auto dockSpaceSize = ImVec2(Whale::System::getMainWindowSize().x - sidebarWidth, ImGui::GetContentRegionAvail().y - footerHeight);


            auto dockId = ImGui::DockSpace(ImGui::GetID("WhaleDockSpace"), dockSpaceSize);
            Whale::System::setMainDockSpaceId(dockId);

            drawList->AddRectFilled(ImGui::GetWindowPos(), ImGui::GetWindowPos() + ImGui::GetWindowSize() - ImVec2(dockSpaceSize.x, footerHeight - ImGui::GetStyle().FramePadding.y - 1), ImGui::GetColorU32(ImGuiCol_MenuBarBg));

            ImGui::Separator();
            ImGui::SetCursorPosX(8);
            ImGui::Text("Developed by VSC-E-5");


            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
            
            if (ImGui::BeginMainMenuBar())
            {
                if (ImGui::BeginMenu("File"))
                {
                    // ShoweMenuFile();
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("Project"))
                {
                    if (ImGui::MenuItem("Choose Project"))
                    {
                        // SetActiveProject();
                    }
                    if (ImGui::MenuItem("Import new project"))
                    {
                        // ImportNewProject();
                    }
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("Diag Log"))
                {
                    if (ImGui::MenuItem("Open Log File"))
                    {
                        //auto filename = openFileBrowser({ {"CAN Log file", "log"} });
                        //buslog = new Whale::BusLog(filename);
                    };

                    ImGui::EndMenu();
                }
                // if (ImGui::MenuItem("MenuItem")) {} // You can also use MenuItem() inside a menu bar!
                if (ImGui::BeginMenu("Help"))
                {
                    //ImGui::MenuItem("About", NULL, &show_app_about);
                    ImGui::EndMenu();
                }
                ImGui::EndMainMenuBar();
            }

            
            ImGui::PopStyleVar();

           

            this->beginNativeWindowFrame();

            drawList->AddLine(ImGui::GetWindowPos(), ImGui::GetWindowPos() + ImGui::GetWindowSize() - ImVec2(dockSpaceSize.x + 2, footerHeight - ImGui::GetStyle().FramePadding.y - 2), ImGui::GetColorU32(ImGuiCol_Separator));
            //drawList->AddLine(ImGui::GetWindowPos(), ImGui::GetCurrentWindow()->MenuBarHeight(), ImGui::GetWindowPos() + ImVec2(ImGui::GetWindowSize().x, ImGui::GetCurrentWindow()->MenuBarHeight()), ImGui::GetColorU32(ImGuiCol_Separator));
        }
        ImGui::End();
        ImGui::PopStyleVar(2);
    }

    void Window::frame() {

        auto& io = ImGui::GetIO();
        
        static bool show_demo = true;

        if (show_demo) {
            ImGui::ShowDemoWindow(&show_demo);
        }
 

        this->m_pressedKeys.clear();
    }

    void Window::frameEnd() {
        this->endNativeWindowFrame();
        ImGui::Render();

        int displayWidth, displayHeight;
        glfwGetFramebufferSize(this->m_window, &displayWidth, &displayHeight);
        glViewport(0, 0, displayWidth, displayHeight);
        glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        GLFWwindow* backup_current_context = glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(backup_current_context);

        glfwSwapBuffers(this->m_window);
    }

    void Window::initGLFW() {
        glfwSetErrorCallback([](int error, const char* desc) {
            WH_ERROR("GLFW Error [{}] : {}", error, desc);
            });

        if (!glfwInit()) {
            WH_CRITICAL("Failed to initialize GLFW!");
            std::abort();
        }

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

        glfwWindowHint(GLFW_DECORATED, true ? GL_FALSE : GL_TRUE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

        this->m_windowTitle = "Whale";
        this->m_window = glfwCreateWindow(1280, 720, this->m_windowTitle.c_str(), nullptr, nullptr);

        glfwSetWindowUserPointer(this->m_window, this);

        if (this->m_window == nullptr) {
            WH_CRITICAL("Failed to create window!");
            std::abort();
        }

        glfwMakeContextCurrent(this->m_window);
        glfwSwapInterval(1);

        {
            int x = 0, y = 0;
            glfwGetWindowPos(this->m_window, &x, &y);

            Whale::System::setMainWindowPosition(x, y);
        }

        {
            int width = 0, height = 0;
            glfwGetWindowSize(this->m_window, &width, &height);
            glfwSetWindowSize(this->m_window, width, height);
            Whale::System::setMainWindowSize(width, height);
        }

        glfwSetWindowPosCallback(this->m_window, [](GLFWwindow* window, int x, int y) {
            Whale::System::setMainWindowPosition(x, y);

            if (auto g = ImGui::GetCurrentContext(); g == nullptr || g->WithinFrameScope) return;

            auto win = static_cast<Window*>(glfwGetWindowUserPointer(window));
            win->frameBegin();
            win->frame();
            win->frameEnd();
            win->processEvent();
            });

        glfwSetWindowSizeCallback(this->m_window, [](GLFWwindow* window, int width, int height) {
            if (!glfwGetWindowAttrib(window, GLFW_ICONIFIED))
                Whale::System::setMainWindowSize(width, height);

            if (auto g = ImGui::GetCurrentContext(); g == nullptr || g->WithinFrameScope) return;

            auto win = static_cast<Window*>(glfwGetWindowUserPointer(window));
            win->frameBegin();
            win->frame();
            win->frameEnd();
            win->processEvent();
            });

        glfwSetMouseButtonCallback(this->m_window, [](GLFWwindow* window, int button, int action, int mods) {
            //hex::unused(button, mods);

            auto win = static_cast<Window*>(glfwGetWindowUserPointer(window));

            if (action == GLFW_PRESS)
                win->m_mouseButtonDown = true;
            else if (action == GLFW_RELEASE)
                win->m_mouseButtonDown = false;
            win->processEvent();
            });

        glfwSetKeyCallback(this->m_window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
            //hex::unused(mods);

            auto keyName = glfwGetKeyName(key, scancode);
            if (keyName != nullptr)
                key = std::toupper(keyName[0]);

            auto win = static_cast<Window*>(glfwGetWindowUserPointer(window));

            if (action == GLFW_PRESS || action == GLFW_REPEAT) {
                win->m_pressedKeys.push_back(key);
            }
            win->processEvent();
            });

        glfwSetDropCallback(this->m_window, [](GLFWwindow*, int count, const char** paths) {
            if (count != 1)
                return;

            for (int i = 0; i < count; i++) {
                auto path = std::filesystem::path(reinterpret_cast<const char8_t*>(paths[i]));

                bool handled = false;
            }
            });

        glfwSetCursorPosCallback(this->m_window, [](GLFWwindow* window, double x, double y) {
            //hex::unused(x, y);

            auto win = static_cast<Window*>(glfwGetWindowUserPointer(window));
            win->processEvent();
            });

        /*glfwSetWindowCloseCallback(this->m_window, [](GLFWwindow* window) {
            EventManager::post<EventWindowClosing>(window);
            });*/

        glfwSetWindowSizeLimits(this->m_window, 720, 480, GLFW_DONT_CARE, GLFW_DONT_CARE);

        glfwShowWindow(this->m_window);
    }

    void Window::initImGui() {
        IMGUI_CHECKVERSION();

        //auto fonts = View::getFontAtlas();

        //GImGui = ImGui::CreateContext(fonts);
        GImGui = ImGui::CreateContext();


        ImGuiIO& io = ImGui::GetIO();
        ImGuiStyle& style = ImGui::GetStyle();

        style.Alpha = 1.0F;
        style.WindowRounding = 0.0F;

        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable | ImGuiConfigFlags_NavEnableKeyboard | ImGuiConfigFlags_ViewportsEnable;

        ImGui::StyleColorsDark();

        //io.ConfigViewportsNoTaskBarIcon = false;


        //auto scale = Whale::System::getGlobalScale();
        //style.ScaleAllSizes(scale);
        //io.DisplayFramebufferScale = ImVec2(scale, scale);

        {
            GLsizei width, height;
            unsigned char* fontData;

            io.Fonts->GetTexDataAsRGBA32(&fontData, &width, &height);

            // Create new font atlas
            GLuint tex;
            glGenTextures(1, &tex);
            glBindTexture(GL_TEXTURE_2D, tex);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA8, GL_UNSIGNED_INT, fontData);
            io.Fonts->SetTexID(reinterpret_cast<ImTextureID>(tex));
        }

        style.WindowMenuButtonPosition = ImGuiDir_None;
        style.IndentSpacing = 10.0F;

        // Install custom settings handler
       /* ImGuiSettingsHandler handler;
        handler.TypeName = "ImHex";
        handler.TypeHash = ImHashStr("ImHex");
        handler.ReadOpenFn = ImHexSettingsHandler_ReadOpenFn;
        handler.ReadLineFn = ImHexSettingsHandler_ReadLine;
        handler.WriteAllFn = ImHexSettingsHandler_WriteAll;
        handler.UserData = this;
        ImGui::GetCurrentContext()->SettingsHandlers.push_back(handler);

        for (const auto& dir : fs::getDefaultPaths(fs::ImHexPath::Config)) {
            if (std::fs::exists(dir) && fs::isPathWritable(dir)) {
                this->m_imguiSettingsPath = dir / "interface.ini";
                io.IniFilename = nullptr;
                break;
            }
        }

        if (!this->m_imguiSettingsPath.empty() && fs::exists(this->m_imguiSettingsPath))
            ImGui::LoadIniSettingsFromDisk(this->m_imguiSettingsPath.string().c_str());*/

        ImGui_ImplGlfw_InitForOpenGL(this->m_window, true);
        ImGui_ImplOpenGL3_Init("#version 130");
    }

    void Window::exitGLFW() {
        glfwDestroyWindow(this->m_window);
        glfwTerminate();
    }

    void Window::exitImGui() {
        //ImGui::SaveIniSettingsToDisk(this->m_imguiSettingsPath.string().c_str());

        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

}
