#include "Application.h"
#include "PortalGun.h"
#include <iostream>
#include "InputManager.h"
#include <cmath>
#include "Trigger.h"
#include <glm/gtc/matrix_transform.hpp>
float lastX = 1920.0f / 2.0f;
float lastY = 1080.0f / 2.0f;
bool firstMouse = true;
float deltaTime = 0.0f;
float lastFrame = 0.0f;

Application::Application(int width, int height, const std::string &title)
    : width(width), height(height), title(title), window(nullptr), fallbackCamera(glm::vec3(0.0f, 0.0f, 3.0f)) {
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

    // Initialize input manager
    input.initialize(window);

    // Initialize Player
    scene->player = std::make_unique<Player>(glm::vec3(0.0f, 20.0f, 0.0f));
    scene->lightPos = glm::vec3(5.0f, 5.0f, 5.0f);

    // --- Load Resources ---
    auto cubeModel = std::make_unique<Model>("resources/obj/wall/cube.obj");
    auto portal_cubeModel = std::make_unique<Model>("resources/obj/portal_cube/portal_cube.obj");
    auto portal_gunModel = std::make_unique<Model>("resources/obj/portal_gun/portal_gun.obj");

    scene->addModelResource("cube", std::move(cubeModel));
    scene->addModelResource("portal_cube", std::move(portal_cubeModel));
    scene->addModelResource("portal_gun", std::move(portal_gunModel));

    // Portal A
    scene->portalA = std::make_unique<Portal>(scene->modelResources["cube"].get(), width, height);
    scene->portalA->position = glm::vec3(100.0f, 1.0f, 101.0f);
    scene->portalA->scale = glm::vec3(1.0f, 1.6f, 0.005f);
    scene->portalA->name = "PortalA";
    scene->portalA->init(scene.get());

    // Portal B
    scene->portalB = std::make_unique<Portal>(scene->modelResources["cube"].get(), width, height);
    scene->portalB->position = glm::vec3(100.0f, 0.0f, 100.0f);
    scene->portalB->rotation = glm::vec3(0.0f, 180.0f, 0.0f);
    scene->portalB->scale = glm::vec3(1.0f, 1.6f, 0.005f);
    scene->portalB->name = "PortalB";
    scene->portalB->init(scene.get());

    // Link Portals
    scene->portalA->setLinkedPortal(scene->portalB.get());
    scene->portalB->setLinkedPortal(scene->portalA.get());

    // 2. Floor (Ground)
    auto floor = std::make_unique<GameObject>(scene->modelResources["cube"].get());
    floor->position = glm::vec3(0.0f, -2.0f, 0.0f);
    floor->scale = glm::vec3(10.0f, 0.1f, 10.0f);
    floor->canOpenPortal = true;
    scene->addPhysics(floor.get(), true);
    scene->addObject("floor", std::move(floor));

    // 2. ceiling
    auto ceil = std::make_unique<GameObject>(scene->modelResources["cube"].get());
    ceil->position = glm::vec3(5.0f, 5.0f, 5.0f);
    ceil->scale = glm::vec3(5.0f, 0.1f, 5.0f);
    ceil->canOpenPortal = true;
    scene->addPhysics(ceil.get(), true);
    scene->addObject("ceiling", std::move(ceil));

    // 3. Back Wall
    auto backWall = std::make_unique<GameObject>(scene->modelResources["cube"].get());
    backWall->position = glm::vec3(0.0f, 2.0f, -10.0f);
    backWall->scale = glm::vec3(10.0f, 5.0f, 0.1f);
    backWall->rotation = glm::vec3(-25.0f, 0.0f, 0.0f);
    backWall->canOpenPortal = true;
    scene->addPhysics(backWall.get(), true);
    scene->addObject("backWall", std::move(backWall));

    // 4. Left Wall
    auto leftWall = std::make_unique<GameObject>(scene->modelResources["cube"].get());
    leftWall->position = glm::vec3(-10.0f, 3.0f, 0.0f);
    leftWall->scale = glm::vec3(0.1f, 5.0f, 10.0f);
    leftWall->canOpenPortal = true;
    scene->addPhysics(leftWall.get(), true);
    scene->addObject("leftWall", std::move(leftWall));

    // 3. Front Wall
    auto frontWall = std::make_unique<GameObject>(scene->modelResources["cube"].get());
    frontWall->position = glm::vec3(0.0f, 3.0f, 10.0f);
    frontWall->scale = glm::vec3(10.0f, 5.0f, 0.1f);
    frontWall->canOpenPortal = true;
    scene->addPhysics(frontWall.get(), true);
    scene->addObject("frontWall", std::move(frontWall));

    // 6. Falling Cube (Test Gravity)
    auto fallingCube = std::make_unique<GameObject>(scene->modelResources["portal_cube"].get());
    fallingCube->position = glm::vec3(0.0f, 30.0f, -5.0f);
    // fallingCube->rotation = glm::vec3(25.0f, 25.0f, 0.0f);
    fallingCube->setScaleToSizeX(0.7f);
    fallingCube->isTeleportable = true;
    scene->addPhysics(fallingCube.get(), false, 1.0f, 0.2f, 0.5f);
    scene->addObject("fallingCube", std::move(fallingCube));

    // 7. Portal Gun
    scene->portalGun = std::make_unique<PortalGun>(scene->modelResources["portal_gun"].get());
    scene->portalGun->position = glm::vec3(0.5f, -0.5f, -1.0f);
    scene->portalGun->scale = glm::vec3(0.05f);

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

    // Initialize Physics World State (Update OBBs)
    if (scene->physicsSystem) {
        scene->physicsSystem->update(0.0f);
    }

    return true;
}

void Application::run() {
    // Reset time to avoid large dt on first frame
    lastFrame = static_cast<float>(glfwGetTime());

    while (!glfwWindowShouldClose(window)) {
        // per-frame time logic
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Clamp deltaTime to avoid physics explosions (e.g. during debugging or lag spikes)
        if (deltaTime > 0.1f) deltaTime = 0.1f;

        // input
        // poll and update input state first
        input.update();
        processInput(deltaTime);

        // --- Logic Update ---
        Camera &activeCamera = getActiveCamera();
        scene->update(deltaTime, activeCamera);

        // render
        renderer->render(*scene, activeCamera);

        // glfw: swap buffers and poll IO events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}

void Application::shutdown() {
    glfwTerminate();
}

Camera &Application::getActiveCamera() {
    if (scene->player) {
        return scene->player->camera;
    }
    return fallbackCamera;
}

void Application::processInput(float deltaTime) {
    if (input.isKeyDown(GLFW_KEY_ESCAPE))
        glfwSetWindowShouldClose(window, true);

    if (scene->player) {
        scene->player->processInput(input, deltaTime);
    }

    Camera &cam = getActiveCamera();

    if (input.isKeyDown(GLFW_KEY_G)) {
        auto cube = scene->objects["fallingCube"].get();
        if (cube && cube->rigidBody) cube->rigidBody->addForce(cam.Front * 50.0f);
    }

    if (input.isKeyPressed(GLFW_KEY_R)) {
        auto cube = scene->objects["fallingCube"].get();
        if (cube && cube->rigidBody) {
            cube->position = glm::vec3(0.0f, 30.0f, -5.0f);
            cube->rigidBody->velocity = glm::vec3(0.0f);
            cube->rigidBody->clearForces();
        }
    }

    if (input.isKeyPressed(GLFW_KEY_T)) {
        scene->player->position = glm::vec3(0.0f, 20.0f, 0.0f);
        scene->player->rigidBody->velocity = glm::vec3(0.0f);
        scene->player->rigidBody->clearForces();
    }

    if (input.isKeyPressed(GLFW_KEY_E)) {
        if (scene->player->isGrabbing) {
            // Release
            scene->player->isGrabbing = false;
        } else {
            auto result = scene->physicsSystem->raycast(cam.Position, cam.Front, 5.0f);
            if (result.hit && result.object && result.object->isTeleportable) {
                scene->player->isGrabbing = true;
                scene->player->grabbedObject = result.object;
            }
        }
    }

    if (input.isMousePressed(GLFW_MOUSE_BUTTON_LEFT)) {
        scene->portalGun->fire();
        auto result = scene->physicsSystem->raycast(cam.Position, cam.Front, 100.0f);
        scene->portalA->checkRaycast(result);
    }
    if (input.isMousePressed(GLFW_MOUSE_BUTTON_RIGHT)) {
        scene->portalGun->fire();
        auto result = scene->physicsSystem->raycast(cam.Position, cam.Front, 100.0f);
        scene->portalB->checkRaycast(result);
    }
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
    Application *app = static_cast<Application *>(glfwGetWindowUserPointer(window));
    if (!app) return;

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

    app->getActiveCamera().ProcessMouseMovement(xoffset, yoffset);
}

void Application::scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
    Application *app = static_cast<Application *>(glfwGetWindowUserPointer(window));
    if (!app) return;
    app->getActiveCamera().ProcessMouseScroll(static_cast<float>(yoffset));
}