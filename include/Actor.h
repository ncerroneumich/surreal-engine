#pragma once

#include <string>
#include <queue>
#include <unordered_map>
#include <set>
#include "lua/lua.hpp"
#include "LuaBridge/LuaBridge.h"
#include "ComponentManager.h"


class Actor
{
public:
    Actor();
    Actor& operator=(const Actor& other);
    bool enabled = true;

    std::string GetName();
    int GetID();

    luabridge::LuaRef GetComponentByKey(const std::string& key);
    luabridge::LuaRef GetComponent(const std::string& type);
    luabridge::LuaRef GetComponents(const std::string& type);
    luabridge::LuaRef AddComponent(const std::string& type);
    void RemoveComponent(luabridge::LuaRef component);
    void ProcessComponentQueues();
    void InjectConvenienceReference(luabridge::LuaRef component);
    void AddComponentLifecycle(luabridge::LuaRef component, const std::string& key);
    bool HasCollisionEnterComponents();
    bool HasCollisionExitComponents();
    void SetupForDestruction();
    void ProcessDestroyedComponents();
    static void SetLuaState(lua_State* L);

    int id;
    std::string name;
    std::unordered_map<std::string, luabridge::LuaRef> components;
    std::unordered_map<std::string, std::set<std::string>> componentsByType;
    std::map<std::string, luabridge::LuaRef> onStartComponents, onUpdateComponents, onLateUpdateComponents, onDestroyComponents;
    std::map<std::string, luabridge::LuaRef> onCollisionEnterComponents, onCollisionExitComponents, onTriggerEnterComponents, onTriggerExitComponents;
    std::vector<luabridge::LuaRef> componentAddQueue;
    std::map<std::string, luabridge::LuaRef> componentsToRemove;
    static inline lua_State* luaState;
};
