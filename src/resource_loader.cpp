#include "resource_loader.h"

namespace ResourceLoader
{

    GLuint LoadShaderGL(const char *vertex_shader_path, const char *fragment_shader_path)
    {
        // Read vertex shader
        std::string vertex_code;
        std::ifstream v_shader_file;
        v_shader_file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        try
        {
            v_shader_file.open(vertex_shader_path);
            std::stringstream v_shader_stream;
            v_shader_stream << v_shader_file.rdbuf();
            v_shader_file.close();
            vertex_code = v_shader_stream.str();
        }
        catch (std::ifstream::failure &e)
        {
            std::cout << "ERROR::SHADER::VERTEX::FILE_NOT_SUCCESSFULLY_READ" << std::endl;
        }

        // Read fragment shader
        std::string fragment_code;
        std::ifstream f_shader_file;
        f_shader_file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        try
        {
            f_shader_file.open(fragment_shader_path);
            std::stringstream f_shader_stream;
            f_shader_stream << f_shader_file.rdbuf();
            f_shader_file.close();
            fragment_code = f_shader_stream.str();
        }
        catch (std::ifstream::failure &e)
        {
            std::cout << "ERROR::SHADER::FRAGMENT::FILE_NOT_SUCCESSFULLY_READ" << std::endl;
        }

        const char *v_shader_code = vertex_code.c_str();
        const char *f_shader_code = fragment_code.c_str();

        // Compile shaders
        unsigned int vertex, fragment;
        int success;
        char info_log[512];

        // Vertex Shader
        vertex = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex, 1, &v_shader_code, NULL);
        glCompileShader(vertex);
        glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(vertex, 512, NULL, info_log);
            std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n"
                      << info_log << std::endl;
        }

        // Fragment Shader
        fragment = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment, 1, &f_shader_code, NULL);
        glCompileShader(fragment);
        glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(fragment, 512, NULL, info_log);
            std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n"
                      << info_log << std::endl;
        }

        // Shader Program
        GLuint shader_program = glCreateProgram();
        glAttachShader(shader_program, vertex);
        glAttachShader(shader_program, fragment);
        glLinkProgram(shader_program);
        glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(shader_program, 512, NULL, info_log);
            std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n"
                      << info_log << std::endl;
        }

        // Delete shaders as they're linked into our program now and no longer necessary
        glDeleteShader(vertex);
        glDeleteShader(fragment);

        return shader_program;
    }

    GLuint LoadImageGL(const char *image_file_path)
    {
        SDL_Surface *surface = IMG_Load(image_file_path);
        if (!surface)
        {
            std::cout << "Failed to load image: " << image_file_path << std::endl;
            std::cout << "SDL_image Error: " << IMG_GetError() << std::endl;
            return 0;
        }

        // Create OpenGL texture
        GLuint texture_id;
        glGenTextures(1, &texture_id);
        glBindTexture(GL_TEXTURE_2D, texture_id);

        // Set texture parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // Load image data into OpenGL texture
        int mode = GL_RGB;
        if (surface->format->BytesPerPixel == 4)
        {
            mode = GL_RGBA;
        }

        glTexImage2D(GL_TEXTURE_2D, 0, mode, surface->w, surface->h, 0, mode, GL_UNSIGNED_BYTE, surface->pixels);
        glGenerateMipmap(GL_TEXTURE_2D);

        // Free the SDL_Surface
        SDL_FreeSurface(surface);

        return texture_id;
    }

} // namespace ResourceLoader
