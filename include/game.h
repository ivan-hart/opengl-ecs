#ifndef GAME_H
#define GAME_H

#include <iostream>

#include <SDL2/SDL.h>
#include <glad/glad.h>
#include <glm/ext.hpp>

#include <resource_loader.h>
#include <nomad_entity.hpp>

#define WINDOW_TITLE ""
#define WINDOW_POS SDL_WINDOWPOS_CENTERED
#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720
#define WINDOW_ARGS SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL

#define MAX_FRAMERATE 120

#define MAX_KEYS_LENGTH 322

class Game
{
    public:
        static Game & Instance();
        void Run();
        void Close();

        Game(const Game&) = delete;
        Game& operator=(const Game&) = delete;

        void SetClearColor(glm::vec4 color);
        void SetWindowTitle(const char * window_title);

        bool Running;
        bool Keys[MAX_KEYS_LENGTH];
        float DeltaTime;
        SDL_Event Event;
    private:
        Game() = default;
        // init functions
        bool initSDL();
        void initWindowHints();
        bool createWindow();
        bool createGLContext();
        bool initGLAD();
        bool init();

        void createSquare(Entity entity);
        float calculateDeltaTime(unsigned int NOW, unsigned int & LAST);

        void pollEvents();
        void pollKeys();
        void update();

        void render();

        SDL_Window * window;
        SDL_GLContext ctx;
        ECS ecs;

        glm::mat4 view, projection;
        glm::vec4 clearColor;
};

#endif