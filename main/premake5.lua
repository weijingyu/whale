
project "main"
kind "ConsoleApp"
language "C++"
cppdialect "C++20"
staticruntime "off"

targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

files
{
    "src/**.h",
    "src/**.cpp"
}

includedirs
{
    "%{wks.location}/whale/external/spdlog/include",
    "%{wks.location}/whale/src",
    "%{wks.location}/whale/external/glfw/include",
    "%{wks.location}/whale/external/imgui"
}

links
{
    "whale",
}

filter "system:windows"
    systemversion "latest"

    defines
    {
        "WH_PLATFORM_WINDOWS"
    }

filter "configurations:Debug"
    defines "WH_DEBUG"
    runtime "Debug"
    symbols "On"

filter "configurations:Release"
    defines "WH_RELEASE"
    runtime "Release"
    optimize "On"

filter "configurations:Dist"
    defines "WH_DIST"
    runtime "Release"
    optimize "On"
