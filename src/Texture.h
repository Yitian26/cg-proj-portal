#pragma once

#include <string>

#include <glad/gl.h>

class Texture {
public:
    unsigned int ID;
    std::string type;
    std::string path;

    Texture(const char *path, const std::string &directory, const std::string &typeName = "texture_diffuse");
    // Helper for loading texture from path
    static unsigned int TextureFromFile(const char *path, const std::string &directory, bool gamma = false);

    // Default textures
    static unsigned int WhiteTexture;
    static unsigned int CheckerboardTexture;
    static void InitDefaultTextures();
};
