#include "Application.h"
#include "PortalGun.h"
#include "InputManager.h"
#include "Trigger.h"
#include "Door.h"
#include <iostream>
#include <cmath>

#include <glm/gtc/matrix_transform.hpp>
#include "GameMechanics.h"

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
    glm::vec3 spawnPos(9.2169f, -3.1f, 3.0f);
    scene->player = std::make_unique<Player>(spawnPos);
    scene->player->name = "player"; 
    scene->lightPos = glm::vec3(0.0f, 15.0f, 0.0f);

    // --- Load Resources ---
    auto cubeModel = std::make_unique<Model>("resources/obj/wall/cube.obj");
    auto portal_cubeModel = std::make_unique<Model>("resources/obj/portal_cube/portal_cube.obj");
    auto portal_gunModel = std::make_unique<Model>("resources/obj/portal_gun/portal_gun.obj");
    scene->addModelResource("portal_cube", std::move(portal_cubeModel));
    scene->addModelResource("portal_gun", std::move(portal_gunModel));

    // --- Level Models ---
    scene->addModelResource("cube", std::move(cubeModel));
    scene->addModelResource("spawn_floor", std::make_unique<Model>("resources/obj/level/spawn_floor.obj"));
    scene->addModelResource("spawn_ceiling", std::make_unique<Model>("resources/obj/level/spawn_ceiling.obj"));
    scene->addModelResource("spawn_wall_01", std::make_unique<Model>("resources/obj/level/spawn_wall_01.obj"));
    scene->addModelResource("spawn_wall_02", std::make_unique<Model>("resources/obj/level/spawn_wall_02.obj"));
    scene->addModelResource("spawn_wall_03", std::make_unique<Model>("resources/obj/level/spawn_wall_03.obj"));
    scene->addModelResource("spawn_wall_04", std::make_unique<Model>("resources/obj/level/spawn_wall_04.obj"));
    scene->addModelResource("spawn_wall_05", std::make_unique<Model>("resources/obj/level/spawn_wall_05.obj"));

    scene->addModelResource("main_floor", std::make_unique<Model>("resources/obj/level/main_floor.obj"));
    scene->addModelResource("main_ceiling", std::make_unique<Model>("resources/obj/level/main_ceiling.obj"));
    scene->addModelResource("main_wall_01", std::make_unique<Model>("resources/obj/level/main_wall_01.obj"));
    scene->addModelResource("main_wall_02", std::make_unique<Model>("resources/obj/level/main_wall_02.obj"));
    scene->addModelResource("main_wall_03_l", std::make_unique<Model>("resources/obj/level/main_wall_03_l.obj"));
    scene->addModelResource("main_wall_03_r", std::make_unique<Model>("resources/obj/level/main_wall_03_r.obj"));
    scene->addModelResource("main_wall_03_t", std::make_unique<Model>("resources/obj/level/main_wall_03_t.obj"));
    scene->addModelResource("main_wall_04_l", std::make_unique<Model>("resources/obj/level/main_wall_04_l.obj"));
    scene->addModelResource("main_wall_04_r", std::make_unique<Model>("resources/obj/level/main_wall_04_r.obj"));
    scene->addModelResource("main_wall_04_t", std::make_unique<Model>("resources/obj/level/main_wall_04_t.obj"));
    scene->addModelResource("main_wall_04_b", std::make_unique<Model>("resources/obj/level/main_wall_04_b.obj"));

    scene->addModelResource("goal_floor", std::make_unique<Model>("resources/obj/level/goal_floor.obj"));
    scene->addModelResource("goal_ceiling", std::make_unique<Model>("resources/obj/level/goal_ceiling.obj"));
    scene->addModelResource("goal_wall_01", std::make_unique<Model>("resources/obj/level/goal_wall_01.obj"));
    scene->addModelResource("goal_wall_03", std::make_unique<Model>("resources/obj/level/goal_wall_03.obj"));
    scene->addModelResource("goal_wall_04", std::make_unique<Model>("resources/obj/level/goal_wall_04.obj"));
    scene->addModelResource("platform", std::make_unique<Model>("resources/obj/level/platform.obj"));
    scene->addModelResource("button_base", std::make_unique<Model>("resources/obj/level/button_base.obj"));
    scene->addModelResource("button_top",  std::make_unique<Model>("resources/obj/level/button_top.obj"));
    scene->addModelResource("barrier",     std::make_unique<Model>("resources/obj/level/barrier.obj"));

    scene->addModelResource("spawn", std::make_unique<Model>("resources/obj/level/spawn.obj"));
    scene->addModelResource("goal",  std::make_unique<Model>("resources/obj/level/goal.obj"));


    // --- Portal A ---
    scene->portalA = std::make_unique<Portal>(width, height);
    scene->portalA->position = glm::vec3(100.0f, 1.0f, 101.0f);
    scene->portalA->scale = glm::vec3(0.8f, 1.4f, 0.005f);
    scene->portalA->name = "PortalA";
    scene->portalA->type = PORTAL_A;
    scene->portalA->init(scene.get());

    // --- Portal B ---
    scene->portalB = std::make_unique<Portal>(width, height);
    scene->portalB->position = glm::vec3(100.0f, 0.0f, 100.0f);
    scene->portalB->rotation = glm::vec3(0.0f, 180.0f, 0.0f);
    scene->portalB->scale = glm::vec3(0.8f, 1.4f, 0.005f);
    scene->portalB->name = "PortalB";
    scene->portalB->type = PORTAL_B;
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
    (void)level;
    auto addStaticLevelObj = [&](const std::string& name,
                                const std::string& modelKey,
                                bool canPortal)
    {
        auto obj = std::make_unique<GameObject>(
            scene->modelResources[modelKey].get()
        );

        obj->name = name;
        obj->position = glm::vec3(0.0f);
        obj->rotation = glm::vec3(0.0f);
        obj->scale    = glm::vec3(1.0f);
        obj->canOpenPortal = canPortal;

        scene->addPhysics(obj.get(), true);
        scene->addObject(name, std::move(obj));
    };  

    addStaticLevelObj("spawn_floor",   "spawn_floor",   false);
    addStaticLevelObj("spawn_ceiling", "spawn_ceiling", false);
    addStaticLevelObj("spawn_wall_01", "spawn_wall_01", false);
    addStaticLevelObj("spawn_wall_02", "spawn_wall_02", false);
    addStaticLevelObj("spawn_wall_03", "spawn_wall_03", false);
    addStaticLevelObj("spawn_wall_04", "spawn_wall_04", false);
    addStaticLevelObj("spawn_wall_05", "spawn_wall_05", false);

    addStaticLevelObj("main_floor",   "main_floor",   true);
    addStaticLevelObj("main_ceiling", "main_ceiling", true);
    addStaticLevelObj("main_wall_01", "main_wall_01", true);
    addStaticLevelObj("main_wall_02", "main_wall_02", true);
    addStaticLevelObj("main_wall_03_l",  "main_wall_03_l",  true);
    addStaticLevelObj("main_wall_03_r", "main_wall_03_r", true);
    addStaticLevelObj("main_wall_03_t",   "main_wall_03_t",   true);
    addStaticLevelObj("main_wall_04_l", "main_wall_04_l", true);
    addStaticLevelObj("main_wall_04_r", "main_wall_04_r", true);
    addStaticLevelObj("main_wall_04_t", "main_wall_04_t", true);
    addStaticLevelObj("main_wall_04_b", "main_wall_04_b", true);

    addStaticLevelObj("goal_floor",   "goal_floor",   false);
    addStaticLevelObj("goal_ceiling", "goal_ceiling", false);
    addStaticLevelObj("goal_wall_01", "goal_wall_01", false);
    addStaticLevelObj("goal_wall_03", "goal_wall_03", false);
    addStaticLevelObj("goal_wall_04", "goal_wall_04", false);

    addStaticLevelObj("platform", "platform", true);


auto cubeObj = std::make_unique<GameObject>(scene->modelResources["portal_cube"].get());
    cubeObj->name = "companion_cube";
    cubeObj->position = glm::vec3(6.0f, -4.0f, 22.0f); 
    cubeObj->scale = glm::vec3(0.05f); 
    cubeObj->canOpenPortal = false;    
    cubeObj->isTeleportable = true; 
    scene->addPhysics(cubeObj.get(), false, 1.0f, 0.2f, 0.5f); 
    scene->addObject("companion_cube", std::move(cubeObj));


auto doorModel = std::make_unique<Model>("resources/obj/level/door.obj");
    scene->addModelResource("door", std::move(doorModel));
    auto door = std::make_unique<Door>(scene->modelResources["door"].get());
    door->position = glm::vec3(11.388f, -3.2731f, 11.356f);
    door->scale    = glm::vec3(1.0f);
    scene->addObject("door1", std::move(door));
    scene->addPhysics(scene->objects["door1"].get(), true /*static*/);

    glm::vec3 zeroPos(0.0f); 

    // A. 底座 (Button Base)
    auto baseObj = std::make_unique<GameObject>(scene->modelResources["button_base"].get());
    baseObj->position = zeroPos; 
    baseObj->scale = glm::vec3(1.0f); 
    baseObj->name = "button_base";
    scene->addPhysics(baseObj.get(), true); 
    scene->addObject("button_base", std::move(baseObj));

    // B. 盖子 (Button Top)
    auto topModel = scene->modelResources["button_top"].get();
    auto button = std::make_unique<Button>(topModel, zeroPos, glm::vec3(1.0f)); 
    button->name = "mechanism_button";
    
    scene->addPhysics(button.get(), true);
    button->initTrigger(scene.get()); 
    
    Button* btnPtr = button.get();
    scene->addObject("mechanism_button", std::move(button));

    // ==========================================
    // 4. 创建障碍墙 (Barrier) - 修正坐标版
    // ==========================================
    
    auto barrierModel = scene->modelResources["barrier"].get();
    auto barrier = std::make_unique<Barrier>(barrierModel, zeroPos, glm::vec3(1.0f), 10.0f); 
    barrier->name = "gate_barrier";
    
    barrier->setLinkedButton(btnPtr);
    scene->addPhysics(barrier.get(), true);
    scene->addObject("gate_barrier", std::move(barrier));

    // ==========================================
    // 5. 起点装饰 (Spawn)
    // ==========================================
    // 这是一个静态模型，坐标已经烘焙在 OBJ 里了
    auto spawnModel = scene->modelResources["spawn"].get();
    auto spawnObj = std::make_unique<GameObject>(spawnModel);
    spawnObj->position = glm::vec3(0.0f); 
    spawnObj->name = "spawn_point";
    // 设为 false (Sensor/Ghost) 或者设为 true 但 collisionMask 设为 0
    // 为了防止出生就卡在模型里，我们可以给它加物理，但这一步要小心。
    // 如果模型是空心的管子，addPhysics(..., true) 会生成一个大的实心盒子(AABB)把玩家卡住。
    // *最稳妥的做法*：只作为视觉物体，不加 physics，或者 mask=0
    // scene->addPhysics(spawnObj.get(), true); <--- 先注释掉，免得卡住玩家
    scene->addObject("spawn_point", std::move(spawnObj));

    // ==========================================
    // 6. 终点 (Goal)
    // ==========================================
    auto goalModel = scene->modelResources["goal"].get();
    auto goal = std::make_unique<Goal>(goalModel, zeroPos, glm::vec3(1.0f));
    goal->name = "level_goal";
    if (goalModel) {
        goal->collider = std::make_unique<AABB>(goalModel->minBound, goalModel->maxBound);
    }
    goal->initTrigger(scene.get());
    scene->addObject("level_goal", std::move(goal));
}   
/*
void Application::createScene(int level) {
    
    (void *)level;
    auto floor = std::make_unique<GameObject>(scene->modelResources["cube"].get());
    floor->position = glm::vec3(0.0f, -2.0f, 0.0f);
    floor->scale = glm::vec3(10.0f, 0.1f, 10.0f);
    floor->canOpenPortal = true;
    scene->addPhysics(floor.get(), true);
    scene->addObject("floor", std::move(floor));

    auto ceil = std::make_unique<GameObject>(scene->modelResources["cube"].get());
    ceil->position = glm::vec3(5.0f, 5.0f, 5.0f);
    ceil->scale = glm::vec3(5.0f, 0.1f, 5.0f);
    ceil->canOpenPortal = true;
    scene->addPhysics(ceil.get(), true);
    scene->addObject("ceiling", std::move(ceil));

    auto backWall = std::make_unique<GameObject>(scene->modelResources["cube"].get());
    backWall->position = glm::vec3(0.0f, 2.0f, -10.0f);
    backWall->scale = glm::vec3(10.0f, 5.0f, 0.1f);
    backWall->rotation = glm::vec3(-25.0f, 0.0f, 0.0f);
    backWall->canOpenPortal = true;
    scene->addPhysics(backWall.get(), true);
    scene->addObject("backWall", std::move(backWall));

    auto leftWall = std::make_unique<GameObject>(scene->modelResources["cube"].get());
    leftWall->position = glm::vec3(-10.0f, 3.0f, 0.0f);
    leftWall->scale = glm::vec3(0.1f, 5.0f, 10.0f);
    leftWall->canOpenPortal = true;
    scene->addPhysics(leftWall.get(), true);
    scene->addObject("leftWall", std::move(leftWall));

    auto frontWall = std::make_unique<GameObject>(scene->modelResources["cube"].get());
    frontWall->position = glm::vec3(0.0f, 3.0f, 10.0f);
    frontWall->scale = glm::vec3(10.0f, 5.0f, 0.1f);
    frontWall->canOpenPortal = true;
    scene->addPhysics(frontWall.get(), true);
    scene->addObject("frontWall", std::move(frontWall));

    auto fallingCube = std::make_unique<GameObject>(scene->modelResources["portal_cube"].get());
    fallingCube->position = glm::vec3(0.0f, 30.0f, -5.0f);
    fallingCube->setScaleToSizeX(0.7f);
    fallingCube->isTeleportable = true;
    scene->addPhysics(fallingCube.get(), false, 1.0f, 0.2f, 0.5f);
    scene->addObject("fallingCube", std::move(fallingCube));

    auto levelObj = std::make_unique<GameObject>(scene->modelResources["level"].get());
    levelObj->position = glm::vec3(0.0f, 0.0f, 0.0f);
    levelObj->scale    = glm::vec3(1.0f);
    levelObj->canOpenPortal = false;  // 先关掉，避免射线打到它的整体 OBB
    // 注意：先别 addPhysics（否则只会得到一个“巨大盒子”的碰撞）
    scene->addObject("level", std::move(levelObj));
}*/




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
                if (!isLevelComplete) {
            scene->update(deltaTime, activeCamera);
        } else {
            winTimer += deltaTime;
            int remain = std::max(0, (int)std::ceil(3.0f - winTimer));
            std::string title = "PASS! WIN! Resetting in " + std::to_string(remain) + "s...";
            glfwSetWindowTitle(window, title.c_str());

            if (winTimer > 3.0f) {
                resetLevel();
                glfwSetWindowTitle(window, "PortalGame"); 
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
       resetLevel();
    }

    if (scene->objects.count("level_goal")) {
        auto goal = dynamic_cast<Goal*>(scene->objects["level_goal"].get());
        if (goal && goal->isWon  && !isLevelComplete ) {
            std::cout << "Victory Detected! Resetting..." << std::endl;
            isLevelComplete = true;
            winTimer = 0.0f;
            //resetLevel();
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

void Application::resetLevel() {
    std::cout << "Resetting Level..." << std::endl;
    
    isLevelComplete = false;
    winTimer = 0.0f;
    // 1. 重置玩家
    glm::vec3 spawnPos(9.2169f, -3.1f, 3.0f);
    if (scene->player) {
        scene->player->position = spawnPos;
        // 强制回正视角
        scene->player->camera.Yaw = -90.0f;
        scene->player->camera.Pitch = 0.0f;
        scene->player->camera.updateCameraVectors();

        if (scene->player->rigidBody) {
            scene->player->rigidBody->velocity = glm::vec3(0.0f);
            scene->player->rigidBody->clearForces();
        }
    }

    // 2. 重置方块
    if (scene->objects.count("companion_cube")) {
        auto cube = scene->objects["companion_cube"].get();
        cube->position = glm::vec3(6.0f, -4.0f, 22.0f);
        cube->rotation = glm::vec3(0.0f);
        if (cube->rigidBody) {
            cube->rigidBody->velocity = glm::vec3(0.0f);
            cube->rigidBody->clearForces();
        }
    }

    // 3. 重置传送门
    if (scene->portalA) scene->portalA->reset();
    if (scene->portalB) scene->portalB->reset();

    // 4. 重置按钮 (Button)
    if (scene->objects.count("mechanism_button")) {
        GameObject* obj = scene->objects["mechanism_button"].get();
        Button* btn = dynamic_cast<Button*>(obj);
        if (btn) btn->reset();
    }

    // 5. 重置门 (Door)
    for (auto& [name, obj] : scene->objects) {
        if (auto door = dynamic_cast<Door*>(obj.get())) {
            door->reset();
        }
    }

    // 6. 重置终点状态 (Goal)
    if (scene->objects.count("level_goal")) {
        GameObject* obj = scene->objects["level_goal"].get();
        Goal* goal = dynamic_cast<Goal*>(obj);
        if (goal) goal->isWon = false;
    }



} 
