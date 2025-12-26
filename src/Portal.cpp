#include "Portal.h"
#include "Scene.h"
#include "Player.h"

Portal::Portal(Model *model, int width, int height, glm::vec3 pos, glm::vec3 rot, glm::vec3 scale)
    : GameObject(model, pos, rot, scale), linkedPortal(nullptr) {
    frameBuffer[0] = std::make_unique<FrameBuffer>(width, height);
    frameBuffer[1] = std::make_unique<FrameBuffer>(width, height);
}

void Portal::init(Scene *scene) {
    createFrames(scene->modelResources["cube"].get(), 0.15f, 0.2f);
    registerFramesPhysics(scene, COLLISION_MASK_PORTALFRAME);
    nearTrigger = new Trigger(glm::vec3(100.0f), glm::vec3(101.0f));
    nearTrigger->onEnter = [](GameObject *obj) {
        obj->setCollisionMask(COLLISION_MASK_NEARPORTAL);
        };
    nearTrigger->onInside = [](GameObject *obj) {
        obj->setCollisionMask(COLLISION_MASK_NEARPORTAL);
        };
    nearTrigger->onExit = [](GameObject *obj) {
        obj->setCollisionMask(COLLISION_MASK_DEFAULT);
        };
    scene->addTrigger(name + "NearTrigger", std::unique_ptr<Trigger>(nearTrigger));
    teleportTrigger = new Trigger(glm::vec3(100.0f), glm::vec3(101.0f));
    teleportTrigger->onEnter = [this](GameObject *obj) {
        if (!linkedPortal || !obj || !obj->isTeleportable) return;

        // Build rotation matrices for source (this) and destination (linkedPortal)
        glm::mat4 srcRot = glm::mat4(1.0f);
        srcRot = glm::rotate(srcRot, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
        srcRot = glm::rotate(srcRot, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
        srcRot = glm::rotate(srcRot, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

        glm::mat4 dstRot = glm::mat4(1.0f);
        dstRot = glm::rotate(dstRot, glm::radians(linkedPortal->rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
        dstRot = glm::rotate(dstRot, glm::radians(linkedPortal->rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
        dstRot = glm::rotate(dstRot, glm::radians(linkedPortal->rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

        // Add a 180 degree rotation around the portal's local Y to map facing correctly.
        glm::mat4 rot180 = glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));

        // Transform position: compute local position in source's local axes, apply 180deg, then transform to dest world
        glm::vec4 relPos = glm::inverse(srcRot) * glm::vec4(obj->position - this->position, 1.0f);
        glm::vec4 newWorldPos4 = glm::vec4(linkedPortal->position, 1.0f) + dstRot * (rot180 * relPos);
        // push slightly forward along the destination portal normal to avoid immediate re-trigger
        glm::vec3 dstForward = glm::normalize(glm::vec3(dstRot * glm::vec4(0.0f, 0.0f, 1.0f, 0.0f)));
        const float teleportForwardPush = 0.15f;
        obj->position = glm::vec3(newWorldPos4) + dstForward * teleportForwardPush;

        // Transform velocity if present (treat as direction vector, w=0) with 180deg flip
        if (obj->rigidBody) {
            glm::vec4 relVel = glm::inverse(srcRot) * glm::vec4(obj->rigidBody->velocity, 0.0f);
            glm::vec4 newVel4 = dstRot * (rot180 * relVel);
            obj->rigidBody->velocity = glm::vec3(newVel4);
        }

        // If this is the Player, also rotate the camera's orientation (Front/Yaw/Pitch)
        Player *player = dynamic_cast<Player *>(obj);
        if (player) {
            // Transform camera Front vector through src->rot180->dst
            glm::vec4 relFront = glm::inverse(srcRot) * glm::vec4(player->camera.Front, 0.0f);
            glm::vec4 newFront4 = dstRot * (rot180 * relFront);
            glm::vec3 newFront = glm::normalize(glm::vec3(newFront4));
            player->camera.Front = newFront;

            // Recompute yaw/pitch from new front vector. Use atan2 to get yaw consistent with Camera implementation.
            float pitch = glm::degrees(glm::asin(glm::clamp(newFront.y, -1.0f, 1.0f)));
            float yaw = glm::degrees(std::atan2(newFront.z, newFront.x));
            player->camera.Yaw = yaw;
            player->camera.Pitch = pitch;

            // Manually update Right and Up vectors (can't call private updateCameraVectors)
            player->camera.Right = glm::normalize(glm::cross(player->camera.Front, player->camera.WorldUp));
            player->camera.Up = glm::normalize(glm::cross(player->camera.Right, player->camera.Front));

            // Also move camera position to follow player properly
            player->camera.Position = player->position + glm::vec3(0.0f, player->height * 0.4f, 0.0f);
        }
        };
    scene->addTrigger(name + "TeleportTrigger", std::unique_ptr<Trigger>(teleportTrigger));
}

void Portal::createFrames(Model *cubeModel, float thickness, float depth) {
    // Create 4 thin boxes around the portal. They are owned by the Portal.
    for (int i = 0; i < 4; ++i) {
        frames[i] = std::make_unique<GameObject>(cubeModel);
    }
    // Apply initial transform based on current portal transform
    updateFramesTransform();
    // set initial scales: top/bottom span portal width, left/right span portal height
    frames[0]->scale = glm::vec3(scale.x, thickness, depth);
    frames[1]->scale = glm::vec3(scale.x, thickness, depth);
    frames[2]->scale = glm::vec3(thickness, scale.y, depth);
    frames[3]->scale = glm::vec3(thickness, scale.y, depth);
}

void Portal::registerFramesPhysics(Scene *scene, uint32_t collisionMask) {
    if (!scene) return;
    for (int i = 0; i < 4; ++i) {
        if (frames[i]) {
            // register physics only; Portal retains ownership and is responsible for transforms/draw
            scene->addPhysics(frames[i].get(), true, collisionMask);
        }
    }
}

void Portal::updateFramesTransform() {
    // Compute axes from portal rotation
    glm::mat4 rotationMat = glm::mat4(1.0f);
    rotationMat = glm::rotate(rotationMat, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
    rotationMat = glm::rotate(rotationMat, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
    rotationMat = glm::rotate(rotationMat, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

    glm::vec3 axes[3];
    axes[0] = glm::vec3(rotationMat[0]); // right
    axes[1] = glm::vec3(rotationMat[1]); // up
    axes[2] = glm::vec3(rotationMat[2]); // forward

    float halfW = scale.x;
    float halfH = scale.y;
    // Determine current thickness/depth from frame scales if present
    float thickness = 0.05f;
    float depth = 0.1f;

    // Top
    if (frames[0]) {
        frames[0]->rotation = rotation;
        frames[0]->position = position + axes[1] * (halfH + 0.18f - thickness * 0.5f) - axes[2] * (depth * 2.0f - 0.05f);
    }
    // Bottom
    if (frames[1]) {
        frames[1]->rotation = rotation;
        frames[1]->position = position + axes[1] * (-halfH - 0.18f + thickness * 0.5f) - axes[2] * (depth * 2.0f - 0.05f);
    }
    // Left
    if (frames[2]) {
        frames[2]->rotation = rotation;
        frames[2]->position = position + axes[0] * (-halfW - 0.18f + thickness * 0.5f) - axes[2] * (depth * 2.0f - 0.05f);
    }
    // Right
    if (frames[3]) {
        frames[3]->rotation = rotation;
        frames[3]->position = position + axes[0] * (halfW + 0.18f - thickness * 0.5f) - axes[2] * (depth * 2.0f - 0.05f);
    }
}

void Portal::checkRaycast(RaycastHit result) {
    if (result.hit) {
        if (onObject) {
            onObject->setCollisionMask(COLLISION_MASK_DEFAULT);
        }
        result.object->setCollisionMask(COLLISION_MASK_PORTALON);
        onObject = result.object;
        // std::cout << "Raycast hit object at distance: " << result.distance << std::endl;
        position = result.point + result.normal * 0.05f;
        // Align portal normal (0,0,1) to hit normal
        float ny = glm::clamp(result.normal.y, -1.0f, 1.0f);
        float pitch = -glm::degrees(std::asin(ny));
        float yaw = 0.0f;
        if (glm::abs(result.normal.x) > 1e-5 || glm::abs(result.normal.z) > 1e-5) {
            yaw = glm::degrees(std::atan2(result.normal.x, result.normal.z));
        }
        rotation = glm::vec3(pitch, yaw, 0.0f);
        // move trigger
        glm::mat4 rotationMat = glm::mat4(1.0f);
        rotationMat = glm::rotate(rotationMat, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
        rotationMat = glm::rotate(rotationMat, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
        rotationMat = glm::rotate(rotationMat, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

        glm::vec3 axes[3];
        axes[0] = glm::vec3(rotationMat[0]); // right
        axes[1] = glm::vec3(rotationMat[1]); // up
        axes[2] = glm::vec3(rotationMat[2]); // forward (portal normal)

        // Portal width/height from portal->scale (assume x=width, y=height)
        float halfWidth = scale.x;
        float halfHeight = scale.y;
        nearTrigger->setFromCenterAxesExtents(position + result.normal * 0.9f, axes, glm::vec3(halfWidth, halfHeight, 1.0f));
        teleportTrigger->setFromCenterAxesExtents(position - result.normal * 0.4f, axes, glm::vec3(halfWidth, halfHeight, 0.46f));
        // Update frames to follow portal
        updateFramesTransform();
    } else {
        std::cout << "Raycast missed." << std::endl;
    }
}

void Portal::beginRender() {
    currentBuffer = (currentBuffer + 1) % 2;
    frameBuffer[currentBuffer]->Bind();
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

glm::mat4 Portal::getTransformedView(glm::mat4 view) {
    if (!linkedPortal) return view;
    // 1. Get Model Matrices
    glm::mat4 myModel = glm::mat4(1.0f);
    myModel = glm::translate(myModel, position);
    myModel = glm::rotate(myModel, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
    myModel = glm::rotate(myModel, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
    myModel = glm::rotate(myModel, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

    glm::mat4 otherModel = glm::mat4(1.0f);
    otherModel = glm::translate(otherModel, linkedPortal->position);
    otherModel = glm::rotate(otherModel, glm::radians(linkedPortal->rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
    otherModel = glm::rotate(otherModel, glm::radians(linkedPortal->rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
    otherModel = glm::rotate(otherModel, glm::radians(linkedPortal->rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

    // 2. Calculate Relative Transform (Camera -> Me)

    glm::mat4 camTransform = glm::inverse(view);

    // Transformation from this portal's local space to the other portal's local space

    // Step 1: Transform Camera to My Local Space
    glm::mat4 camInLocal = glm::inverse(myModel) * camTransform;

    // Step 2: Rotate 180 degrees around Y (to face out)

    glm::mat4 rotation180 = glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 destView = otherModel * rotation180 * glm::inverse(myModel) * camTransform;

    return glm::inverse(destView);
}

void Portal::endRender(int scrWidth, int scrHeight) {
    frameBuffer[currentBuffer]->Unbind();
    glViewport(0, 0, scrWidth, scrHeight);
}

glm::vec4 Portal::getPlaneEquation() {
    if (!linkedPortal) return glm::vec4(0.0f);

    glm::mat4 modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, linkedPortal->position);
    modelMatrix = glm::rotate(modelMatrix, glm::radians(linkedPortal->rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
    modelMatrix = glm::rotate(modelMatrix, glm::radians(linkedPortal->rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
    modelMatrix = glm::rotate(modelMatrix, glm::radians(linkedPortal->rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

    glm::vec3 normal = glm::vec3(modelMatrix * glm::vec4(0.0f, 0.0f, 1.0f, 0.0f));
    normal = glm::normalize(normal);

    // Plane equation: Ax + By + Cz + D = 0
    // D = -dot(N, P)
    float d = -glm::dot(normal, linkedPortal->position);

    return glm::vec4(normal, d);
}

void Portal::draw(Shader &shader) {
    glActiveTexture(GL_TEXTURE10);
    glBindTexture(GL_TEXTURE_2D, frameBuffer[currentBuffer]->GetTextureID());
    shader.setInt("reflectionTexture", 10);

    GameObject::draw(shader);

    glActiveTexture(GL_TEXTURE0);
}

void Portal::drawPrev(Shader &shader) {
    glActiveTexture(GL_TEXTURE10);
    glBindTexture(GL_TEXTURE_2D, frameBuffer[(currentBuffer + 1) % 2]->GetTextureID());
    shader.setInt("reflectionTexture", 10);

    GameObject::draw(shader);

    glActiveTexture(GL_TEXTURE0);
}

unsigned int Portal::getTextureID() const {
    return frameBuffer[(currentBuffer + 1) % 2]->GetTextureID();
}
