#pragma once

#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <string>
#include <memory>

#include "Shader.h"
#include "Camera.h"
#include "Scene.h"
#include "Renderer.h"
#include <vector>
#include "InputManager.h"

class Application {
public:
    Application(int width, int height, const std::string &title);
    ~Application();

    bool initialize();
    void run();
    void shutdown();

private:
    void processInput(float deltaTime);

    static void framebuffer_size_callback(GLFWwindow *window, int width, int height);
    static void mouse_callback(GLFWwindow *window, double xpos, double ypos);
    static void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
    static void mouse_button_callback(GLFWwindow *window, int button, int action, int mods);

    int width, height;
    std::string title;
    GLFWwindow *window;

    // Core Systems
    std::unique_ptr<Renderer> renderer;
    std::unique_ptr<Scene> scene;

    // Camera
    static Camera camera;

    // Input
    InputManager input;
};
