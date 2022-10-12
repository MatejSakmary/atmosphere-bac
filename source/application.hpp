#pragma once

/* Let GLFW include it's own definitions and load Vulkan header with it */
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <memory.h>

#include "vulkan/renderer.hpp"

// const uint32_t WIDTH = 1280;
// const uint32_t HEIGHT = 720;

const uint32_t WIDTH = 1920;
const uint32_t HEIGHT = 1080;
struct GLFWUserData
{
    float lastFrame = 0.0f;
    float mouseLastX = 0.0f;
    float mouseLastY = 0.0f;
    float deltaTime = 0.0f;
    bool flyMode = false;
    bool firstInput = false;
    float flyModeToggleTimeout = 0.0f;
    Renderer *renderer;
};


class Application
{
public:
    bool framebufferResized;
    std::unique_ptr<Renderer> renderer;

    Application();
    ~Application();
    void Run();

private:
    GLFWwindow *window;
    GLFWUserData *glfwData;

    void MainLoop();
    void InitWindow();
};