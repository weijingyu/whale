workspace "whale"
    architecture "x64"
    startproject "main"

    configurations
    {
        "Debug",
        "Release",
        "Dist"
    }

outputdir = "%{cfg.buildcfg}"

group "Dependencies"
    include "whale/external/glfw"
    include "whale/external/imgui"
group ""

include "whale"
include "main"