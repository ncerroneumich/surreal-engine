#pragma once

#include "Actor.h"
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <map>
#include <vector>
#include <memory>
#include "TemplateManager.h"
#include "ReadJsonFile.h"
#include "lua/lua.hpp"
#include "LuaBridge/LuaBridge.h"

enum LifecycleFunctionType { OnStart = 0, OnUpdate = 1, OnLateUpdate = 2 };

class SceneManager
{
public:
    static void LoadScene(const std::string& sceneName);
    static void RunOnStartLifecycleFunctions();
    static void RunOnUpdateLifecycleFunctions();
    static void RunOnLateUpdateLifecycleFunctions();
    static void RunOnDestroyLifecycleFunctions();
    static luabridge::LuaRef GetActor(const std::string& actorName);
    static luabridge::LuaRef GetAllActors(const std::string& actorName);
    static void UpdateAllActorComponents();
    static Actor* InstantiateActor(const std::string& templateName);
    static void DestroyActor(Actor* actor);
    static void ProcessActorQueues();
    static void LoadSceneRuntime(const std::string& sceneName);
    static std::string GetCurrentSceneName();
    static void DontDestroy(Actor* actor);
    static void SetLuaState(lua_State* state);
    static inline bool loadingNewScene = false;
    static inline std::string nextSceneName;

private:
    static inline std::string sceneName;
    static inline uint32_t totalActors = 0;
    static inline std::vector<std::shared_ptr<Actor>> actorVector;
    static inline std::vector<std::shared_ptr<Actor>> dontDestroyOnLoadActors;
    static inline std::unordered_map<std::string, std::vector<std::shared_ptr<Actor>>> actorMap;
    static inline std::vector<std::shared_ptr<Actor>> actorsToAdd;
    static inline std::unordered_set<Actor*> actorsToRemove;
    static void ParseScene(const rapidjson::Document& sceneDoc);
    static bool IsActorFlaggedForRemoval(Actor* actor);
    static inline lua_State* luaState;
};
