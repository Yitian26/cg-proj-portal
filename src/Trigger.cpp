#include "Trigger.h"
#include "GameObject.h"
#include <cmath>
#include <glad/gl.h>
#include <glm/gtc/type_ptr.hpp>

Trigger::Trigger(const OBB &obb) : bounds(obb) {}

void Trigger::check(GameObject *obj) {
    if (!isActive || !obj) return;

    bool inside = isPointInside(obj->position);
    bool wasInside = objectsInside.count(obj) > 0;

    if (inside && !wasInside) {
        objectsInside.insert(obj);
        if (onEnter) onEnter(obj);
    } else if (!inside && wasInside) {
        objectsInside.erase(obj);
        if (onExit) onExit(obj);
    }

    if (inside && onInside) {
        onInside(obj);
    }
}

void Trigger::drawOBBDebug(Shader &shader) {
    if (!isActive) return;

    // Prepare 8 corner points of the OBB in world space
    glm::vec3 c = bounds.center;
    glm::vec3 ax = bounds.axes[0] * bounds.halfExtents[0];
    glm::vec3 ay = bounds.axes[1] * bounds.halfExtents[1];
    glm::vec3 az = bounds.axes[2] * bounds.halfExtents[2];

    glm::vec3 corners[8];
    // order: 0(-x,-y,-z),1(+x,-y,-z),2(-x,-y,+z),3(+x,-y,+z),
    //        4(-x,+y,-z),5(+x,+y,-z),6(-x,+y,+z),7(+x,+y,+z)
    corners[0] = c - ax - ay - az;
    corners[1] = c + ax - ay - az;
    corners[2] = c - ax - ay + az;
    corners[3] = c + ax - ay + az;
    corners[4] = c - ax + ay - az;
    corners[5] = c + ax + ay - az;
    corners[6] = c - ax + ay + az;
    corners[7] = c + ax + ay + az;

    // 12 edges, each edge has 2 vertices -> 24 vertices
    float verts[24 * 3];
    int vi = 0;
    auto pushEdge = [&](int a, int b) {
        verts[vi++] = corners[a].x; verts[vi++] = corners[a].y; verts[vi++] = corners[a].z;
        verts[vi++] = corners[b].x; verts[vi++] = corners[b].y; verts[vi++] = corners[b].z;
        };

    // bottom face (y -)
    pushEdge(0, 1);
    pushEdge(1, 3);
    pushEdge(3, 2);
    pushEdge(2, 0);
    // top face (y +)
    pushEdge(4, 5);
    pushEdge(5, 7);
    pushEdge(7, 6);
    pushEdge(6, 4);
    // vertical edges
    pushEdge(0, 4);
    pushEdge(1, 5);
    pushEdge(2, 6);
    pushEdge(3, 7);

    // Use a simple dynamic VBO/VAO (static to reuse across calls)
    static unsigned int dbgVAO = 0, dbgVBO = 0;
    if (dbgVAO == 0) {
        glGenVertexArrays(1, &dbgVAO);
        glGenBuffers(1, &dbgVBO);
    }

    shader.use();
    // set model to identity (we already provide world-space positions)
    shader.setMat4("model", glm::mat4(1.0f));

    glBindVertexArray(dbgVAO);
    glBindBuffer(GL_ARRAY_BUFFER, dbgVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);

    // Draw lines
    glDrawArrays(GL_LINES, 0, 24);

    glBindVertexArray(0);
}

bool Trigger::isPointInside(const glm::vec3 &point) const {
    // Transform point to OBB local space
    glm::vec3 d = point - bounds.center;

    // Project d onto each axis and check against halfExtents
    for (int i = 0; i < 3; ++i) {
        float dist = glm::dot(d, bounds.axes[i]);
        if (std::abs(dist) > bounds.halfExtents[i]) {
            return false;
        }
    }
    return true;
}
