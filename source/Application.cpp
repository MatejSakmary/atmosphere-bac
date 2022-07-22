#include "Application.h"


static void mouseCallback(GLFWwindow* window, double x, double y)
{
    GLFWUserData* data = static_cast<GLFWUserData*>(glfwGetWindowUserPointer(window));
    float xoffset;
    float yoffset;
    if(!data->firstInput)
    {
        xoffset = data->mouseLastX - x;
        yoffset = data->mouseLastY - y;
    }else{
        xoffset = 0.0f;
        yoffset = 0.0f;
        data->firstInput = false;
    }

    if(data->flyMode)
    {
        data->mouseLastX = x;
        data->mouseLastY = y;
        data->renderer->camera->updateFrontVec(xoffset, yoffset);
    }
}

static void framebufferResizeCallback(GLFWwindow *window, int width, int height)
{
    /* get pointer to this instance store in initWindow*/
    auto data = reinterpret_cast<GLFWUserData *>(glfwGetWindowUserPointer(window));
    data->renderer->framebufferResized = true;
}

void processInput(GLFWwindow *window)
{
    GLFWUserData* data = static_cast<GLFWUserData*>(glfwGetWindowUserPointer(window));
    float currentFrame = glfwGetTime();
    data->deltaTime = currentFrame - data->lastFrame;
    data->lastFrame = currentFrame;
    data->flyModeToggleTimeout = data->flyModeToggleTimeout - data->deltaTime < 0.0f ? 
        0.0f : data->flyModeToggleTimeout - data->deltaTime;

    /* end program */
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS){
        glfwSetWindowShouldClose(window, true);
    }
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        data->renderer->camera->forward(data->deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        data->renderer->camera->back(data->deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        data->renderer->camera->left(data->deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        data->renderer-> camera->right(data->deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && data->flyMode) {
        data->renderer->camera->up(data->deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS && data->flyMode) {
        data->renderer->camera->down(data->deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS && data->flyModeToggleTimeout == 0.0f) {
        data->flyMode = !data->flyMode;
        if(data->flyMode)
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            data->firstInput = true;
        }else{
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
        data->flyModeToggleTimeout = 0.2f;
    }
}


Application::Application()
{
    glfwData = new GLFWUserData();
    InitWindow();
    /* TODO: Computing should be done at the start of run loop, however
       I need to implement Renderer loading textures after constructing
       which would require rewrite */
    // ComputeTransmittanceLUT();
    // mesh = std::make_shared<Mesh>("models/viking_room.obj");
    renderer = std::make_unique<Renderer>(window);
    glfwData->renderer = renderer.get();
    framebufferResized = false;
};

Application::~Application()
{
    glfwDestroyWindow(window);
    glfwTerminate();
};

void Application::Run()
{
    MainLoop();
};

void Application::MainLoop()
{
    while (!glfwWindowShouldClose(window))
    {
       glfwPollEvents();
       processInput(window);
       renderer->drawComputeFrame();
    }

    /*  Wait for logical device to finish operations ->
     *  Because drawFrame operations are asynchornous when we exit the loop
     *  drawing and presentation operations may still be going on*/
    vkDeviceWaitIdle(renderer->vDevice->device);
}

void Application::InitWindow()
{       
    glfwInit();
    /* Tell GLFW to not create OpenGL context */
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
    /* GLFW allows us to store arbitrary point inside of it*/
    glfwSetWindowUserPointer(window, glfwData);
    glfwSetCursorPosCallback(window, mouseCallback);
    glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
}
