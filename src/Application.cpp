#include "Application.h"
#include "PortalGun.h"
#include <iostream>

Camera Application::camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = 1920.0f / 2.0f;
float lastY = 1080.0f / 2.0f;
bool firstMouse = true;
float deltaTime = 0.0f;
float lastFrame = 0.0f;

Application::Application(int width, int height, const std::string &title)
    : width(width), height(height), title(title), window(nullptr) {
}

Application::~Application() {
    shutdown();
}

bool Application::initialize() {
    // glfw: initialize and configure
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    window = glfwCreateWindow(width, height, title.c_str(), NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(window);
    glfwGetFramebufferSize(window, &width, &height);
    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    if (!gladLoadGL((GLADloadfunc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return false;
    }

    stbi_set_flip_vertically_on_load(true);
    Texture::InitDefaultTextures();

    glEnable(GL_DEPTH_TEST);

    // --- Initialize Core Systems ---
    scene = std::make_unique<Scene>();
    renderer = std::make_unique<Renderer>(width, height);
    renderer->initialize();

    scene->lightPos = glm::vec3(5.0f, 5.0f, 5.0f);
    // --- Load Resources ---
    auto cubeModel = std::make_unique<Model>("resources/obj/wall/cube.obj");
    auto portal_cubeModel = std::make_unique<Model>("resources/obj/portal_cube/portal_cube.obj");
    auto portal_gunModel = std::make_unique<Model>("resources/obj/portal_gun/portal_gun.obj");

    // Store raw pointers for GameObject creation before moving ownership
    Model *cubeModelPtr = cubeModel.get();
    Model *portalCubeModelPtr = portal_cubeModel.get();
    Model *portalGunModelPtr = portal_gunModel.get();

    scene->modelResources.push_back(std::move(cubeModel));
    scene->modelResources.push_back(std::move(portal_cubeModel));
    scene->modelResources.push_back(std::move(portal_gunModel));

    // Portal A on Back Wall
    scene->portalA = std::make_unique<Portal>(cubeModelPtr, width, height);
    scene->portalA->position = glm::vec3(0.0f, 1.0f, -9.8f);
    scene->portalA->scale = glm::vec3(1.8f, 2.7f, 0.005f);

    // Portal B on Left Wall
    scene->portalB = std::make_unique<Portal>(cubeModelPtr, width, height);
    scene->portalB->position = glm::vec3(-9.8f, 1.0f, 0.0f);
    scene->portalB->rotation = glm::vec3(0.0f, 90.0f, 0.0f);
    scene->portalB->scale = glm::vec3(1.8f, 2.7f, 0.005f);

    // Link Portals
    scene->portalA->setLinkedPortal(scene->portalB.get());
    scene->portalB->setLinkedPortal(scene->portalA.get());

    // 2. Floor (Ground)
    auto floor = std::make_unique<GameObject>(cubeModelPtr);
    floor->position = glm::vec3(0.0f, -2.0f, 0.0f);
    floor->scale = glm::vec3(10.0f, 0.1f, 10.0f);
    scene->objects.push_back(std::move(floor));

    // 3. Back Wall
    auto backWall = std::make_unique<GameObject>(cubeModelPtr);
    backWall->position = glm::vec3(0.0f, 3.0f, -10.0f);
    backWall->scale = glm::vec3(10.0f, 5.0f, 0.1f);
    scene->objects.push_back(std::move(backWall));

    // 4. Left Wall
    auto leftWall = std::make_unique<GameObject>(cubeModelPtr);
    leftWall->position = glm::vec3(-10.0f, 3.0f, 0.0f);
    leftWall->scale = glm::vec3(0.1f, 5.0f, 10.0f);
    scene->objects.push_back(std::move(leftWall));

    // 5. Center Cube
    auto centerCube = std::make_unique<GameObject>(portalCubeModelPtr);
    centerCube->position = glm::vec3(-5.0f, 0.0f, -5.0f);
    centerCube->scale = glm::vec3(0.1f);
    scene->objects.push_back(std::move(centerCube));
    // 6. Portal Gun
    scene->portalGun = std::make_unique<PortalGun>(portalGunModelPtr);
    scene->portalGun->position = glm::vec3(0.5f, -0.5f, -1.0f);
    scene->portalGun->scale = glm::vec3(0.05f);

    // Initialize Skybox
    // Initialize Skybox
    std::vector<std::string> faces = {
        "resources/skybox/right.jpg",
        "resources/skybox/left.jpg",
        "resources/skybox/top.jpg",
        "resources/skybox/bottom.jpg",
        "resources/skybox/front.jpg",
        "resources/skybox/back.jpg"
    };
    scene->skybox = std::make_unique<Skybox>(faces);

    return true;
}

void Application::run() {
    while (!glfwWindowShouldClose(window)) {
        // per-frame time logic
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        processInput(deltaTime);

        // --- Logic Update ---
        scene->update(deltaTime, camera);

        // render
        renderer->render(*scene, camera);

        // glfw: swap buffers and poll IO events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}

void Application::shutdown() {
    glfwTerminate();
}

void Application::processInput(float deltaTime) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
}

void Application::framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    glViewport(0, 0, width, height);
    Application *app = static_cast<Application *>(glfwGetWindowUserPointer(window));
    if (app) {
        app->width = width;
        app->height = height;
        if (app->renderer) {
            app->renderer->resize(width, height);
        }
    }
}

void Application::mouse_callback(GLFWwindow *window, double xposIn, double yposIn) {
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

void Application::scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

void Application::mouse_button_callback(GLFWwindow *window, int button, int action, int mods) {
    Application *app = static_cast<Application *>(glfwGetWindowUserPointer(window));
    if (app && button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        if (app->scene && app->scene->portalGun) {
            app->scene->portalGun->fire();
        }
    }
}
