#pragma once

#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include "GameObject.h"
#include "Portal.h"
#include "Model.h"
#include "Skybox.h"
#include "PortalGun.h"

struct Scene {
    // Resource Management
    std::vector<std::unique_ptr<Model>> modelResources;

    // Scene Graph
    std::vector<std::unique_ptr<GameObject>> objects;

    // Special Objects
    std::unique_ptr<Portal> portalA;
    std::unique_ptr<Portal> portalB;
    std::unique_ptr<Skybox> skybox;
    std::unique_ptr<PortalGun> portalGun;

    // Lighting
    glm::vec3 lightPos;

    void update(float dt, const Camera &camera) {
        for (auto &obj : objects) {
            obj->update(dt, camera);
        }
        if (portalGun) portalGun->update(dt, camera);
    }
};
