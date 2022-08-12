#pragma once
#include <filesystem>


using ImGuiID = unsigned int;
using u32 = unsigned int;
struct ImVec2;

namespace Whale
{

	namespace System
	{

		void setMainWindowPosition(u32 x, u32 y);
		void setMainWindowSize(u32 width, u32 height);
		void setMainDockSpaceId(ImGuiID id);

		void setGlobalScale(float scale);
		void setNativeScale(float scale);

		void setCustomFontPath(const std::filesystem::path& path);
		void setFontSize(float size);

		void setBorderlessWindowMode(bool enabled);

		enum class Theme {
			Dark = 1,
			Light = 2,
			Classic = 3
		};

		ImVec2 getMainWindowPosition();
		ImVec2 getMainWindowSize();
		ImGuiID getMainDockSpaceId();

		const std::filesystem::path& getCustomFontPath();
		float getFontSize();

		bool isBorderlessWindowModeEnabled();

		void setTheme(Theme theme);
		Theme getTheme();

	}
} // namespace Whale
