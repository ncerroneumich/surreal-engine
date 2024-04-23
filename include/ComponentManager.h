// ComponentManager.h
#pragma once

#include <unordered_map>
#include <unordered_set>
#include <string>
#include "lua/lua.hpp"
#include "LuaBridge/LuaBridge.h"
#include "Rigidbody.h"


struct CompareComponent {
    bool operator()(luabridge::LuaRef a, luabridge::LuaRef b) {
        return a["key"].tostring() < b["key"].tostring();
    }
};

class ComponentManager
{
public:
    static void SetState(lua_State* lua_State);
    static luabridge::LuaRef LoadComponent(const std::string& componentKey, const std::string& componentName);
    static luabridge::LuaRef LoadComponentRuntime(const std::string& componentName);
    static void EstablishInheritance(luabridge::LuaRef instanceTable, luabridge::LuaRef parentTable);
    static luabridge::LuaRef CreateNewRigidbody(luabridge::LuaRef originalRigidbodyComponent, Actor* actorPtr);
    static void CppLog(const std::string& message);
    static void CppLogError(const std::string& message);

private:
    ComponentManager();
    static bool CheckLuaState(); // Checks if luaState variable is set
    static void AddLifecycleFunctions(const std::string& componentName, luabridge::LuaRef component);

    static inline lua_State* luaState;
    static inline std::unordered_map<std::string, luabridge::LuaRef> components;
    static inline std::vector<std::pair<luabridge::LuaRef, std::shared_ptr<Rigidbody>>> rigidbodys;
    static inline std::string componentFolderPath = "resources/component_types/";
    // Keeps track of the number of times a component of a certain type has been added
    static inline std::unordered_map<std::string, int> addComponentsCounter;
};

