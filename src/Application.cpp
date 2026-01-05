#include "Button.h"
#include "Application.h"
#include "Flip.h"
#include "PortalGun.h"
#include "InputManager.h"
#include "Trigger.h"

#include <iostream>
#include <cmath>

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

    // --- Initialize input manager ---
    input.initialize(window);

    // --- Initialize Player ---
    scene->player = std::make_unique<Player>(glm::vec3(0.0f, 0.0f, 0.0f));
    scene->lightPos = glm::vec3(0.0f, 15.0f, 0.0f);

    // --- Load Resources ---
    scene->addModelResource("portal_gun", std::make_unique<Model>("resources/obj/portal_gun/portal_gun.obj"));
    scene->addModelResource("cube", std::make_unique<Model>("resources/obj/wall/cube.obj"));
    scene->addModelResource("portal_cube", std::make_unique<Model>("resources/obj/portal_cube/portal_cube.obj"));

    // --- Portal A ---
    scene->portalA = std::make_unique<Portal>(width, height);
    scene->portalA->position = glm::vec3(100.0f, 1.0f, 101.0f);
    scene->portalA->scale = glm::vec3(0.8f, 1.4f, 0.005f);
    scene->portalA->name = "PortalA";
    scene->portalA->type = PORTAL_A;
    scene->portalA->isActive = false;
    scene->portalA->init(scene.get());

    // --- Portal B ---
    scene->portalB = std::make_unique<Portal>(width, height);
    scene->portalB->position = glm::vec3(100.0f, 0.0f, 100.0f);
    scene->portalB->rotation = glm::vec3(0.0f, 180.0f, 0.0f);
    scene->portalB->scale = glm::vec3(0.8f, 1.4f, 0.005f);
    scene->portalB->name = "PortalB";
    scene->portalB->type = PORTAL_B;
    scene->portalB->isActive = false;
    scene->portalB->init(scene.get());

    // --- Link Portals ---
    scene->portalA->setLinkedPortal(scene->portalB.get());
    scene->portalB->setLinkedPortal(scene->portalA.get());

    // --- Portal Gun ---
    scene->portalGun = std::make_unique<PortalGun>(scene->modelResources["portal_gun"].get());
    scene->portalGun->position = glm::vec3(0.5f, -0.5f, -1.0f);
    scene->portalGun->scale = glm::vec3(0.05f);

    createScene();

    // --- Initialize Skybox ---
    std::vector<std::string> faces = {
        "resources/skybox/right.jpg",
        "resources/skybox/left.jpg",
        "resources/skybox/top.jpg",
        "resources/skybox/bottom.jpg",
        "resources/skybox/front.jpg",
        "resources/skybox/back.jpg"
    };
    scene->skybox = std::make_unique<Skybox>(faces);

    // --- Initialize Physics World State ---
    if (scene->physicsSystem) {
        scene->physicsSystem->update(0.0f);
    }

    return true;
}

void Application::createScene(int level) {
    (void *)level;
    scene->addModelResource("banner", std::make_unique<Model>("resources/obj/level/banner.obj"));
    scene->addModelResource("button_flip", std::make_unique<Model>("resources/obj/level/button_flip.obj"));
    scene->addModelResource("button_goal", std::make_unique<Model>("resources/obj/level/button_goal.obj"));
    scene->addModelResource("movable", std::make_unique<Model>("resources/obj/level/movable.obj"));
    scene->addModelResource("wall1", std::make_unique<Model>("resources/obj/level/wall1.obj"));
    scene->addModelResource("wall2", std::make_unique<Model>("resources/obj/level/wall2.obj"));
    scene->addModelResource("wall3", std::make_unique<Model>("resources/obj/level/wall3.obj"));
    scene->addModelResource("wall4_p", std::make_unique<Model>("resources/obj/level/wall4_p.obj"));
    scene->addModelResource("wall5", std::make_unique<Model>("resources/obj/level/wall5.obj"));
    scene->addModelResource("wall6_p", std::make_unique<Model>("resources/obj/level/wall6_p.obj"));
    scene->addModelResource("wall7", std::make_unique<Model>("resources/obj/level/wall7.obj"));
    scene->addModelResource("wall8", std::make_unique<Model>("resources/obj/level/wall8.obj"));
    scene->addModelResource("wall9", std::make_unique<Model>("resources/obj/level/wall9.obj"));
    scene->addModelResource("wall10", std::make_unique<Model>("resources/obj/level/wall10.obj"));
    scene->addModelResource("wall11_p", std::make_unique<Model>("resources/obj/level/wall11_p.obj"));
    scene->addModelResource("wall12_p", std::make_unique<Model>("resources/obj/level/wall12_p.obj"));

    auto addStaticObj = [&](const std::string &name, const std::string &modelName, bool canPortal) {
        auto obj = std::make_unique<GameObject>(scene->modelResources[modelName].get());
        obj->isTeleportable = false;
        obj->canOpenPortal = canPortal;
        // Register physics on the object before moving it into the scene map
        scene->addPhysics(obj.get(), true);
        scene->addObject(name, std::move(obj));
        };

    addStaticObj("banner", "banner", false);
    addStaticObj("wall1", "wall1", false);
    addStaticObj("wall2", "wall2", false);
    addStaticObj("wall3", "wall3", false);
    addStaticObj("wall4", "wall4_p", true);
    addStaticObj("wall5", "wall5", false);
    addStaticObj("wall6", "wall6_p", true);
    addStaticObj("wall7", "wall7", false);
    addStaticObj("wall8", "wall8", false);
    addStaticObj("wall9", "wall9", false);
    addStaticObj("wall10", "wall10", false);
    addStaticObj("wall11", "wall11_p", true);
    addStaticObj("wall12", "wall12_p", true);

    auto buttonFlip = std::make_unique<Button>(scene->modelResources["button_flip"].get(), glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(1.0f));
    buttonFlip->isTeleportable = false;
    buttonFlip->canOpenPortal = false;
    scene->addTrigger("button_flip_trigger", buttonFlip->createTrigger());
    scene->addPhysics(buttonFlip.get(), true);
    scene->addObject("button_flip", std::move(buttonFlip));

    auto flipWall = std::make_unique<Flip>(scene->modelResources["movable"].get(), glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(1.0f));
    flipWall->isTeleportable = false;
    flipWall->canOpenPortal = true;
    flipWall->setPivot(glm::vec3(0.0f, 0.0f, 0.0f));
    flipWall->setPosition(glm::vec3(-1.5f, 5.5f, -5.0f));
    scene->addPhysics(flipWall.get(), true);
    scene->addObject("flip_wall", std::move(flipWall));

    auto cube = std::make_unique<GameObject>(scene->modelResources["portal_cube"].get());
    cube->isTeleportable = true;
    cube->canOpenPortal = false;
    cube->scale = glm::vec3(0.05f);
    cube->position = glm::vec3(7.0f, -4.0f, -2.0f);
    scene->addPhysics(cube.get(), false);
    scene->addObject("portal_cube", std::move(cube));

    auto button_goal = std::make_unique<Button>(scene->modelResources["button_goal"].get(), glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(1.0f));
    button_goal->isTeleportable = false;
    button_goal->canOpenPortal = false;
    scene->addTrigger("button_goal_trigger", button_goal->createTrigger());
    scene->addPhysics(button_goal.get(), true);
    scene->addObject("button_goal", std::move(button_goal));
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
        auto button_goal = scene->objects.find("button_goal");
        if (button_goal != scene->objects.end()) {
            Button *btn = dynamic_cast<Button *>(button_goal->second.get());
            if (btn && btn->getIsPressed()) {
                //change window title to "You Win!"
                glfwSetWindowTitle(window, "You Win!");
            }
        }

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
        scene->player->processInput(input, scene.get(), deltaTime);
    }

    if (input.isKeyPressed(GLFW_KEY_T)) {
        if (scene->player) {
            scene->player->position = glm::vec3(0.0f, 0.0f, 0.0f);
            scene->player->rigidBody->velocity = glm::vec3(0.0f);
        }
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