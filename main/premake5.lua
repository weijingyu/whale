
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
        "%{wks.location}/whale/external/imgui",
        "%{wks.location}/whale/external/imgui/backends"
    }
    
    links
    {
        "whale"
        -- "imgui"
        -- "glfw",
        -- "opengl32.lib"
    }
    
    filter "system:windows"
        systemversion "latest"
    
        defines
        {
            "WHALE_PLATFORM_WINDOWS"
        }
    
    filter "configurations:Debug"
        defines "WHALE_DEBUG"
        runtime "Debug"
        symbols "On"
    
    filter "configurations:Release"
        defines "WHALE_RELEASE"
        runtime "Release"
        optimize "On"
    
    filter "configurations:Dist"
        defines "WHALE_DIST"
        runtime "Release"
        optimize "On"
