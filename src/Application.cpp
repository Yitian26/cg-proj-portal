#include "Application.h"
#include <iostream>

Camera Application::camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = 1920.0f / 2.0f;
float lastY = 1080.0f / 2.0f;
bool firstMouse = true;
float deltaTime = 0.0f;
float lastFrame = 0.0f;

Application::Application(int width, int height, const std::string &title)
    : width(width), height(height), title(title), window(nullptr) {
    lightPos = glm::vec3(1.2f, 1.0f, 2.0f);
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

    // build and compile shaders
    shader = std::make_unique<Shader>("shaders/default.vert", "shaders/default.frag");
    portalShader = std::make_unique<Shader>("shaders/screen.vert", "shaders/screen.frag");

    // --- Load Resources ---
    auto cubeModel = std::make_unique<Model>("resources/obj/wall/cube.obj");
    auto portal_cubeModel = std::make_unique<Model>("resources/obj/portal_cube/portal_cube.obj");
    auto portal_gunModel = std::make_unique<Model>("resources/obj/portal_gun/portal_gun.obj");

    // Store raw pointers for GameObject creation before moving ownership
    Model *cubeModelPtr = cubeModel.get();
    Model *portalCubeModelPtr = portal_cubeModel.get();
    Model *portalGunModelPtr = portal_gunModel.get();

    modelResources.push_back(std::move(cubeModel));
    modelResources.push_back(std::move(portal_cubeModel));
    modelResources.push_back(std::move(portal_gunModel));

    // Portal A on Back Wall
    portalA = std::make_unique<Portal>(cubeModelPtr, width, height);
    portalA->position = glm::vec3(0.0f, 1.5f, -9.5f);
    portalA->scale = glm::vec3(2.0f, 3.0f, 0.1f);

    // Portal B on Left Wall
    portalB = std::make_unique<Portal>(cubeModelPtr, width, height);
    portalB->position = glm::vec3(-9.5f, 1.5f, 0.0f);
    portalB->rotation = glm::vec3(0.0f, 90.0f, 0.0f);
    portalB->scale = glm::vec3(2.0f, 3.0f, 0.1f);

    // Link Portals
    portalA->setLinkedPortal(portalB.get());
    portalB->setLinkedPortal(portalA.get());

    // 2. Floor (Ground)
    auto floor = std::make_unique<GameObject>(cubeModelPtr);
    floor->position = glm::vec3(0.0f, -2.0f, 0.0f);
    floor->scale = glm::vec3(10.0f, 0.1f, 10.0f);
    sceneObjects.push_back(std::move(floor));

    // 3. Back Wall
    auto backWall = std::make_unique<GameObject>(cubeModelPtr);
    backWall->position = glm::vec3(0.0f, 3.0f, -10.0f);
    backWall->scale = glm::vec3(10.0f, 5.0f, 0.1f);
    sceneObjects.push_back(std::move(backWall));

    // 4. Left Wall
    auto leftWall = std::make_unique<GameObject>(cubeModelPtr);
    leftWall->position = glm::vec3(-10.0f, 3.0f, 0.0f);
    leftWall->scale = glm::vec3(0.1f, 5.0f, 10.0f);
    sceneObjects.push_back(std::move(leftWall));

    // 5. Center Cube
    auto centerCube = std::make_unique<GameObject>(portalCubeModelPtr);
    centerCube->position = glm::vec3(-5.0f, 0.0f, -5.0f);
    centerCube->scale = glm::vec3(0.1f);
    sceneObjects.push_back(std::move(centerCube));

    // 6. Portal Gun
    auto portalGun = std::make_unique<GameObject>(portalGunModelPtr);
    portalGun->position = glm::vec3(0.5f, -0.5f, -1.0f);
    portalGun->scale = glm::vec3(0.05f);
    sceneObjects.push_back(std::move(portalGun));

    // Initialize Skybox
    std::vector<std::string> faces = {
        "resources/skybox/right.jpg",
        "resources/skybox/left.jpg",
        "resources/skybox/top.jpg",
        "resources/skybox/bottom.jpg",
        "resources/skybox/front.jpg",
        "resources/skybox/back.jpg"
    };
    skybox = std::make_unique<Skybox>(faces);

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
        for (auto &obj : sceneObjects) {
            obj->update(deltaTime);
        }

        // render
        render(deltaTime);

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

void Application::drawScene(Shader &shader, const glm::mat4 &view, const glm::mat4 &projection) {
    shader.use();
    shader.setMat4("projection", projection);
    shader.setMat4("view", view);
    shader.setVec3("viewPos", camera.Position);

    // Directional light
    shader.setVec3("dirLight.direction", -0.2f, -1.0f, -0.3f);
    shader.setVec3("dirLight.ambient", 0.3f, 0.3f, 0.3f);
    shader.setVec3("dirLight.diffuse", 0.4f, 0.4f, 0.4f);
    shader.setVec3("dirLight.specular", 0.5f, 0.5f, 0.5f);

    // Point light
    shader.setVec3("pointLights[0].position", lightPos);
    shader.setVec3("pointLights[0].ambient", 0.2f, 0.2f, 0.2f);
    shader.setVec3("pointLights[0].diffuse", 0.8f, 0.8f, 0.8f);
    shader.setVec3("pointLights[0].specular", 1.0f, 1.0f, 1.0f);
    shader.setFloat("pointLights[0].constant", 1.0f);
    shader.setFloat("pointLights[0].linear", 0.09f);
    shader.setFloat("pointLights[0].quadratic", 0.032f);

    shader.setFloat("material.shininess", 32.0f);

    for (auto &obj : sceneObjects) {
        obj->draw(shader);
    }

    skybox->draw(view, projection);
}

void Application::renderPortal(Portal *portal, const glm::mat4 &view, const glm::mat4 &projection) {
    glm::mat4 portalView = portal->beginRender(camera);

    // --- Oblique Frustum Clipping ---
    // 1. Get Linked Portal Plane in World Space (The plane we are looking out of)
    glm::vec4 worldPlane = portal->getPlaneEquation();

    // 2. Transform Plane to Camera Space (View Space)
    glm::vec4 viewPlane = worldPlane * glm::inverse(portalView);

    // 3. Modify Projection Matrix
    glm::mat4 obliqueProjection = projection;
    glm::vec4 q = glm::inverse(obliqueProjection) * glm::vec4(
        (viewPlane.x > 0.0f ? 1.0f : -1.0f),
        (viewPlane.y > 0.0f ? 1.0f : -1.0f),
        1.0f,
        1.0f
    );
    glm::vec4 c = viewPlane * (2.0f / glm::dot(viewPlane, q));

    obliqueProjection[0][2] = c.x - obliqueProjection[0][3];
    obliqueProjection[1][2] = c.y - obliqueProjection[1][3];
    obliqueProjection[2][2] = c.z - obliqueProjection[2][3];
    obliqueProjection[3][2] = c.w - obliqueProjection[3][3];

    // Draw scene from portal's perspective

    drawScene(*shader, portalView, obliqueProjection);

    portal->endRender(width, height);
}

void Application::render(float deltaTime) {
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)width / (float)height, 0.1f, 100.0f);
    glm::mat4 view = camera.GetViewMatrix();

    // 1. Render Portal A's View (What we see inside Portal A)
    renderPortal(portalA.get(), view, projection);

    // 2. Render Portal B's View (What we see inside Portal B)
    renderPortal(portalB.get(), view, projection);

    // 3. Render Main Pass
    {
        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        drawScene(*shader, view, projection);

        portalShader->use();
        portalShader->setMat4("projection", projection);
        portalShader->setMat4("view", view);

        // Draw Portal A
        portalA->draw(*portalShader);

        // Draw Portal B
        portalB->draw(*portalShader);
    }
}

void Application::framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    glViewport(0, 0, width, height);
    Application *app = static_cast<Application *>(glfwGetWindowUserPointer(window));
    if (app) {
        app->width = width;
        app->height = height;
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
