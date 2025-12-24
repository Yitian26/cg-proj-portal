#pragma once

#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <string>
#include <memory>

#include "Shader.h"
#include "Camera.h"
#include "Model.h"
#include "Skybox.h"
#include "GameObject.h"
#include "Portal.h"
#include <vector>

class Application {
public:
    Application(int width, int height, const std::string &title);
    ~Application();

    bool initialize();
    void run();
    void shutdown();

private:
    void processInput(float deltaTime);
    void render(float deltaTime);
    void drawScene(Shader &shader, const glm::mat4 &view, const glm::mat4 &projection);
    void renderPortal(Portal *portal, const glm::mat4 &view, const glm::mat4 &projection);

    static void framebuffer_size_callback(GLFWwindow *window, int width, int height);
    static void mouse_callback(GLFWwindow *window, double xpos, double ypos);
    static void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);

    int width, height;
    std::string title;
    GLFWwindow *window;

    std::unique_ptr<Shader> shader;
    std::unique_ptr<Shader> portalShader;
    std::unique_ptr<Skybox> skybox;
    std::unique_ptr<Portal> portalA;
    std::unique_ptr<Portal> portalB;

    // Resource Management
    std::vector<std::unique_ptr<Model>> modelResources;

    // Scene Graph
    std::vector<std::unique_ptr<GameObject>> sceneObjects;

    // Camera
    static Camera camera;

    // Lighting
    glm::vec3 lightPos;
};
