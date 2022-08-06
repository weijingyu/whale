workspace "whale"
	architecture "x64"

	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}

outputdir = "%{cfg.buildcfg}"

project "whale"
	location "whale"
	kind "SharedLib"
	language "C++"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}

	includedirs
	{
		"%{prj.name}/external/spdlog/include"
	}

	filter "system:windows"
		cppdialect "C++20"
		staticruntime "On"
		systemversion "latest"

		defines
		{
			"WH_PLATFORM_WINDOWS",
			"WH_BUILD_DLL"
		}

		postbuildcommands
		{
			("{COPY} %{cfg.buildtarget.relpath} ../bin/" .. outputdir .. "/main")
		}

	filter "configurations:Debug"
		defines "WH_DEBUG"
		symbols "On"

	filter "configurations:Release"
		defines "WH_RELEASE"
		optimize "On"

	filter "configurations:Dist"
		defines "WH_DIST"
		optimize "On"

project "main"
	location "main"
	kind "ConsoleApp"
	language "C++"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}

	includedirs
	{
		"whale/external/spdlog/include",
		"whale/src"
	}

	links
	{
		"whale"
	}

	filter "system:windows"
		cppdialect "C++20"
		staticruntime "On"
		systemversion "latest"

		defines
		{
			"WH_PLATFORM_WINDOWS"
		}

	filter "configurations:Debug"
		defines "WH_DEBUG"
		symbols "On"

	filter "configurations:Release"
		defines "WH_RELEASE"
		optimize "On"

	filter "configurations:Dist"
		defines "WH_DIST"
		optimize "On"