#include "Actor.h"


Actor::Actor() : id(-1), name("") {};

Actor& Actor::operator=(const Actor& other) {
    if (this != &other) {
        name = other.name;

        for (const auto& otherPair : other.components) {
            luabridge::LuaRef parentScript = otherPair.second;

            if (parentScript["type"].tostring() == "Rigidbody") {
                luabridge::LuaRef newRB = ComponentManager::CreateNewRigidbody(parentScript, this);
                InjectConvenienceReference(newRB);
                components.insert(std::pair(otherPair.first, newRB));
                componentsByType[newRB["type"].tostring()].insert(otherPair.first);
                continue;
            }

            luabridge::LuaRef instanceScript = luabridge::newTable(luaState);
            ComponentManager::EstablishInheritance(instanceScript, parentScript);
            InjectConvenienceReference(instanceScript);
            components.insert(std::pair(otherPair.first, instanceScript));
            componentsByType[instanceScript["type"].tostring()].insert(otherPair.first);
        }
    }

    return *this;
}

std::string Actor::GetName() {
    return name;
}

int Actor::GetID() {
    return id;
}

void Actor::InjectConvenienceReference(luabridge::LuaRef component) {
    component["actor"] = this;
}

luabridge::LuaRef Actor::GetComponentByKey(const std::string& key) {
    const auto& it = components.find(key);

    if (it == std::end(components))
        return luabridge::LuaRef(luaState);

    // Don't return any components queued for removal
    if (componentsToRemove.find(key) != std::end(componentsToRemove)) {
        return luabridge::LuaRef(luaState);
    }

    return it->second;
}

luabridge::LuaRef Actor::GetComponent(const std::string& type) {
    const auto& it = componentsByType.find(type);

    if (it == std::end(componentsByType))
        return luabridge::LuaRef(luaState);

    std::set<std::string>& foundComponents = it->second;

    if (foundComponents.empty())
        return luabridge::LuaRef(luaState);

    const std::string& key = *std::begin(foundComponents);

    // Don't return any components queued for removal
    if (componentsToRemove.find(key) != std::end(componentsToRemove)) {
        return luabridge::LuaRef(luaState);
    }

    return components.find(key)->second;
}

luabridge::LuaRef Actor::GetComponents(const std::string& type) {
    luabridge::LuaRef resultTable = luabridge::newTable(luaState);

    const auto& it = componentsByType.find(type);

    // If no components exist of this type
    if (it == std::end(componentsByType))
        return resultTable;

    std::set<std::string>& foundComponents = it->second;

    // If components did exist at some point but were removed
    if (foundComponents.empty())
        return resultTable;

    int i = 1;
    for (const std::string& componentKey : foundComponents) {
        if (componentsToRemove.find(componentKey) != std::end(componentsToRemove)) {
            continue;
        }
        resultTable[i] = components.find(componentKey)->second;
        i++;
    }

    return resultTable;
}

luabridge::LuaRef Actor::AddComponent(const std::string& type) {
    luabridge::LuaRef component = ComponentManager::LoadComponentRuntime(type);
    InjectConvenienceReference(component);
    componentAddQueue.push_back(component);
    
    return component;
}

void Actor::RemoveComponent(luabridge::LuaRef component) {
    component["enabled"] = false;
    componentsToRemove.insert(std::pair(component["key"].tostring(), component));
}

void Actor::ProcessComponentQueues() {
    for (luabridge::LuaRef component : componentAddQueue) {
        std::string key = component["key"].tostring();
        AddComponentLifecycle(component, key);
        component["enabled"] = true;
        components.insert(std::pair(key, component));
        componentsByType[component["type"].tostring()].insert(key);
    }

    for (const auto& component: componentsToRemove) {
        // Remove all references to the component on this actor

        // erase from typedComponents first bc it requires the component
        const std::string& type = component.second["type"];
        std::set<std::string>& typedComponents = componentsByType[type];
        typedComponents.erase(component.first);

        components.erase(component.first);
        onStartComponents.erase(component.first);
        onUpdateComponents.erase(component.first);
        onLateUpdateComponents.erase(component.first);
    }

    componentAddQueue.clear();
    componentsToRemove.clear();
}

void Actor::AddComponentLifecycle(luabridge::LuaRef component, const std::string& key) {
    luabridge::LuaRef lifecycleFunction = component["OnStart"];
    if (lifecycleFunction.isFunction()) {
        onStartComponents.insert(std::pair(key, component));
    }

    lifecycleFunction = component["OnUpdate"];
    if (lifecycleFunction.isFunction()) {
        onUpdateComponents.insert(std::pair(key, component));
    }

    lifecycleFunction = component["OnLateUpdate"];
    if (lifecycleFunction.isFunction()) {
        onLateUpdateComponents.insert(std::pair(key, component));
    }

    lifecycleFunction = component["OnCollisionEnter"];
    if (lifecycleFunction.isFunction()) {
        onCollisionEnterComponents.insert(std::pair(key, component));
    }

    lifecycleFunction = component["OnCollisionExit"];
    if (lifecycleFunction.isFunction()) {
        onCollisionExitComponents.insert(std::pair(key, component));
    }

    lifecycleFunction = component["OnTriggerEnter"];
    if (lifecycleFunction.isFunction()) {
        onTriggerEnterComponents.insert(std::pair(key, component));
    }

    lifecycleFunction = component["OnTriggerExit"];
    if (lifecycleFunction.isFunction()) {
        onTriggerExitComponents.insert(std::pair(key, component));
    }

    lifecycleFunction = component["OnDestroy"];
    if (lifecycleFunction.isFunction()) {
        onDestroyComponents.insert(std::pair(key, component));
    }
}

bool Actor::HasCollisionEnterComponents() {
    return onCollisionEnterComponents.size() > 0;
}

bool Actor::HasCollisionExitComponents() {
    return onCollisionExitComponents.size() > 0;
}

void Actor::SetLuaState(lua_State* L) {
    luaState = L;
}

void Actor::ProcessDestroyedComponents() {
    for (const auto& component: componentsToRemove) {
        if (onDestroyComponents.find(component.first) == std::end(onDestroyComponents))
            continue;

        try {
            component.second["OnDestroy"](component.second);
        }
        catch (luabridge::LuaException e) {
            std::string errorMessage = e.what();
            std::replace(std::begin(errorMessage), std::end(errorMessage), '\\', '/');
            std::cout << "\033[31m" << GetName() << " : " << errorMessage << "\033[0m" << std::endl;
        }
    }
}

void Actor::SetupForDestruction() {
    for (const auto& component : components) {
        componentsToRemove.insert(std::pair(component.first, component.second));
    }
}
