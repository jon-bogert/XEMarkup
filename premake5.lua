workspace "Markup"
architecture "x64"
    configurations { "Debug", "Release" }
    outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

project "Core"
    location "%{prj.name}"
    kind "ConsoleApp"
    language "C++"
    targetname "%{prj.name}"
    targetdir ("bin/".. outputdir)
    objdir ("%{prj.name}/int/" .. outputdir)
    cppdialect "C++17"
    staticruntime "Off"

    files
    {
        "%{prj.name}/**.h",
        "%{prj.name}/**.c",
        "%{prj.name}/**.hpp",
        "%{prj.name}/**.cpp"
    }

    includedirs
    {
        "%{prj.name}/include",
        "%{prj.name}/src",
        "XEMarkup-Common/include",
        "XEMarkup-YAML/include",
        "XEMarkup-JSON/include",
        "XEMarkup-BSON/include"
    }

    libdirs "%{prj.name}/lib"

    links
    {
        "XEMarkup-YAML",
        "XEMarkup-JSON",
        "XEMarkup-BSON"
    }

    filter "system:windows"
		systemversion "latest"
		defines { "WIN32" }

	filter "configurations:Debug"
		defines { "_DEBUG", "_CONSOLE" }
		symbols "On"

    filter "configurations:Release"
		defines { "NDEBUG", "_CONSOLE" }
		optimize "On"



project "XEMarkup-YAML"
    location "%{prj.name}"
    kind "StaticLib"
    language "C++"
    targetname "%{prj.name}"
    targetdir ("bin/".. outputdir)
    objdir ("%{prj.name}/int/" .. outputdir)
    cppdialect "C++17"
    staticruntime "Off"

    files
    {
        "%{prj.name}/**.h",
        "%{prj.name}/**.c",
        "%{prj.name}/**.hpp",
        "%{prj.name}/**.cpp",
        "ext/yaml-cpp/**.cpp",
        "ext/yaml-cpp/**.h"
    }

    includedirs
    {
        "%{prj.name}/include",
        "ext/yaml-cpp/include",
        "ext/yaml-cpp/src",
        "%{prj.name}/src",
        "XEMarkup-Common/include"
    }
    defines
    {
        "YAML_CPP_STATIC_DEFINE"
    }

    libdirs "%{prj.name}/lib"

    filter "system:windows"
        systemversion "latest"
        defines { "WIN32" }

    filter "configurations:Debug"
        defines { "_DEBUG", "_CONSOLE" }
        symbols "On"

    filter "configurations:Release"
        defines { "NDEBUG", "_CONSOLE" }
        optimize "On"

project "XEMarkup-JSON"
    location "%{prj.name}"
    kind "StaticLib"
    language "C++"
    targetname "%{prj.name}"
    targetdir ("bin/".. outputdir)
    objdir ("%{prj.name}/int/" .. outputdir)
    cppdialect "C++17"
    staticruntime "Off"

    files
    {
        "%{prj.name}/**.h",
        "%{prj.name}/**.c",
        "%{prj.name}/**.hpp",
        "%{prj.name}/**.cpp",
        "ext/nlohmann/**.hpp",
    }

    includedirs
    {
        "%{prj.name}/include",
        "ext/nlohmann/include",
        "%{prj.name}/src",
        "XEMarkup-Common/include"
    }

    libdirs "%{prj.name}/lib"

    filter "system:windows"
        systemversion "latest"
        defines { "WIN32" }

    filter "configurations:Debug"
        defines { "_DEBUG", "_CONSOLE" }
        symbols "On"

    filter "configurations:Release"
        defines { "NDEBUG", "_CONSOLE" }
        optimize "On"

project "XEMarkup-BSON"
    location "%{prj.name}"
    kind "StaticLib"
    language "C++"
    targetname "%{prj.name}"
    targetdir ("bin/".. outputdir)
    objdir ("%{prj.name}/int/" .. outputdir)
    cppdialect "C++17"
    staticruntime "Off"

    files
    {
        "%{prj.name}/**.h",
        "%{prj.name}/**.c",
        "%{prj.name}/**.hpp",
        "%{prj.name}/**.cpp",
        "ext/nlohmann/**.hpp",
    }

    includedirs
    {
        "%{prj.name}/include",
        "ext/nlohmann/include",
        "%{prj.name}/src",
        "XEMarkup-Common/include"
    }

    libdirs "%{prj.name}/lib"

    filter "system:windows"
        systemversion "latest"
        defines { "WIN32" }

    filter "configurations:Debug"
        defines { "_DEBUG", "_CONSOLE" }
        symbols "On"

    filter "configurations:Release"
        defines { "NDEBUG", "_CONSOLE" }
        optimize "On"