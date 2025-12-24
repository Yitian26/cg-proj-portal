#pragma once

#include <glad/gl.h>
#include <vector>
#include <string>
#include <iostream>
#include "Shader.h"

class Skybox {
public:
    Skybox(const std::vector<std::string> &faces);
    ~Skybox();

    void draw(const glm::mat4 &view, const glm::mat4 &projection);

private:
    unsigned int textureID;
    unsigned int VAO, VBO;
    Shader *shader;

    void loadCubemap(const std::vector<std::string> &faces);
    void setupMesh();
};
