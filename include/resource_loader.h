#ifndef RESOURCE_LOADER_H
#define RESOURCE_LOADER_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <glad/glad.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

namespace ResourceLoader
{
    GLuint LoadShaderGL(const char * vertex_shader_path, const char * fragment_shader_path);
    GLuint LoadImageGL(const char * image_file_path);
}

#endif