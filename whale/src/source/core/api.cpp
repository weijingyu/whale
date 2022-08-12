#include <core/api.h>
#include <imgui.h>
#include <filesystem>

namespace Whale
{
	namespace System {

		static ImVec2 s_mainWindowPos;
		static ImVec2 s_mainWindowSize;
		void setMainWindowPosition(u32 x, u32 y) {
			s_mainWindowPos = ImVec2(double(x), double(y));
		}

		void setMainWindowSize(u32 width, u32 height) {
			s_mainWindowSize = ImVec2(width, height);
		}

		static ImGuiID s_mainDockSpaceId;
		void setMainDockSpaceId(ImGuiID id) {
			s_mainDockSpaceId = id;
		}


		static float s_globalScale = 1.0;
		void setGlobalScale(float scale) {
			s_globalScale = scale;
		}

		static float s_nativeScale = 1.0;
		void setNativeScale(float scale) {
			s_nativeScale = scale;
		}


		static bool s_borderlessWindowMode;
		void setBorderlessWindowMode(bool enabled) {
			s_borderlessWindowMode = enabled;
		}

		static std::filesystem::path s_customFontPath;
		void setCustomFontPath(const std::filesystem::path& path) {
			s_customFontPath = path;
		}

		static float s_fontSize = 13.0;
		void setFontSize(float size) {
			s_fontSize = size;
		}

		static std::string s_gpuVendor;
		void setGPUVendor(const std::string& vendor) {
			s_gpuVendor = vendor;
		}

		static bool s_portableVersion = false;
		void setPortableVersion(bool enabled) {
			s_portableVersion = enabled;
		}

		ImVec2 getMainWindowPosition() {
			return s_mainWindowPos;
		}

		ImVec2 getMainWindowSize() {
			return s_mainWindowSize;
		}


		ImGuiID getMainDockSpaceId() {
			return s_mainDockSpaceId;
		}

		const std::filesystem::path& getCustomFontPath() {
			return s_customFontPath;
		}

		float getFontSize() {
			return s_fontSize;
		}

		bool isBorderlessWindowModeEnabled() {
			return s_borderlessWindowMode;
		}

		static Theme s_theme;

		void setTheme(Theme theme) {
			s_theme = theme;
		}

		Theme getTheme() {
			return s_theme;
		}

	}
} // namespace Whale
