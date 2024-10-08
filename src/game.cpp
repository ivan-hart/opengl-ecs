#include <game.h>

struct Renderable
{
    GLuint VAO;
    GLuint VBO;
    GLuint EBO;
    GLuint shaderProgram;
    glm::mat4 model;
};

Game & Game::Instance()
{
    static Game game;
    return game;
}

bool Game::initSDL()
{
    return 0 > SDL_Init(SDL_INIT_EVERYTHING);
}

void Game::initWindowHints()
{
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
}

bool Game::createWindow()
{
    window = SDL_CreateWindow(WINDOW_TITLE, WINDOW_POS, WINDOW_POS, WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_ARGS);
    return nullptr == window;
}

bool Game::createGLContext()
{
    ctx = SDL_GL_CreateContext(window);
    return NULL == ctx;
}

bool Game::initGLAD()
{
    return !gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress);
}

bool Game::init()
{
    bool success = true;

    initWindowHints();

    if (initSDL())
    {
        std::cout << "SDL failed to init: " << SDL_GetError() << std::endl;
        success = false;
    }

    if (createWindow())
    {
        std::cout << "Window failed to create: " << SDL_GetError() << std::endl;
        success = false;
    }

    if (createGLContext())
    {
        std::cout << "Context creation failed: " << SDL_GetError() << std::endl;
        success = false;
    }

    if (initGLAD())
    {
        std::cout << "GLAD failed to init" << glGetError() << std::endl;
        success = false;
    }

    Running = true;

    std::fill_n(Keys, MAX_KEYS_LENGTH, false);

    SetClearColor(glm::vec4(0.6f, 0.0f, 0.6f, 1.0f));

    glEnable(GL_DEPTH_TEST);

    ecs.init();
    ecs.registerComponent<Renderable>();

    Entity square = ecs.createEntity();
    createSquare(square);

    return success;
}

void Game::SetWindowTitle(const char *window_title)
{
    SDL_SetWindowTitle(window, window_title);
}

void Game::SetClearColor(glm::vec4 color)
{
    clearColor = color;
    glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
}

void Game::createSquare(Entity entity)
{
    Renderable renderable;

    // Vertex data
    float vertices[] = {
        0.5f, 0.5f, 0.0f,
        0.5f, -0.5f, 0.0f,
        -0.5f, -0.5f, 0.0f,
        -0.5f, 0.5f, 0.0f};

    // Index data
    unsigned int indices[] = {
        0, 1, 3,
        1, 2, 3};

    // Generate and bind VAO
    glGenVertexArrays(1, &renderable.VAO);
    glBindVertexArray(renderable.VAO);

    // Generate and bind VBO
    glGenBuffers(1, &renderable.VBO);
    glBindBuffer(GL_ARRAY_BUFFER, renderable.VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Generate and bind EBO
    glGenBuffers(1, &renderable.EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderable.EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Set vertex attribute pointers
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    // Load shaders
    renderable.shaderProgram = ResourceLoader::LoadShaderGL("shaders/vert.glsl", "shaders/frag.glsl");

    // Set up model matrix
    renderable.model = glm::mat4(1.0f);

    // Add component to entity
    ecs.addComponent(entity, renderable);
}

float Game::calculateDeltaTime(unsigned int NOW, unsigned int &LAST)
{
    float deltaTime = (float)(NOW - LAST) / SDL_GetPerformanceFrequency();
    LAST = NOW;
    return deltaTime;
}

void Game::pollEvents()
{
    while (SDL_PollEvent(&Event))
    {
        switch (Event.type)
        {
        case SDL_QUIT:
            Running = false;
            break;
        case SDL_KEYDOWN:
            Keys[Event.key.keysym.sym] = true;
            break;
        case SDL_KEYUP:
            Keys[Event.key.keysym.sym] = false;
            break;
        default:
            break;
        }
    }
}

void Game::pollKeys()
{
    if (Keys[SDLK_ESCAPE])
        Running = false;
    
    for (int entityId = 0; entityId < MAX_ENTITIES; ++entityId) {
        Entity entity(entityId);
        
        // Check if the entity has a Renderable component
        if (ecs.getEntityManager()->getSignature(entity).test(ecs.getComponentType<Renderable>())) {
            auto& renderable = ecs.getComponent<Renderable>(entity);
            if(Keys[SDLK_w])
            {
                renderable.model = glm::translate(renderable.model, glm::vec3(0.0f, 1.0f, 0.0f) * DeltaTime);
            }
            if(Keys[SDLK_s])
            {
                renderable.model = glm::translate(renderable.model, glm::vec3(0.0f, -1.0f, 0.0f) * DeltaTime);
            }
            if(Keys[SDLK_a])
            {
                renderable.model = glm::translate(renderable.model, glm::vec3(-1.0f, 0.0f, 0.0f) * DeltaTime);
            }
            if(Keys[SDLK_d])
            {
                renderable.model = glm::translate(renderable.model, glm::vec3(1.0f, 0.0f, 0.0f) * DeltaTime);
            }
        }
    }
}

void Game::update()
{
    pollEvents();
    pollKeys();
}

void Game::render()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Set up view and projection matrices
    glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.1f, 100.0f);

    // Iterate through all entities
    for (int entityId = 0; entityId < MAX_ENTITIES; ++entityId) {
        Entity entity(entityId);
        
        // Check if the entity has a Renderable component
        if (ecs.getEntityManager()->getSignature(entity).test(ecs.getComponentType<Renderable>())) {
            auto& renderable = ecs.getComponent<Renderable>(entity);

            glUseProgram(renderable.shaderProgram);

            // Set uniforms
            GLuint modelLoc = glGetUniformLocation(renderable.shaderProgram, "model");
            GLuint viewLoc = glGetUniformLocation(renderable.shaderProgram, "view");
            GLuint projectionLoc = glGetUniformLocation(renderable.shaderProgram, "projection");
            GLuint colorLoc = glGetUniformLocation(renderable.shaderProgram, "color");

            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(renderable.model));
            glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
            glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
            glUniform4fv(colorLoc, 1, glm::value_ptr(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f)));

            // Draw the square
            glBindVertexArray(renderable.VAO);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        }
    }

    SDL_GL_SwapWindow(window);
}


void Game::Run()
{
    if (!init())
        return;

    unsigned int NOW = SDL_GetPerformanceCounter(), LAST = SDL_GetPerformanceCounter();
    float frameTime = (1.0f / MAX_FRAMERATE) * 1000.0f;
    while (Running)
    {
        NOW = SDL_GetPerformanceCounter();
        DeltaTime = calculateDeltaTime(NOW, LAST);
        std::cout << DeltaTime << std::endl;
        update();
        render();
        float timeToWait = frameTime - (DeltaTime * 1000.0f);
        if (timeToWait > 0)
            SDL_Delay(timeToWait);
    }
}

void Game::Close()
{
    if (ctx)
        SDL_GL_DeleteContext(ctx);

    if (window)
        SDL_DestroyWindow(window);

    SDL_Quit();
}
