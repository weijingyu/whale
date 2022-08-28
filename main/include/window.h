#pragma once

#include "bus_log.h"
#include "pdx.h"
#include "type.h"

#include <memory>
#include <string>
#include <list>
#include <vector>

struct GLFWwindow;
struct ImGuiSettingsHandler;


namespace whale {

    class Window {
    public:
        Window();
        ~Window();

        void loop();

    private:
        void setDarkThemeColors();

        void frameBegin();
        void frame();
        void frameEnd();

        void processEvent() { this->m_hadEvent = true; }

        void initGLFW();
        void initImGui();
        void exitGLFW();
        void exitImGui();

        void showTraceBrowser(bool* p_open);
        void showOdxBrowser(bool* p_open);
        void loadLogFile(const std::filesystem::path &logFilePath);


        GLFWwindow *m_window = nullptr;

        std::string m_windowTitle;

        double m_lastFrameTime = 0;

        // ImGui::Texture m_logoTexture = { nullptr };

        std::list<std::string> m_popupsToOpen;
        std::vector<int> m_pressedKeys;

        std::filesystem::path m_imguiSettingsPath = "imgui.ini";

        bool m_mouseButtonDown = false;

        bool m_hadEvent = false;
        bool m_frameRateTemporarilyUnlocked = false;
        double m_frameRateUnlockTime = 0;

        bool m_showDemoWindow;
        bool m_showTraceBrowser;
        bool m_showOdxBrowser;

        BusLog* m_busLog = nullptr;
        pdx::PDX* m_pdx = nullptr;

    };

}