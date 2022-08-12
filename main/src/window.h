#pragma once

#include <filesystem>
#include <memory>
#include <string>
#include <list>
#include <vector>


struct GLFWwindow;
struct ImGuiSettingsHandler;


namespace Whale {

    class Window {
    public:
        Window();
        ~Window();

        void loop();

        static void initNative();

    private:
        void setupNativeWindow();
        void beginNativeWindowFrame();
        void endNativeWindowFrame();
        void drawTitleBar();

        void frameBegin();
        void frame();
        void frameEnd();

        void processEvent() { this->m_hadEvent = true; }

        void initGLFW();
        void initImGui();
        void exitGLFW();
        void exitImGui();

        GLFWwindow* m_window = nullptr;

        std::string m_windowTitle;

        double m_lastFrameTime = 0;

        std::list<std::string> m_popupsToOpen;
        std::vector<int> m_pressedKeys;

        bool m_mouseButtonDown = false;

        bool m_hadEvent = false;
        bool m_frameRateTemporarilyUnlocked = false;
        double m_frameRateUnlockTime = 0;
    };

}