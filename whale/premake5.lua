project "whale"
    kind "StaticLib"
    -- kind "SharedLib"
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
        "src/include",
        "external/spdlog/include",
        "external/imgui",
        "external/glfw/include"
    }

    defines
    {
        "_CRT_SECURE_NO_WARNINGS",
        -- "GLFW_INCLUDE_NONE"
    }

    links
    {
        "glfw",
        "imgui",
        "opengl32.lib",
        -- "Dwmapi.lib"
    }


    filter "system:windows"
        systemversion "latest"

        defines
        {
            "WH_PLATFORM_WINDOWS"
        }

        -- postbuildcommands
        -- {
        --     ("{COPY} %{cfg.buildtarget.relpath} ../bin/" .. outputdir .. "/main/")
        -- }

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