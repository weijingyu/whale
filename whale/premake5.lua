project "whale"
    kind "StaticLib"
    -- kind "SharedLib"
    language "C++"
    cppdialect "C++20"

    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

    files
    {
        "%{prj.name}/src/**.h",
        "%{prj.name}/src/**.cpp"
    }

    includedirs
    {
        "src",
        "external/spdlog/include",
        "external/imgui",
        "external/glfw/include"
    }

    defines
    {
        "_CRT_SECURE_NO_WARNINGS",
        "GLFW_INCLUDE_NONE"
    }

    links
    {
        "glfw",
        "imgui",
        "spdlog",
        "opengl32.lib"
    }


    filter "system:windows"
        systemversion "latest"

        defines
        {
        }

        postbuildcommands
        {
            ("{COPY} %{cfg.buildtarget.relpath} ../bin/" .. outputdir .. "/main/")
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