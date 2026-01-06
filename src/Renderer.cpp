#include "Renderer.h"

#include <glad/gl.h>
#include <glm/gtc/matrix_transform.hpp>

Renderer::Renderer(int width, int height) : width(width), height(height) {}

void Renderer::initialize() {
    glEnable(GL_DEPTH_TEST);

    // build and compile shaders
    auto shader = std::make_unique<Shader>("shaders/default.vert", "shaders/default.frag");
    shader->setBool("useAlphaTest", false);
    auto portalShader = std::make_unique<Shader>("shaders/screen.vert", "shaders/screen.frag");
    shaderCache["default"] = std::move(shader);
    shaderCache["portal"] = std::move(portalShader);
    // HUD manager
    hud = std::make_unique<HUD>();
    hud->initialize();
}

void Renderer::resize(int width, int height) {
    this->width = width;
    this->height = height;
    glViewport(0, 0, width, height);
}

void Renderer::drawScene(Scene &scene, Shader &shader, const glm::mat4 &view, const glm::mat4 &projection, const glm::vec3 &viewPos) {
    shader.use();
    shader.setMat4("projection", projection);
    shader.setMat4("view", view);
    shader.setVec3("viewPos", viewPos);

    // Directional light
    shader.setVec3("dirLight.direction", -0.2f, -1.0f, -0.3f);
    shader.setVec3("dirLight.ambient", 0.3f, 0.3f, 0.3f);
    shader.setVec3("dirLight.diffuse", 0.4f, 0.4f, 0.4f);
    shader.setVec3("dirLight.specular", 0.5f, 0.5f, 0.5f);

    // Point light
    shader.setVec3("pointLights[0].position", scene.lightPos);
    shader.setVec3("pointLights[0].ambient", 0.2f, 0.2f, 0.2f);
    shader.setVec3("pointLights[0].diffuse", 0.8f, 0.8f, 0.8f);
    shader.setVec3("pointLights[0].specular", 1.0f, 1.0f, 1.0f);
    shader.setFloat("pointLights[0].constant", 1.0f);
    shader.setFloat("pointLights[0].linear", 0.09f);
    shader.setFloat("pointLights[0].quadratic", 0.032f);

    shader.setFloat("material.shininess", 32.0f);

    for (auto &pair : scene.objects) {
        pair.second->draw(shader);
    }

    if (scene.skybox) {
        scene.skybox->draw(view, projection);
    }
}

void Renderer::renderPortal(Scene &scene, Portal *portal, glm::mat4 view, const glm::mat4 &projection, int recursionDepth) {
    if (recursionDepth <= 0) return;//last frame TODO:render a foo texture
    glm::mat4 transformedCam = portal->getTransformedView(view);// get transformed camera view
    renderPortal(scene,portal, transformedCam, projection, recursionDepth - 1);
    portal->beginRender();//render deeper level scene(for current portal)
    glm::vec4 worldPlane = portal->getPlaneEquation();
    glm::vec4 viewPlane = worldPlane * glm::inverse(transformedCam);
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

    glm::vec3 virtualCamPos = glm::vec3(glm::inverse(transformedCam)[3]);
    drawScene(scene, *shaderCache["default"], transformedCam, obliqueProjection, virtualCamPos);//render current level scene
    if (recursionDepth > 1) {
        auto portalShader = shaderCache["portal"].get();
        portalShader->use();
        portalShader->setMat4("projection", obliqueProjection);
        portalShader->setMat4("view", transformedCam);
        auto shader = shaderCache["default"].get();
        shader->use();
        shader->setMat4("projection", obliqueProjection);
        shader->setMat4("view", transformedCam);
        portal->drawPrev(*portalShader, *shader);// render the previous frame texture on the portal surface
        portalShader->unuse();
    }
    portal->endRender(width, height);
}

void Renderer::render(Scene &scene, Camera &camera) {
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)width / (float)height, 0.1f, 100.0f);
    glm::mat4 view = camera.GetViewMatrix();

    // 1. Render Portal Views
    if (scene.portalA) renderPortal(scene, scene.portalA.get(), view, projection, MAX_PORTAL_RECURSION);
    if (scene.portalB) renderPortal(scene, scene.portalB.get(), view, projection, MAX_PORTAL_RECURSION);

    // 2. Render Main Pass
    glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    drawScene(scene, *shaderCache["default"], view, projection, camera.Position);

    // 3. Draw Portals
    auto portalShader = shaderCache["portal"].get();
    portalShader->use();
    portalShader->setMat4("projection", projection);
    portalShader->setMat4("view", view);
    auto shader = shaderCache["default"].get();
    shader->use();
    shader->setMat4("projection", projection);
    shader->setMat4("view", view);
    if (scene.portalA) scene.portalA->draw(*portalShader, *shader);
    if (scene.portalB) scene.portalB->draw(*portalShader, *shader);

    
    // for (auto &pair : scene.triggers) {
    //     pair.second->drawOBBDebug(*shader);
    // }
    
    // 4. Draw Portal Gun
    if (scene.portalGun) scene.portalGun->draw(*shader);

    // Draw HUD
    if (hud) hud->render();
}
