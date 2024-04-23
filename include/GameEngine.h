// GameEngine.h
#pragma once
#define SDL_MAIN_HANDLED


#include <chrono>
#include <string>
#include <vector>
#include "SDL2/SDL.h"
#include "lua/lua.hpp"
#include "ComponentManager.h"
#include "box2d/box2d.h"
#include "ContactListener.h"

class ContactListener;

class GameEngine {
public:
    GameEngine();
    void Initialize();
    void Run();
    static lua_State* GetLuaState();
    static std::shared_ptr<b2World> GetPhysicsWorld();

private:
    bool running;
    SDL_Window* window;
    SDL_Renderer* renderer;
    static inline lua_State* luaState;

    static inline float deltaTime;
    static inline std::chrono::high_resolution_clock::time_point lastFrameTime;

    // Physics config
    static inline std::shared_ptr<b2World> world;
    std::shared_ptr<ContactListener> contactListener;
    float timeStep = 1.0f / 60.0f;

    // Config variables
    std::string windowTitle = "";
    int windowWidth = 640, windowHeight = 360;
    int clearColorR = 255, clearColorG = 255, clearColorB = 255;     

    void LoadResources();
    void InitializeLua();
    void InitializeB2D();
    void ProcessInput();
    void Update();
    void Render();
    void StepPhysics();

    // Application Scripting API
    static void ApplicationQuit();
    static void ApplicationSleep(int ms);
    static void ApplicationOpenURL(const std::string& url);
    static float ApplicationTime();
    
};
