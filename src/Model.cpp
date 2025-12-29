#include "Model.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <map>


Model::Model(std::string const &path, bool gamma) : gammaCorrection(gamma) {
    minBound = glm::vec3(std::numeric_limits<float>::max());
    maxBound = glm::vec3(std::numeric_limits<float>::lowest());
    loadModel(path);
}

glm::vec3 Model::getCenter() const {
    return (minBound + maxBound) * 0.5f;
}

float Model::getNormalizationScale() const {
    float maxDim = std::max(std::max(maxBound.x - minBound.x, maxBound.y - minBound.y), maxBound.z - minBound.z);
    if (maxDim == 0.0f) return 1.0f;
    return 2.0f / maxDim;
}

void Model::Draw(Shader &shader) {
    for (unsigned int i = 0; i < meshes.size(); i++)
        meshes[i].Draw(shader);
}

void Model::loadMTL(std::string const &path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cout << "Failed to open MTL file: " << path << std::endl;
        return;
    }

    std::string line;
    std::string currentMtlName;

    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string prefix;
        ss >> prefix;

        if (prefix == "newmtl") {
            ss >> currentMtlName;
            materials[currentMtlName].name = currentMtlName;
        } else if (prefix == "Ka") {
            glm::vec3 color;
            ss >> color.x >> color.y >> color.z;
            materials[currentMtlName].ambientColor = color;
        } else if (prefix == "Kd") {
            glm::vec3 color;
            ss >> color.x >> color.y >> color.z;
            materials[currentMtlName].diffuseColor = color;
        } else if (prefix == "Ks") {
            glm::vec3 color;
            ss >> color.x >> color.y >> color.z;
            materials[currentMtlName].specularColor = color;
        } else if (prefix == "Ns") {
            float shininess;
            ss >> shininess;
            materials[currentMtlName].shininess = shininess;
        } else if (prefix == "map_Kd") {
            std::string textureFile;
            ss >> textureFile;
            materials[currentMtlName].diffuseMap = textureFile;
        }
    }
}

void Model::loadModel(std::string const &path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cout << "Failed to open OBJ file: " << path << std::endl;
        return;
    }

    directory = path.substr(0, path.find_last_of('/'));

    std::vector<glm::vec3> temp_positions;
    std::vector<glm::vec2> temp_texCoords;
    std::vector<glm::vec3> temp_normals;

    // Per-mesh data
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;

    // Map unique vertex string "v/vt/vn" to index
    std::map<std::string, unsigned int> uniqueVertices;

    std::string line;
    std::string currentMaterial = "";

    // Helper to flush current mesh
    auto flushMesh = [&]() {
        if (!vertices.empty()) {
            glm::vec3 ambient = glm::vec3(1.0f);
            glm::vec3 diffuse = glm::vec3(1.0f);
            glm::vec3 specular = glm::vec3(0.5f);
            float shininess = 32.0f;

            if (materials.find(currentMaterial) != materials.end()) {
                ambient = materials[currentMaterial].ambientColor;
                diffuse = materials[currentMaterial].diffuseColor;
                specular = materials[currentMaterial].specularColor;
                shininess = materials[currentMaterial].shininess;
            }
            meshes.push_back(Mesh(vertices, indices, textures, ambient, diffuse, specular, shininess));
            vertices.clear();
            indices.clear();
            textures.clear();
            uniqueVertices.clear();
        }
        };

    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string prefix;
        ss >> prefix;

        if (prefix == "mtllib") {
            std::string mtlFile;
            std::getline(ss, mtlFile);
            // Trim leading whitespace
            size_t first = mtlFile.find_first_not_of(' ');
            if (std::string::npos != first) {
                mtlFile = mtlFile.substr(first);
            }
            loadMTL(directory + "/" + mtlFile);
        } else if (prefix == "usemtl") {
            flushMesh(); // Start new mesh on material change
            ss >> currentMaterial;

            // Load textures for this material
            if (materials.find(currentMaterial) != materials.end()) {
                // Diffuse map
                std::string diffPath = materials[currentMaterial].diffuseMap;
                if (!diffPath.empty()) {
                    bool skip = false;
                    for (unsigned int j = 0; j < textures_loaded.size(); j++) {
                        if (std::strcmp(textures_loaded[j].path.data(), diffPath.c_str()) == 0) {
                            textures.push_back(textures_loaded[j]);
                            skip = true;
                            break;
                        }
                    }
                    if (!skip) {
                        Texture texture(diffPath.c_str(), directory);
                        texture.type = "texture_diffuse";
                        textures.push_back(texture);
                        textures_loaded.push_back(texture);
                    }
                }
            }
        } else if (prefix == "v") {
            glm::vec3 pos;
            ss >> pos.x >> pos.y >> pos.z;
            temp_positions.push_back(pos);

            // Update bounds
            if (pos.x < minBound.x) minBound.x = pos.x;
            if (pos.y < minBound.y) minBound.y = pos.y;
            if (pos.z < minBound.z) minBound.z = pos.z;
            if (pos.x > maxBound.x) maxBound.x = pos.x;
            if (pos.y > maxBound.y) maxBound.y = pos.y;
            if (pos.z > maxBound.z) maxBound.z = pos.z;
        } else if (prefix == "vt") {
            glm::vec2 tex;
            ss >> tex.x >> tex.y;
            temp_texCoords.push_back(tex);
        } else if (prefix == "vn") {
            glm::vec3 norm;
            ss >> norm.x >> norm.y >> norm.z;
            temp_normals.push_back(norm);
        } else if (prefix == "f") {
            std::string vertexStr;
            std::vector<std::string> faceVertices;
            while (ss >> vertexStr) {
                faceVertices.push_back(vertexStr);
            }

            // Triangulate (fan)
            for (size_t i = 1; i < faceVertices.size() - 1; ++i) {
                std::string v[3] = { faceVertices[0], faceVertices[i], faceVertices[i + 1] };

                for (int j = 0; j < 3; ++j) {
                    if (uniqueVertices.count(v[j]) == 0) {
                        uniqueVertices[v[j]] = static_cast<unsigned int>(vertices.size());

                        Vertex vertex;
                        std::stringstream vss(v[j]);
                        std::string segment;
                        std::vector<std::string> indicesStr;

                        while (std::getline(vss, segment, '/')) {
                            indicesStr.push_back(segment);
                        }

                        // Position
                        int posIdx = std::stoi(indicesStr[0]) - 1;
                        vertex.Position = temp_positions[posIdx];

                        // TexCoord
                        if (indicesStr.size() > 1 && !indicesStr[1].empty()) {
                            int texIdx = std::stoi(indicesStr[1]) - 1;
                            vertex.TexCoords = temp_texCoords[texIdx];
                        } else {
                            vertex.TexCoords = glm::vec2(0.0f, 0.0f);
                        }

                        // Normal
                        if (indicesStr.size() > 2 && !indicesStr[2].empty()) {
                            int normIdx = std::stoi(indicesStr[2]) - 1;
                            vertex.Normal = temp_normals[normIdx];
                        } else {
                            vertex.Normal = glm::vec3(0.0f, 0.0f, 0.0f);
                        }

                        vertices.push_back(vertex);
                    }
                    indices.push_back(uniqueVertices[v[j]]);
                }
            }
        }
    }
    flushMesh();
}
