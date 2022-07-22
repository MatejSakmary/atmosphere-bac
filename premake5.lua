workspace "AtmosphereRender"
    configurations {"Debug"}
    platforms {"Win", "Linux"}

project "Atmosphere"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    architecture "x86_64"
    files {
        "source/**.c",
        "source/**.cpp",
        "source/**.h",
    }

    includedirs {"source/**", "source"}
    optimize "On"

    filter { "platforms:Win" }
        links {"dependencies/GLFW/lib-vc2022/glfw3.lib", 
               "$(VULKAN_SDK)/lib/vulkan-1.lib"}
        includedirs {
            "$(VULKAN_SDK)/include", 
            "dependencies/GLFW/include",
        }
        system "windows"

    filter { "platforms:Linux" }
        links {"glfw", "vulkan", "dl", "pthread", "X11", "Xxf86vm", "Xrandr", "Xi"}
        system "linux"
        symbols "on"
        buildoptions {"-TINYEXR_USE_THREAD"}