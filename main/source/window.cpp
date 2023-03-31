#include <window.h>

#include <chrono>
#include <csignal>
#include <iostream>
#include <set>
#include <thread>
#include <cassert>


#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <GLFW/glfw3.h>


#include <fs.h>
#include <log.h>

namespace whale {

    using namespace std::literals::chrono_literals;

    Window::Window() {

        this->initGLFW();
        this->initImGui();

        m_pdx = &PDX::get();
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

            // const auto targetFps = ImHexApi::System::getTargetFPS();
            static float targetFPS = 60.0F;
            if (targetFPS <= 200)
                std::this_thread::sleep_for(std::chrono::milliseconds((unsigned long long)((this->m_lastFrameTime + 1 / targetFPS - glfwGetTime()) * 1000)));

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
        ImGui::SetNextWindowSize(viewport->Size - ImVec2(0, ImGui::GetTextLineHeightWithSpacing()));
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

        ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoBringToFrontOnFocus |
            ImGuiWindowFlags_NoNavFocus |
            ImGuiWindowFlags_NoBackground |
            ImGuiWindowFlags_NoDocking |
            // ImGuiWindowFlags_MenuBar |
            ImGuiWindowFlags_NoCollapse;

        ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;


        if (ImGui::Begin("DockSpaceWindow", nullptr, windowFlags)) {

            ImGui::PopStyleVar();

            ImGuiID dockSpaceId = ImGui::GetID("WhaleDockSpace");
            if (ImGui::DockSpace(dockSpaceId, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode)) {
                ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 2.0f);

                if (ImGui::BeginMainMenuBar()) {

                    if (ImGui::BeginMenu("File")) {
                        if (ImGui::MenuItem("Open Log File")) {
                            fs::openFileBrowser(fs::DialogMode::Open, {}, [this](const std::fs::path& path) {
                                this->m_busLog = new BusLog(path);
                                this->m_reloadBusLog = true;
                                });
                        }
                        ImGui::EndMenu();
                    }

                    if (ImGui::BeginMenu("Project")) {
                        if (ImGui::MenuItem("Import PDX project...")) {
                            fs::openFileBrowser(fs::DialogMode::Folder, {}, [this](const std::fs::path& path) {
                                this->m_pdx->init(path);
                                });
                        }
                        ImGui::EndMenu();
                    }

                    if (ImGui::BeginMenu("View")) {
                        if (ImGui::MenuItem("Show Demo Window", nullptr, &this->m_showDemoWindow)) {}
                        if (ImGui::MenuItem("Trace Browser", nullptr, &this->m_showTraceBrowser)) {}
                        if (ImGui::MenuItem("Odx Browser", nullptr, &this->m_showOdxBrowser)) {}
                        ImGui::EndMenu();
                    }

                    if (ImGui::BeginMenu("Theme")) {
                        if (ImGui::MenuItem("Light")) {
                            ImGui::StyleColorsLight();
                        }
                        if (ImGui::MenuItem("Dark")) {
                            ImGui::StyleColorsDark();
                            setDarkThemeColors();
                        }
                        if (ImGui::MenuItem("Classic")) {
                            ImGui::StyleColorsClassic();
                        }
                        ImGui::EndMenu();
                    }

                    ImGui::EndMainMenuBar();
                }
                ImGui::PopStyleVar();
            }

            ImGui::End();
        }

        ImGui::PopStyleVar(2);
    }

    void Window::frame() {


        auto& io = ImGui::GetIO();

        if (this->m_showDemoWindow) {
            ImGui::ShowDemoWindow(&this->m_showDemoWindow);
        }
        if (this->m_showTraceBrowser) {
            this->showTraceBrowser(&this->m_showTraceBrowser);
        }
        /*if (this->m_showOdxBrowser) {
            this->showOdxBrowser(&this->m_showOdxBrowser);
        }*/
    }

    // Demonstrate create a window with multiple child windows.
    void Window::showTraceBrowser(bool* p_open)
    {
        static std::string selectedEcu;
        static std::string prevSelectedEcu;
        static std::vector<std::string> ecuList;

        if (m_reloadBusLog) {
            ecuList = this->m_busLog->getEcuList();
            this->m_reloadBusLog = false;
        }
        //ImGui::SetNextWindowSize(ImVec2(500, 440), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("ECU List", nullptr, true))
        {
            // Left
            static int selected = 0;
            if (this->m_busLog != nullptr) {
                {
                    ImGui::BeginChild("left pane", ImVec2(0, 0), true);

                    for (auto& ecuName : ecuList) {
                        if (ImGui::Selectable(ecuName.c_str(), selectedEcu == ecuName)) {
                            selectedEcu = ecuName;
                        }
                    }
                    ImGui::EndChild();
                }
                ImGui::SameLine();
            }
            ImGui::End();
        }

        static EcuTrace ecuTrace;
        //WH_INFO("Selected: {}, prev: {}", selectedEcu, prevSelectedEcu);
        if (prevSelectedEcu != selectedEcu) {
            ecuTrace = this->m_busLog->ecuTrace(selectedEcu);
            prevSelectedEcu = selectedEcu;
        }

        if (ImGui::Begin("Trace Browser"), nullptr, true) {

            // Right
            if (selectedEcu != "") {
                static ImGuiTableFlags flags =
                    ImGuiTableFlags_Resizable    |
                    ImGuiTableFlags_Reorderable  |
                    ImGuiTableFlags_ScrollY      |
                    ImGuiTableFlags_Hideable     |
                    ImGuiTableFlags_RowBg        |
                    ImGuiTableFlags_BordersOuter |
                    ImGuiTableFlags_BordersH     |
                    ImGuiTableFlags_BordersV;

                if (ImGui::BeginTable("TraceTable", 4, flags)) {
                    ImGui::TableSetupScrollFreeze(0, 1);
                    ImGui::TableSetupColumn("Time");
                    ImGui::TableSetupColumn("ID");
                    ImGui::TableSetupColumn("Trace");
                    ImGui::TableSetupColumn("Text");
                    ImGui::TableHeadersRow();

                    for (auto& trace : ecuTrace.traces) {
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::Text(trace.time.c_str());
                        ImGui::TableSetColumnIndex(1);
                        ImGui::Text(trace.id.c_str());
                        ImGui::TableSetColumnIndex(2);
                        ImGui::Text(trace.trace.c_str());
                        ImGui::TableSetColumnIndex(3);
                        ImGui::Text(trace.hexTrace.c_str());
                    }
                    ImGui::EndTable();
                }
            }
        }
        ImGui::End();
    }

    void Window::showOdxBrowser(bool* p_open)
    {

        if (this->m_pdx->isLoaded() && ImGui::Begin("Odx Browser", nullptr, true))
        {
            ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);

            static std::string selectedBv;

            // Child 1: no border, enable horizontal scrollbar
            {
                ImGuiWindowFlags window_flags = ImGuiWindowFlags_None;
                ImGui::BeginChild("ChildL", ImVec2(200, ImGui::GetContentRegionAvail().y), false, window_flags);
                
                // Vehicle Info
                static int selected = 0;
                auto vehicleInfos = this->m_pdx->getVehicleInfoShortNames();
                static int vi_current_idx = 0; // Here we store our selection data as an index.
                String& combo_preview_value = vehicleInfos[vi_current_idx];  // Pass in the preview value visible before opening the combo (it could be anything)

                if (ImGui::BeginCombo("Vehicle Info", combo_preview_value.c_str(), 0))
                {
                    for (int n = 0; n < vehicleInfos.size(); n++)
                    {
                        const bool is_selected = (vi_current_idx == n);
                        if (ImGui::Selectable(vehicleInfos[n].c_str(), is_selected))
                            vi_current_idx = n;

                        // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                        if (is_selected)
                            ImGui::SetItemDefaultFocus();
                    }
                    ImGui::EndCombo();
                }
                auto bvs = this->m_pdx->getBvShortNamesByVehicleInfoId(combo_preview_value);
                WH_INFO("Read base variant from {}", combo_preview_value);
                if (bvs.size() > 0) {
                    {
                        ImGui::BeginChild("Logical Links", ImVec2(0, 0), true);

                        for (auto& bv : bvs) {
                            if (ImGui::Selectable(bv.c_str(), selectedBv == bv)) {
                                selectedBv = bv;
                            }
                        }
                        ImGui::EndChild();
                    }
                    ImGui::SameLine();
                }
                //ImGui::End();
                ImGui::EndChild();
            }

            ImGui::SameLine();

            static String prevBv;
            static int ev_current_idx = 0; // Here we store our selection data as an index.
            static Vec<String> subEvs;
            // Child 2: rounded border
            if (!selectedBv.empty()) {
                ImGuiWindowFlags window_flags = ImGuiWindowFlags_None;
                ImGui::BeginChild("ChildR", ImVec2(0, ImGui::GetContentRegionAvail().y), true, window_flags);

                // Ecu Variant
                static int selectedEv = 0;
                if (prevBv != selectedBv) {
                    WH_INFO("Another BV {} selected, update EVs:", selectedBv);
                    // subEvs.clear();
                    subEvs = this->m_pdx->getEvShortNamesByBvId(selectedBv);
                    ev_current_idx = 0;
                }
                String& ev_preview_value = subEvs[ev_current_idx];  // Pass in the preview value visible before opening the combo (it could be anything)

                if (ImGui::BeginCombo("Vehicle Info", ev_preview_value.c_str(), 0))
                {
                    for (int n = 0; n < subEvs.size(); n++)
                    {
                        const bool is_selected = (ev_current_idx == n);
                        if (ImGui::Selectable(subEvs[n].c_str(), is_selected))
                            ev_current_idx = n;

                        // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                        if (is_selected)
                            ImGui::SetItemDefaultFocus();
                    }
                    ImGui::EndCombo();
                }
                ImGui::EndChild();
            }
            ImGui::PopStyleVar();
            ImGui::End();
        }
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
        // setDarkThemeColors();

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

        // glfwWindowHint(GLFW_DECORATED, ImHexApi::System::isBorderlessWindowModeEnabled() ? GL_FALSE : GL_TRUE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

        this->m_windowTitle = "Hey, Whale";
        this->m_window = glfwCreateWindow(1280, 720, this->m_windowTitle.c_str(), nullptr, nullptr);
        WH_INFO("GLFW window created.");
        glfwSetWindowUserPointer(this->m_window, this);

        if (this->m_window == nullptr) {
            spdlog::critical("Failed to create window!");
            std::abort();
        }

        glfwMakeContextCurrent(this->m_window);
        glfwSwapInterval(1);

        glfwSetWindowSizeLimits(this->m_window, 720, 480, GLFW_DONT_CARE, GLFW_DONT_CARE);

        glfwShowWindow(this->m_window);
    }

    void Window::initImGui() {
        IMGUI_CHECKVERSION();

        // auto fonts = View::getFontAtlas();

        // GImGui   = ImGui::CreateContext(fonts);

        GImGui = ImGui::CreateContext();

        ImGuiIO& io = ImGui::GetIO();

        float fontSize = 16.0f;// *2.0f;
        // io.Fonts->AddFontFromFileTTF("../assets/fonts/Ubuntu-Light.ttf", fontSize);
        // io.Fonts->AddFontFromFileTTF("../assets/fonts/CascadiaMono-Light.ttf", fontSize);
        // io.Fonts->AddFontFromFileTTF("../assets/fonts/opensans/OpenSans-Regular.ttf", fontSize);
        // io.Fonts->AddFontFromFileTTF("../assets/fonts/opensans/OpenSans-Light.ttf", fontSize);
        // io.FontDefault = io.Fonts->AddFontFromFileTTF("assets/fonts/opensans/OpenSans-Regular.ttf", fontSize);
        // ImFont* font = io.Fonts->AddFontFromFileTTF("../assets/fonts/opensans/OpenSans-Regular.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesChineseSimplifiedCommon());


        ImGuiStyle& style = ImGui::GetStyle();

        style.Alpha = 1.0F;
        style.WindowRounding = 0.0F;

        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

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

        // style.WindowMenuButtonPosition = ImGuiDir_None;
        style.IndentSpacing = 10.0F;

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

    void Window::loadLogFile(const std::filesystem::path& logFilePath) {
        this->m_busLog = new BusLog(logFilePath);
    }

}
