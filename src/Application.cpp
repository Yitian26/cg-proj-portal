#include "Application.h"
#include "PortalGun.h"
#include <iostream>
#include "InputManager.h"
#include <cmath>

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
    scene->portalA->scale = glm::vec3(1.8f, 2.7f, 0.005f);

    // Portal B
    scene->portalB = std::make_unique<Portal>(scene->modelResources["cube"].get(), width, height);
    scene->portalB->position = glm::vec3(100.0f, 0.0f, 100.0f);
    scene->portalB->rotation = glm::vec3(0.0f, 180.0f, 0.0f);
    scene->portalB->scale = glm::vec3(1.8f, 2.7f, 0.005f);

    // Link Portals
    scene->portalA->setLinkedPortal(scene->portalB.get());
    scene->portalB->setLinkedPortal(scene->portalA.get());

    // 2. Floor (Ground)
    auto floor = std::make_unique<GameObject>(scene->modelResources["cube"].get());
    floor->position = glm::vec3(0.0f, -2.0f, 0.0f);
    floor->scale = glm::vec3(10.0f, 0.1f, 10.0f);
    scene->addPhysics(floor.get(), true);
    scene->addObject("floor", std::move(floor));

    // 3. Back Wall
    auto backWall = std::make_unique<GameObject>(scene->modelResources["cube"].get());
    backWall->position = glm::vec3(0.0f, 3.0f, -10.0f);
    backWall->scale = glm::vec3(10.0f, 5.0f, 0.1f);
    backWall->rotation = glm::vec3(45.0f, 0.0f, 0.0f);
    scene->addPhysics(backWall.get(), true);
    scene->addObject("backWall", std::move(backWall));

    // 4. Left Wall
    auto leftWall = std::make_unique<GameObject>(scene->modelResources["cube"].get());
    leftWall->position = glm::vec3(-10.0f, 3.0f, 0.0f);
    leftWall->scale = glm::vec3(0.1f, 5.0f, 10.0f);
    scene->addPhysics(leftWall.get(), true);
    scene->addObject("leftWall", std::move(leftWall));

    // 3. Front Wall
    auto frontWall = std::make_unique<GameObject>(scene->modelResources["cube"].get());
    frontWall->position = glm::vec3(0.0f, 3.0f, 10.0f);
    frontWall->scale = glm::vec3(10.0f, 5.0f, 0.1f);
    scene->addPhysics(frontWall.get(), true);
    scene->addObject("frontWall", std::move(frontWall));

    // 6. Falling Cube (Test Gravity)
    auto fallingCube = std::make_unique<GameObject>(scene->modelResources["portal_cube"].get());
    fallingCube->position = glm::vec3(2.0f, 30.0f, 0.0f);
    // fallingCube->rotation = glm::vec3(25.0f, 25.0f, 0.0f);
    fallingCube->scale = glm::vec3(0.1f);
    scene->addPhysics(fallingCube.get(), false);
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

    return true;
}

void Application::run() {
    while (!glfwWindowShouldClose(window)) {
        // per-frame time logic
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // poll and update input state first
        input.update();
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
    if (input.isKeyDown(GLFW_KEY_ESCAPE))
        glfwSetWindowShouldClose(window, true);
    if (input.isKeyDown(GLFW_KEY_W))
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (input.isKeyDown(GLFW_KEY_S))
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (input.isKeyDown(GLFW_KEY_A))
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (input.isKeyDown(GLFW_KEY_D))
        camera.ProcessKeyboard(RIGHT, deltaTime);

    // Use edge detection for single-trigger actions
    if (input.isKeyDown(GLFW_KEY_G)) {
        auto cube = scene->objects["fallingCube"].get();
        if (cube && cube->rigidBody) cube->rigidBody->addForce(camera.Front * -50.0f);
    }
    if (input.isKeyPressed(GLFW_KEY_R)) {
        auto cube = scene->objects["fallingCube"].get();
        if (cube && cube->rigidBody) {
            cube->position = glm::vec3(2.0f, 30.0f, 0.0f);
            cube->rigidBody->velocity = glm::vec3(0.0f);
            cube->rigidBody->clearForces();
        }
    }

    auto spawnPortal = [&](Portal *portal) {
        scene->portalGun->fire();
        auto result = scene->physicsSystem->raycast(camera.Position, camera.Front, 100.0f);
        if (result.hit) {
            // std::cout << "Raycast hit object at distance: " << result.distance << std::endl;
            portal->position = result.point + result.normal * 0.01f;

            // Align portal normal (0,0,1) to hit normal
            float ny = glm::clamp(result.normal.y, -1.0f, 1.0f);
            float pitch = -glm::degrees(std::asin(ny));
            float yaw = 0.0f;
            if (glm::abs(result.normal.x) > 1e-5 || glm::abs(result.normal.z) > 1e-5) {
                yaw = glm::degrees(std::atan2(result.normal.x, result.normal.z));
            }
            portal->rotation = glm::vec3(pitch, yaw, 0.0f);
        } else {
            std::cout << "Raycast missed." << std::endl;
        }
    };

    if (input.isMousePressed(GLFW_MOUSE_BUTTON_LEFT)) {
        spawnPortal(scene->portalA.get());
    }
    if (input.isMousePressed(GLFW_MOUSE_BUTTON_RIGHT)) {
        spawnPortal(scene->portalB.get());
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