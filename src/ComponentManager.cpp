// ComponentManager.cpp
#include "ComponentManager.h"
#include "filesystem"

bool ComponentManager::CheckLuaState() {
    if (luaState == nullptr) {
        std::cout << "Lua state not set in component manager class!" << std::endl;
        exit(0);
    }
    return false;
}

luabridge::LuaRef ComponentManager::LoadComponent(const std::string& componentKey, const std::string& componentName) {
    // Load C++ Components
    if (componentName == "Rigidbody") {
        std::shared_ptr<Rigidbody> rb = std::make_shared<Rigidbody>();
        luabridge::push(luaState, rb.get());
        luabridge::LuaRef component = luabridge::LuaRef::fromStack(luaState, -1);

        rigidbodys.push_back(std::pair(component, rb));
        rb->key = componentKey;

        components.insert(std::pair(componentName, component));
        return component;
    }

    // Load Lua Components
    auto it = components.find(componentName);

    if (it == components.end()) {
        std::string filePath = componentFolderPath + componentName + ".lua";

        if (!std::filesystem::exists(filePath)) {
            std::cout << "error: failed to locate component " << componentName;
            exit(0);
        }

        // Load component and check if it is a valid lua file
        if (luaL_dofile(luaState, filePath.c_str()) != 0) {
            const char* lua_error_msg = lua_tostring(luaState, -1); // Get error message from top of Lua stack
            std::cerr << "\033[31m" << "Error in Lua file '" << componentName << "': " << lua_error_msg << "\033[0m" << std::endl;
            exit(0);
        }

        luabridge::LuaRef component = luabridge::getGlobal(luaState, componentName.c_str());
        component["type"] = componentName;
        component["enabled"] = true;
        components.insert(std::pair(componentName, component));
        it = components.find(componentName);
    }

    luabridge::LuaRef parentScript = it->second;
    luabridge::LuaRef instanceScript = luabridge::newTable(luaState);
    EstablishInheritance(instanceScript, parentScript);

    // Add self.key component member
    instanceScript["key"] = componentKey;

    return instanceScript;
}

luabridge::LuaRef ComponentManager::LoadComponentRuntime(const std::string& componentName) {
    int& componentAddCounter = addComponentsCounter[componentName];
    std::string componentKey = "r" + std::to_string(componentAddCounter);
    componentAddCounter++;

    // Load C++ Components
    if (componentName == "Rigidbody") {
        std::shared_ptr<Rigidbody> rb = std::make_shared<Rigidbody>();
        luabridge::push(luaState, rb.get());
        luabridge::LuaRef component = luabridge::LuaRef::fromStack(luaState, -1);

        rigidbodys.push_back(std::pair(component, rb));

        rb->key = componentKey;
        rb->enabled = false;


        components.insert(std::pair(componentName, component));
        return component;
    }

    auto it = components.find(componentName);

    if (it == components.end()) {
        std::string filePath = componentFolderPath + componentName + ".lua";

        if (!std::filesystem::exists(filePath)) {
            std::cout << "error: failed to locate component " << componentName;
            exit(0);
        }

        // Load component and check if it is a valid lua file
        if (luaL_dofile(luaState, filePath.c_str()) != 0) {
            std::cout << "problem with lua file " << componentName;
            exit(0);
        }

        luabridge::LuaRef component = luabridge::getGlobal(luaState, componentName.c_str());
        component["type"] = componentName;
        component["enabled"] = true;
        components.insert(std::pair(componentName, component));
        it = components.find(componentName);
    }

    luabridge::LuaRef parentScript = it->second;
    luabridge::LuaRef instanceScript = luabridge::newTable(luaState);
    EstablishInheritance(instanceScript, parentScript);

    // Add self.key and self.enabled component member
    instanceScript["key"] = componentKey;
    instanceScript["enabled"] = false;

    return instanceScript;
}

void ComponentManager::EstablishInheritance(luabridge::LuaRef instanceTable, luabridge::LuaRef parentTable) {
    luabridge::LuaRef newMetatable = luabridge::newTable(luaState);
    newMetatable["__index"] = parentTable;

    instanceTable.push(luaState);
    newMetatable.push(luaState);
    lua_setmetatable(luaState, -2);
    lua_pop(luaState, 1);
}

luabridge::LuaRef ComponentManager::CreateNewRigidbody(luabridge::LuaRef originalRigidbodyComponent, Actor* actorPtr) {
    std::shared_ptr<Rigidbody> originalRb;

    for (const auto& pair : rigidbodys) {
        if (pair.first == originalRigidbodyComponent) {
            originalRb = pair.second;
        }
    }

    std::shared_ptr<Rigidbody> newRb = originalRb->Clone(actorPtr);

    luabridge::push(luaState, newRb.get());
    luabridge::LuaRef component = luabridge::LuaRef::fromStack(luaState, -1);

    rigidbodys.push_back(std::pair(component, newRb));

    components.insert(std::pair(originalRigidbodyComponent["type"].tostring(), component));

    return component;
}

void ComponentManager::SetState(lua_State* s) {
    luaState = s;
}

void ComponentManager::CppLog(const std::string& message) {
    std::cout << message << std::endl;
}

void ComponentManager::CppLogError(const std::string& message) {
    std::cerr << message << std::endl;
}
