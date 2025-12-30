#pragma once

#include "Mesh.h"
#include "Shader.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>

#include <glad/gl.h> 
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stb_image.h>

class Model {
public:
    // model data 
    std::vector<Texture> textures_loaded;	// stores all the textures loaded so far, optimization to make sure textures aren't loaded more than once.
    std::vector<Mesh>    meshes;
    std::string directory;

    // constructor, expects a filepath to a 3D model.
    Model(std::string const &path);

    // draws the model, and thus all its meshes
    void Draw(Shader &shader);

    // Bounding box
    glm::vec3 minBound;
    glm::vec3 maxBound;
    glm::vec3 getCenter() const;
    float getNormalizationScale() const;

private:
    // loads a model with supported OBJ extensions from file and stores the resulting meshes in the meshes vector.
    void loadModel(std::string const &path);

    // Custom OBJ parser helpers
    struct Material {
        std::string name;
        std::string diffuseMap;
        glm::vec3 ambientColor = glm::vec3(1.0f);
        glm::vec3 diffuseColor = glm::vec3(1.0f);
        glm::vec3 specularColor = glm::vec3(0.5f);
        float shininess = 32.0f;
    };

    std::map<std::string, Material> materials;
    void loadMTL(std::string const &path);
};
