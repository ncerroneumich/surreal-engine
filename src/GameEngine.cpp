// GameEngine.cpp
#include "AudioManager.h"
#include "Camera2D.h"
#include "GameEngine.h"
#include "glm/glm.hpp"
#include "ImageManager.h"
#include "Input.h"
#include "rapidjson/document.h"
#include "ReadJsonFile.h"
#include "SceneManager.h"
#include "SDL2/SDL_scancode.h"
#include <iostream>
#include <TextManager.h>
#include <thread>
#include "Rigidbody.h"
#include "RayCast.h"
#include "EventBus.h"


GameEngine::GameEngine() : running(true), window(nullptr), renderer(nullptr) {}

void GameEngine::Initialize() {
    Input::Init();
    InitializeLua();
    InitializeB2D();
    LoadResources();

    Camera2D::Initialize(windowWidth, windowHeight);

    SDL_Init(SDL_INIT_VIDEO);
    TextManager::Initialize(renderer);
    ImageManager::Initialize(renderer);
    AudioManager::Initialize();

    lastFrameTime = std::chrono::high_resolution_clock::now();
}

void GameEngine::InitializeLua() {
    luaState = luaL_newstate();
    luaL_openlibs(luaState);

    SceneManager::SetLuaState(luaState);
    ComponentManager::SetState(luaState);
    Actor::SetLuaState(luaState);
    RayCast::SetLuaState(luaState);

    // Add custom functions
    luabridge::getGlobalNamespace(luaState)
        .beginNamespace("Debug")
        .addFunction("Log", &ComponentManager::CppLog)
        .addFunction("LogError", &ComponentManager::CppLogError)
        .endNamespace();

    // Actor API
    luabridge::getGlobalNamespace(luaState)
        .beginClass<Actor>("Actor")
        .addFunction("GetName", &Actor::GetName)
        .addFunction("GetID", &Actor::GetID)
        .addFunction("GetComponentByKey", &Actor::GetComponentByKey)
        .addFunction("GetComponent", &Actor::GetComponent)
        .addFunction("GetComponents", &Actor::GetComponents)
        .addFunction("AddComponent", &Actor::AddComponent)
        .addFunction("RemoveComponent", &Actor::RemoveComponent)
        .endClass();

    luabridge::getGlobalNamespace(luaState)
        .beginNamespace("Actor")
        .addFunction("Find", &SceneManager::GetActor)
        .addFunction("FindAll", &SceneManager::GetAllActors)
        .addFunction("Instantiate", &SceneManager::InstantiateActor)
        .addFunction("Destroy", &SceneManager::DestroyActor)
        .endNamespace();

    // Application API
    luabridge::getGlobalNamespace(luaState)
        .beginNamespace("Application")
        .addFunction("Quit", &GameEngine::ApplicationQuit)
        .addFunction("Sleep", &GameEngine::ApplicationSleep)
        .addFunction("OpenURL", &GameEngine::ApplicationOpenURL)
        .endNamespace();

    // Time API
    luabridge::getGlobalNamespace(luaState)
        .beginNamespace("Time")
        .addFunction("GetCurrent", &GameEngine::ApplicationTime)
        .addVariable("deltaTime", &GameEngine::deltaTime, false)
        .endNamespace();

    luabridge::getGlobalNamespace(luaState)
        .beginClass<glm::vec2>("vec2")
        .addProperty("x", &glm::vec2::x)
        .addProperty("y", &glm::vec2::y)
        .endClass();

    // Input API
    luabridge::getGlobalNamespace(luaState)
        .beginNamespace("Input")
        .addFunction("GetKey", &Input::GetKey)
        .addFunction("GetKeyDown", &Input::GetKeyDown)
        .addFunction("GetKeyUp", &Input::GetKeyUp)
        .addFunction("GetMousePosition", &Input::GetMousePosition)
        .addFunction("GetMouseButton", &Input::GetMouseButton)
        .addFunction("GetMouseButtonDown", &Input::GetMouseButtonDown)
        .addFunction("GetMouseButtonUp", &Input::GetMouseButtonUp)
        .addFunction("GetMouseScrollDelta", &Input::GetMouseScrollDelta)
        .addFunction("GetButton", &Input::GetControllerButton)
        .addFunction("GetButtonDown", &Input::GetControllerButtonDown)
        .addFunction("GetButtonUp", &Input::GetControllerButtonUp)
        .addFunction("GetAxis", &Input::GetControllerAxis)
        .endNamespace();

    // Text Scripting API
    luabridge::getGlobalNamespace(luaState)
        .beginNamespace("Text")
        .addFunction("Draw", &TextManager::SubmitTextDrawRequest)
        .endNamespace();

    // Audio Scripting API
    luabridge::getGlobalNamespace(luaState)
        .beginNamespace("Audio")
        .addFunction("Play", AudioManager::Play)
        .addFunction("GetPosition", AudioManager::GetPosition)
        .addFunction("SetVolume", AudioManager::SetVolume)
        .addFunction("Stop", AudioManager::Stop)
        .endNamespace();

    // Image Scripting API
    luabridge::getGlobalNamespace(luaState)
        .beginNamespace("Image")
        .addFunction("DrawUI", &ImageManager::SubmitUIDrawRequest)
        .addFunction("DrawUIEx", &ImageManager::SubmitUIExDrawRequest)
        .addFunction("Draw", &ImageManager::SubmitScreenSpaceDrawRequest)
        .addFunction("DrawEx", &ImageManager::SubmitScreenSpaceExDrawRequest)
        .addFunction("DrawPixel", &ImageManager::SubmitPixelDrawRequest)
        .endNamespace();

    // Camera API
    luabridge::getGlobalNamespace(luaState)
        .beginNamespace("Camera")
        .addFunction("SetPosition", &Camera2D::SetPosition)
        .addFunction("GetPositionX", &Camera2D::GetPositionX)
        .addFunction("GetPositionY", &Camera2D::GetPositionY)
        .addFunction("SetZoom", &Camera2D::SetZoomFactor)
        .addFunction("GetZoom", &Camera2D::GetZoomFactor)
        .addFunction("GetEase", &Camera2D::GetEaseFactor)
        .addFunction("SetEase", &Camera2D::SetEaseFactor)
        .endNamespace();

    // Scene API
    luabridge::getGlobalNamespace(luaState)
        .beginNamespace("Scene")
        .addFunction("Load", &SceneManager::LoadSceneRuntime)
        .addFunction("GetCurrent", &SceneManager::GetCurrentSceneName)
        .addFunction("DontDestroy", &SceneManager::DontDestroy)
        .endNamespace();

    // Physics API
    luabridge::getGlobalNamespace(luaState)
        .beginClass<b2Vec2>("Vector2")
        .addConstructor<void(*) (float, float)>()
        .addProperty("x", &b2Vec2::x)
        .addProperty("y", &b2Vec2::y)
        .addFunction("Normalize", &b2Vec2::Normalize)
        .addFunction("Length", &b2Vec2::Length)
        .addFunction("__add", &b2Vec2::operator_add)
        .addFunction("__sub", &b2Vec2::operator_sub)
        .addFunction("__mul", &b2Vec2::operator_mul)
        .addStaticFunction("Distance", &b2Distance)
        .addStaticFunction("Dot", static_cast<float (*)(const b2Vec2&, const b2Vec2&)>(&b2Dot))
        .endClass();

    luabridge::getGlobalNamespace(luaState)
        .beginClass<Rigidbody>("Rigidbody")
        .addFunction("GetPosition", &Rigidbody::GetPosition)
        .addFunction("GetRotation", &Rigidbody::GetRotation)
        .addFunction("AddForce", &Rigidbody::AddForce)
        .addFunction("SetVelocity", &Rigidbody::SetVelocity)
        .addFunction("SetPosition", &Rigidbody::SetPosition)
        .addFunction("SetRotation", &Rigidbody::SetRotation)
        .addFunction("SetAngularVelocity", &Rigidbody::SetAngularVelocity)
        .addFunction("GetGravityScale", &Rigidbody::GetGravityScale)
        .addFunction("SetGravityScale", &Rigidbody::SetGravityScale)
        .addFunction("SetUpDirection", &Rigidbody::SetUpDirection)
        .addFunction("GetVelocity", &Rigidbody::GetVelocity)
        .addFunction("GetAngularVelocity", &Rigidbody::GetAngularVelocity)
        .addFunction("GetUpDirection", &Rigidbody::GetUpDirection)
        .addFunction("SetRightDirection", &Rigidbody::SetRightDirection)
        .addFunction("GetRightDirection", &Rigidbody::GetRightDirection)
        .addFunction("OnStart", &Rigidbody::OnStart)
        .addProperty("actor", &Rigidbody::actor)
        .addProperty("enabled", &Rigidbody::enabled)
        .addProperty("key", &Rigidbody::key)
        .addProperty("type", &Rigidbody::type)
        .addProperty("x", &Rigidbody::x)
        .addProperty("y", &Rigidbody::y)
        .addProperty("rotation", &Rigidbody::rotationDegrees)
        .addProperty("body_type", &Rigidbody::bodyTypeString)
        .addProperty("precise", &Rigidbody::precise)
        .addProperty("gravity_scale", &Rigidbody::gravityScale)
        .addProperty("density", &Rigidbody::density)
        .addProperty("angular_friction", &Rigidbody::angularFriction)
        .addProperty("has_collider", &Rigidbody::hasCollider)
        .addProperty("has_trigger", &Rigidbody::hasTrigger)
        .addProperty("collider_type", &Rigidbody::colliderType)
        .addProperty("trigger_type", &Rigidbody::triggerType)
        .addProperty("width", &Rigidbody::width)
        .addProperty("height", &Rigidbody::height)
        .addProperty("radius", &Rigidbody::radius)
        .addProperty("trigger_width", &Rigidbody::triggerWidth)
        .addProperty("trigger_height", &Rigidbody::triggerHeight)
        .addProperty("trigger_radius", &Rigidbody::triggerRadius)
        .addProperty("friction", &Rigidbody::friction)
        .addProperty("bounciness", &Rigidbody::bounciness)
        .endClass();

    luabridge::getGlobalNamespace(luaState)
        .beginClass<Collision>("Collision")
        .addProperty("other", &Collision::other)
        .addProperty("point", &Collision::point)
        .addProperty("relative_velocity", &Collision::relative_velocity)
        .addProperty("normal", &Collision::normal)
        .endClass();

    luabridge::getGlobalNamespace(luaState)
        .beginClass<HitResult>("HitResult")
        .addProperty("actor", &HitResult::actor)
        .addProperty("point", &HitResult::point)
        .addProperty("is_trigger", &HitResult::is_trigger)
        .addProperty("normal", &HitResult::normal)
        .endClass();

    luabridge::getGlobalNamespace(luaState)
        .beginNamespace("Physics")
        .addFunction("Raycast", &RayCast::Raycast)
        .addFunction("RaycastAll", &RayCast::RaycastAll)
        .endNamespace();

    // EventBus API
    luabridge::getGlobalNamespace(luaState)
        .beginNamespace("Event")
        .addFunction("Publish", &EventBus::Publish)
        .addFunction("Subscribe", &EventBus::Subscribe)
        .addFunction("Unsubscribe", &EventBus::Unsubscribe)
        .endNamespace();
}

void GameEngine::InitializeB2D() {
    b2Vec2 gravity(0.0f, 9.8);
    world = std::make_shared<b2World>(gravity);
    contactListener = std::make_shared<ContactListener>();
    world->SetContactListener(contactListener.get());
    Rigidbody::SetWorld(world);
    RayCast::SetPhysicsWorld(world);
}

lua_State* GameEngine::GetLuaState() {
    return luaState;
}

std::shared_ptr<b2World> GameEngine::GetPhysicsWorld() {
    return world;
}

void GameEngine::Run() {
    while (running) {
        auto now = std::chrono::high_resolution_clock::now();
        deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(now - lastFrameTime).count();
        lastFrameTime = now;
        ProcessInput();
        Update();
        Render();
    }
}

void GameEngine::ProcessInput() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        Input::ProcessEvent(event);

        if (event.type == SDL_QUIT) {
            running = false;
        }
    }
}

void GameEngine::Update() {
    if (SceneManager::loadingNewScene)
        SceneManager::LoadScene(SceneManager::nextSceneName);

    SceneManager::RunOnStartLifecycleFunctions();
    SceneManager::RunOnUpdateLifecycleFunctions();
    SceneManager::RunOnLateUpdateLifecycleFunctions();
    SceneManager::RunOnDestroyLifecycleFunctions();
    Input::LateUpdate();

    SceneManager::UpdateAllActorComponents(); // Handles removing and adding of components on actors
    SceneManager::ProcessActorQueues();
    EventBus::HandleSubscriptionQueues();

    Camera2D::UpdateCameraPosition(deltaTime);
    StepPhysics();
}

void GameEngine::Render() {
    SDL_SetRenderDrawColor(renderer, clearColorR, clearColorG, clearColorB, SDL_ALPHA_OPAQUE); // In case a pixel draw call changed it
    SDL_RenderClear(renderer);

    ImageManager::DrawScreenSpaceRequestQueue();
    ImageManager::DrawUIRequestQueue();
    TextManager::DrawRequestQueue();
    ImageManager::DrawPixelRequestQueue();

    SDL_RenderPresent(renderer);
}

void GameEngine::LoadResources() {
    std::string resourcesPath = "resources/";

    // Required directory
    if (!std::filesystem::exists(resourcesPath)) {
        std::cout << "error: resources/ missing";
        exit(0);
    }

    std::string configPath = resourcesPath + "game.config";

    // Required file
    if (!std::filesystem::exists(configPath)) {
        std::cout << "error: resources/game.config missing";
        exit(0);
    }

    // Rendering config
    rapidjson::Document rendering;
    std::string renderingPath = resourcesPath + "rendering.config";

    if (std::filesystem::exists(renderingPath)) {
        ReadJsonFile(renderingPath, rendering);

        if (rendering.HasMember("x_resolution")) {
            windowWidth = rendering["x_resolution"].GetInt();
            Camera2D::size.x = windowWidth;
        }

        if (rendering.HasMember("y_resolution")) {
            windowHeight = rendering["y_resolution"].GetInt();
            Camera2D::size.y = windowHeight;
        }

        if (rendering.HasMember("clear_color_r")) {
            clearColorR = rendering["clear_color_r"].GetInt();
        }

        if (rendering.HasMember("clear_color_g")) {
            clearColorG = rendering["clear_color_g"].GetInt();
        }

        if (rendering.HasMember("clear_color_b")) {
            clearColorB = rendering["clear_color_b"].GetInt();
        }
    }

    // Game config
    rapidjson::Document config;
    ReadJsonFile(configPath, config);

    if (config.HasMember("game_title")) {
        windowTitle = config["game_title"].GetString();
    }


    // Set window and renderer ASAP to avoid load order problems
    window = SDL_CreateWindow(windowTitle.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, windowWidth, windowHeight, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
    SDL_SetRenderDrawColor(renderer, clearColorR, clearColorG, clearColorB, SDL_ALPHA_OPAQUE);

    if (config.HasMember("initial_scene")) {
        const std::string& sceneName = config["initial_scene"].GetString();
        SceneManager::LoadScene(sceneName);
    }
}

void GameEngine::ApplicationQuit() {
    exit(0);
}

void GameEngine::ApplicationSleep(int ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

void GameEngine::ApplicationOpenURL(const std::string& url) {
#if defined(_WIN32)
    std::string command = "start " + url;
#elif defined(__APPLE__)
    std::string command = "open " + url;
#else
    std::string command = "xdg-open " + url;
#endif

    std::system(command.c_str());
}

float GameEngine::ApplicationTime() {
    auto now = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::duration<float>>(now.time_since_epoch());
    return duration.count();
}

void GameEngine::StepPhysics() {
    world->Step(deltaTime, 8, 3);
}
