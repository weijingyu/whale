#include <window.h>

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

#include <spdlog/spdlog.h>

namespace Whale {

    using namespace std::literals::chrono_literals;

    // static void signalHandler(int signalNumber) {
    //     // log::fatal("Terminating with signal {}", signalNumber);

    //     // EventManager::post<EventAbnormalTermination>(signalNumber);

    //     if (std::uncaught_exceptions() > 0) {
    //         log::fatal("Uncaught exception thrown!");
    //     }

    //     // Let's not loop on this...
    //     std::signal(signalNumber, nullptr);

    //     #if defined(DEBUG)
    //         assert(false);
    //     #else
    //         std::raise(signalNumber);
    //     #endif
    // };

    Window::Window() {

        this->initGLFW();
        this->initImGui();
        // this->setupNativeWindow();

        // Initialize default theme
        // EventManager::post<RequestChangeTheme>(1);

        // EventManager::subscribe<RequestCloseImHex>(this, [this](bool noQuestions) {
        //     glfwSetWindowShouldClose(this->m_window, GLFW_TRUE);

        //     if (!noQuestions)
        //         EventManager::post<EventWindowClosing>(this->m_window);
        // });

        // EventManager::subscribe<RequestChangeWindowTitle>(this, [this](const std::string &windowTitle) {
        //     std::string title = "ImHex";

        //     if (ImHexApi::Provider::isValid()) {
        //         auto provider = ImHexApi::Provider::get();
        //         if (!windowTitle.empty())
        //             title += " - " + windowTitle;

        //         if (provider->isDirty())
        //             title += " (*)";

        //         if (!provider->isWritable())
        //             title += " (Read Only)";
        //     }

        //     this->m_windowTitle = title;
        //     glfwSetWindowTitle(this->m_window, title.c_str());
        // });

        // constexpr auto CrashBackupFileName = "crash_backup.hexproj";

        // EventManager::subscribe<EventAbnormalTermination>(this, [this, CrashBackupFileName](int) {
        //     ImGui::SaveIniSettingsToDisk(this->m_imguiSettingsPath.string().c_str());

        //     if (!ImHexApi::Provider::isDirty())
        //         return;

        //     for (const auto &path : fs::getDefaultPaths(fs::ImHexPath::Config)) {
        //         if (ProjectFile::store((std::fs::path(path) / CrashBackupFileName).string()))
        //             break;
        //     }
        // });

        // EventManager::subscribe<RequestOpenPopup>(this, [this](auto name) {
        //     this->m_popupsToOpen.push_back(name);
        // });

        // std::signal(SIGSEGV, signalHandler);
        // std::signal(SIGILL, signalHandler);
        // std::signal(SIGABRT, signalHandler);
        // std::signal(SIGFPE, signalHandler);
        // std::set_terminate([]{ signalHandler(SIGABRT); });

        // auto imhexLogo      = romfs::get("logo.png");
        // this->m_logoTexture = ImGui::LoadImageFromMemory(reinterpret_cast<const ImU8 *>(imhexLogo.data()), imhexLogo.size());

        // ContentRegistry::Settings::store();
        // EventManager::post<EventSettingsChanged>();
        // EventManager::post<EventWindowInitialized>();
    }

    Window::~Window() {
        // EventManager::unsubscribe<EventProviderDeleted>(this);
        // EventManager::unsubscribe<RequestCloseImHex>(this);
        // EventManager::unsubscribe<RequestChangeWindowTitle>(this);
        // EventManager::unsubscribe<EventAbnormalTermination>(this);
        // EventManager::unsubscribe<RequestOpenPopup>(this);

        this->exitImGui();
        this->exitGLFW();
    }

    void Window::loop() {
        this->m_lastFrameTime = glfwGetTime();
        while (!glfwWindowShouldClose(this->m_window)) {
            if (!glfwGetWindowAttrib(this->m_window, GLFW_VISIBLE) || glfwGetWindowAttrib(this->m_window, GLFW_ICONIFIED)) {
                glfwWaitEvents();
            } else {
                glfwPollEvents();

                 bool frameRateUnlocked = ImGui::IsPopupOpen(ImGuiID(0), ImGuiPopupFlags_AnyPopupId) /* || Task::getRunningTaskCount() > 0 */ || this->m_mouseButtonDown || this->m_hadEvent || !this->m_pressedKeys.empty();
                 const double timeout = std::max(0.0, (1.0 / 5.0) - (glfwGetTime() - this->m_lastFrameTime));

                 if ((this->m_lastFrameTime - this->m_frameRateUnlockTime) > 5 && this->m_frameRateTemporarilyUnlocked && !frameRateUnlocked) {
                     this->m_frameRateTemporarilyUnlocked = false;
                 }

                 if (frameRateUnlocked || this->m_frameRateTemporarilyUnlocked) {
                     if (!this->m_frameRateTemporarilyUnlocked) {
                         this->m_frameRateTemporarilyUnlocked = true;
                         this->m_frameRateUnlockTime = this->m_lastFrameTime;
                     }
                 } else {
                     glfwWaitEventsTimeout(timeout);
                 }
            }


            this->frameBegin();
            this->frame();
            this->frameEnd();

            // const auto targetFps = ImHexApi::System::getTargetFPS();
            // if (targetFps <= 200)
            //     std::this_thread::sleep_for(std::chrono::milliseconds(u64((this->m_lastFrameTime + 1 / targetFps - glfwGetTime()) * 1000)));

            this->m_lastFrameTime = glfwGetTime();

            this->m_hadEvent = false;
        }
    }

    

    void Window::frameBegin() {

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGuiViewport *viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->Size);
        ImGui::SetNextWindowViewport(viewport->ID);
        //ImGui::SetNextWindowViewport(viewport->ID - ImGui::GetStyle().FramePadding.y);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

        //ImGuiWindowFlags windowFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;

        ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoTitleBar |
                                       ImGuiWindowFlags_NoResize |
                                       ImGuiWindowFlags_NoMove |
                                       ImGuiWindowFlags_NoBringToFrontOnFocus |
                                       ImGuiWindowFlags_NoNavFocus |
                                       ImGuiWindowFlags_NoBackground |
                                       ImGuiWindowFlags_NoDocking |
                                       ImGuiWindowFlags_MenuBar |
                                       ImGuiWindowFlags_NoCollapse;

        //ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

        static bool dockSpaceOpen = true;

        if (ImGui::Begin("DockSpaceWindow", nullptr, windowFlags)) {

            ImGui::PopStyleVar();

            ImGuiID dockSpaceId = ImGui::GetID("WhaleDockSpace");
            
            if (ImGui::DockSpace(dockSpaceId, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode)) {
                ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.0f);

                if (ImGui::BeginMenuBar()) {

                    if (ImGui::BeginMenu("File")) {
                        if (ImGui::MenuItem("Open Log File")) {

                        }
                        ImGui::EndMenu();
                    }

                    if (ImGui::BeginMenu("Project")) {
                        ImGui::EndMenu();
                    }

                    if (ImGui::BeginMenu("View")) {
                        ImGui::MenuItem("ImGui Demo");
                        ImGui::EndMenu();
                    }

                    ImGui::EndMenuBar();
                }
                ImGui::PopStyleVar();
            }

           ImGui::End();
        }
        
        ImGui::PopStyleVar(2);

        /*this->m_popupsToOpen.remove_if([](const auto &name) {
            if (ImGui::IsPopupOpen(name.c_str()))
                return true;
            else
                ImGui::OpenPopup(name.c_str());

            return false;
        });*/

        // EventManager::post<EventFrameBegin>();
    }

    void Window::frame() {


        auto &io = ImGui::GetIO();
        /*
        for (auto &[name, view] : ContentRegistry::Views::getEntries()) {
            ImGui::GetCurrentContext()->NextWindowData.ClearFlags();

            view->drawAlwaysVisible();

            if (!view->shouldProcess())
                continue;

            if (view->isAvailable()) {
                ImGui::SetNextWindowSizeConstraints(scaled(view->getMinSize()), scaled(view->getMaxSize()));
                view->drawContent();
            }

            if (view->getWindowOpenState()) {
                auto window    = ImGui::FindWindowByName(view->getName().c_str());
                bool hasWindow = window != nullptr;
                bool focused   = false;


                if (hasWindow && !(window->Flags & ImGuiWindowFlags_Popup)) {
                    ImGui::Begin(View::toWindowName(name).c_str());

                    focused = ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows | ImGuiFocusedFlags_NoPopupHierarchy);
                    ImGui::End();
                }

                for (const auto &key : this->m_pressedKeys) {
                    ShortcutManager::process(view, io.KeyCtrl, io.KeyAlt, io.KeyShift, io.KeySuper, focused, key);
                }
            }
        }
        */

        ImGui::ShowDemoWindow(&show_demo_window);
        //for (const auto &key : this->m_pressedKeys) {
        //    // ShortcutManager::processGlobals(io.KeyCtrl, io.KeyAlt, io.KeyShift, io.KeySuper, key);
        //}

        //this->m_pressedKeys.clear();
    }

    void Window::frameEnd() {
        // EventManager::post<EventFrameEnd>();

        // this->endNativeWindowFrame();
        ImGui::Render();

        int displayWidth, displayHeight;
        glfwGetFramebufferSize(this->m_window, &displayWidth, &displayHeight);
        //spdlog::info("Frame buffer size: {}, {}", displayWidth, displayHeight);
        glViewport(0, 0, displayWidth, displayHeight);
        glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        setDarkThemeColors();

        GLFWwindow *backup_current_context = glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(backup_current_context);

        glfwSwapBuffers(this->m_window);
    }

    void Window::initGLFW() {
        glfwSetErrorCallback([](int error, const char *desc) {
            spdlog::error("GLFW Error [{}] : {}", error, desc);
        });

        if (!glfwInit()) {
            spdlog::critical("Failed to initialize GLFW!");
            std::abort();
        }

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

        // glfwWindowHint(GLFW_DECORATED, ImHexApi::System::isBorderlessWindowModeEnabled() ? GL_FALSE : GL_TRUE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

        this->m_windowTitle = "Whale";
        this->m_window = glfwCreateWindow(1280, 720, this->m_windowTitle.c_str(), nullptr, nullptr);

        glfwSetWindowUserPointer(this->m_window, this);

        if (this->m_window == nullptr) {
            spdlog::critical("Failed to create window!");
            std::abort();
        }

        glfwMakeContextCurrent(this->m_window);
        glfwSwapInterval(1);

         /*GLFWmonitor *monitor = glfwGetPrimaryMonitor();
         if (monitor != nullptr) {
             const GLFWvidmode *mode = glfwGetVideoMode(monitor);
             if (mode != nullptr) {
                 int monitorX, monitorY;
                 glfwGetMonitorPos(monitor, &monitorX, &monitorY);

                 int windowWidth, windowHeight;
                 glfwGetWindowSize(this->m_window, &windowWidth, &windowHeight);

                 glfwSetWindowPos(this->m_window, monitorX + (mode->width - windowWidth) / 2, monitorY + (mode->height - windowHeight) / 2);
             }
         }*/

        //{
        //    int x = 0, y = 0;
        //    glfwGetWindowPos(this->m_window, &x, &y);

        //    // ImHexApi::System::impl::setMainWindowPosition(x, y);
        //}

        //{
        //    int width = 0, height = 0;
        //    glfwGetWindowSize(this->m_window, &width, &height);
        //    glfwSetWindowSize(this->m_window, width, height);
        //    // ImHexApi::System::impl::setMainWindowSize(width, height);
        //}

        //glfwSetWindowPosCallback(this->m_window, [](GLFWwindow *window, int x, int y) {
        //    // ImHexApi::System::impl::setMainWindowPosition(x, y);

        //    if (auto g = ImGui::GetCurrentContext(); g == nullptr || g->WithinFrameScope) return;

        //    auto win = static_cast<Window *>(glfwGetWindowUserPointer(window));
        //    win->frameBegin();
        //    win->frame();
        //    win->frameEnd();
        //    win->processEvent();
        //});

        //glfwSetWindowSizeCallback(this->m_window, [](GLFWwindow *window, int width, int height) {
        //    // if (!glfwGetWindowAttrib(window, GLFW_ICONIFIED))
        //    //     ImHexApi::System::impl::setMainWindowSize(width, height);

        //    if (auto g = ImGui::GetCurrentContext(); g == nullptr || g->WithinFrameScope) return;

        //    auto win = static_cast<Window *>(glfwGetWindowUserPointer(window));
        //    win->frameBegin();
        //    win->frame();
        //    win->frameEnd();
        //    win->processEvent();
        //});

        //glfwSetMouseButtonCallback(this->m_window, [](GLFWwindow *window, int button, int action, int mods) {
        //    // hex::unused(button, mods);
        //    (void) button;
        //    (void) mods;

        //    auto win = static_cast<Window *>(glfwGetWindowUserPointer(window));

        //    if (action == GLFW_PRESS)
        //        win->m_mouseButtonDown = true;
        //    else if (action == GLFW_RELEASE)
        //        win->m_mouseButtonDown = false;
        //    win->processEvent();
        //});

        //glfwSetKeyCallback(this->m_window, [](GLFWwindow *window, int key, int scancode, int action, int mods) {
        //    // hex::unused(mods);
        //    (void) mods;

        //    auto keyName = glfwGetKeyName(key, scancode);
        //    if (keyName != nullptr)
        //        key = std::toupper(keyName[0]);

        //    auto win = static_cast<Window *>(glfwGetWindowUserPointer(window));

        //    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        //        win->m_pressedKeys.push_back(key);
        //    }
        //    win->processEvent();
        //});

        //glfwSetDropCallback(this->m_window, [](GLFWwindow *, int count, const char **paths) {
        //    if (count != 1)
        //        return;

        //    for (int i = 0; i < count; i++) {
        //        auto path = std::filesystem::path(reinterpret_cast<const char *>(paths[i]));

        //        // bool handled = false;
        //        // for (const auto &[extensions, handler] : ContentRegistry::FileHandler::getEntries()) {
        //        //     for (const auto &extension : extensions) {
        //        //         if (path.extension() == extension) {
        //        //             if (!handler(path))
        //        //                 log::error("Handler for extensions '{}' failed to process file!", extension);

        //        //             handled = true;
        //        //             break;
        //        //         }
        //        //     }
        //        // }
        //        
        //        // set file path to this

        //        // if (!handled)
        //        //     EventManager::post<RequestOpenFile>(path);
        //    }
        //});

        //glfwSetCursorPosCallback(this->m_window, [](GLFWwindow *window, double x, double y) {
        //    // hex::unused(x, y);
        //    (void) x;
        //    (void) y;

        //    auto win = static_cast<Window *>(glfwGetWindowUserPointer(window));
        //    win->processEvent();
        //});

        //glfwSetWindowCloseCallback(this->m_window, [](GLFWwindow *window) {
        //    // EventManager::post<EventWindowClosing>(window);
        //});

        glfwSetWindowSizeLimits(this->m_window, 720, 480, GLFW_DONT_CARE, GLFW_DONT_CARE);

        glfwShowWindow(this->m_window);
    }

    void Window::initImGui() {
        IMGUI_CHECKVERSION();

        // auto fonts = View::getFontAtlas();

        // GImGui   = ImGui::CreateContext(fonts);

        GImGui   = ImGui::CreateContext();

        ImGuiIO &io       = ImGui::GetIO();

        float fontSize = 18.0f;// *2.0f;
        io.Fonts->AddFontFromFileTTF("assets/fonts/opensans/OpenSans-Bold.ttf", fontSize);
        io.FontDefault = io.Fonts->AddFontFromFileTTF("assets/fonts/opensans/OpenSans-Regular.ttf", fontSize);


        ImGuiStyle &style = ImGui::GetStyle();

        style.Alpha          = 1.0F;
        style.WindowRounding = 0.0F;

        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

        {
            GLsizei width, height;
            unsigned char *fontData;

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
        style.IndentSpacing            = 10.0F;
        
        ImGui_ImplGlfw_InitForOpenGL(this->m_window, true);

        ImGui_ImplOpenGL3_Init("#version 410");

    }

    void Window::exitGLFW() {
        glfwDestroyWindow(this->m_window);
        glfwTerminate();
    }

    void Window::exitImGui() {

        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    void Window::setDarkThemeColors()
    {
        auto& colors = ImGui::GetStyle().Colors;
        colors[ImGuiCol_WindowBg] = ImVec4{ 0.1f, 0.105f, 0.11f, 1.0f };

        // Headers
        colors[ImGuiCol_Header] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
        colors[ImGuiCol_HeaderHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
        colors[ImGuiCol_HeaderActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

        // Buttons
        colors[ImGuiCol_Button] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
        colors[ImGuiCol_ButtonHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
        colors[ImGuiCol_ButtonActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

        // Frame BG
        colors[ImGuiCol_FrameBg] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
        colors[ImGuiCol_FrameBgHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
        colors[ImGuiCol_FrameBgActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

        // Tabs
        colors[ImGuiCol_Tab] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
        colors[ImGuiCol_TabHovered] = ImVec4{ 0.38f, 0.3805f, 0.381f, 1.0f };
        colors[ImGuiCol_TabActive] = ImVec4{ 0.28f, 0.2805f, 0.281f, 1.0f };
        colors[ImGuiCol_TabUnfocused] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
        colors[ImGuiCol_TabUnfocusedActive] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };

        // Title
        colors[ImGuiCol_TitleBg] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
        colors[ImGuiCol_TitleBgActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
        colors[ImGuiCol_TitleBgCollapsed] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
    }

}
