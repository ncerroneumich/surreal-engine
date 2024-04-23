#include "SceneManager.h"

static std::shared_ptr<Actor> ParseActor(const rapidjson::Value& actorDocument) {
    std::shared_ptr<Actor> actor = std::make_shared<Actor>();
    std::shared_ptr<Actor> actorTemplate;
    std::unordered_set<std::string> templateComponentKeys;

    if (actorDocument.HasMember("template")) {
        actorTemplate = TemplateManager::LoadTemplate(actorDocument["template"].GetString());
        *actor = *actorTemplate;

        // Add lifecycle for inherited components
        for (std::pair<std::string, luabridge::LuaRef> componentPair : actor->components) {
            luabridge::LuaRef component = componentPair.second;
            std::string componentKey = componentPair.first;
            actor->InjectConvenienceReference(component);
            templateComponentKeys.insert(componentKey);
            actor->AddComponentLifecycle(component, componentKey);
        }
    }

    if (actorDocument.HasMember("name")) {
        actor->name = actorDocument["name"].GetString();
    }

    if (actorDocument.HasMember("components")) {
        const rapidjson::GenericObject componentsObject = actorDocument["components"].GetObject();

        // Iterate through all components in this actor
        for (rapidjson::Value::ConstMemberIterator componentsItr = componentsObject.MemberBegin(); componentsItr != componentsObject.MemberEnd(); componentsItr++) {
            std::string componentKey = componentsItr->name.GetString();

            bool inherited = false;
            // If the component was inherited, mark it as so
            if (templateComponentKeys.find(componentKey) != std::end(templateComponentKeys))
                inherited = true;


            if (inherited) {
                luabridge::LuaRef component = actor->components.find(componentKey)->second;
                // Iterate through the type and overrides of each individual component
                for (rapidjson::Value::ConstMemberIterator componentMemberItr = componentsItr->value.MemberBegin(); componentMemberItr != componentsItr->value.MemberEnd(); componentMemberItr++) {

                    std::string componentMemberName = componentMemberItr->name.GetString();
                    // Handle component member override
                    if (componentMemberName != "type") {
                        std::string componentMemberToOverride = componentMemberItr->name.GetString();
                        const rapidjson::Value& overrideValue = componentMemberItr->value;

                        // Override the component member with the new value
                        if (overrideValue.IsString()) {
                            component[componentMemberToOverride] = overrideValue.GetString();
                        }
                        else if (overrideValue.IsInt()) {
                            component[componentMemberToOverride] = overrideValue.GetInt();
                        }
                        else if (overrideValue.IsDouble()) {
                            component[componentMemberToOverride] = overrideValue.GetDouble();
                        }
                        else if (overrideValue.IsBool()) {
                            component[componentMemberToOverride] = overrideValue.GetBool();
                        }
                    }
                }

                actor->AddComponentLifecycle(component, componentKey);
            }

            else {
                std::string componentFileName = componentsItr->value["type"].GetString();
                luabridge::LuaRef component = ComponentManager::LoadComponent(componentKey, componentFileName);
                actor->InjectConvenienceReference(component);
                actor->components.insert(std::pair(componentKey, component));
                actor->componentsByType[componentFileName].insert(componentKey);

                // Iterate through the type and overrides of each individual component
                for (rapidjson::Value::ConstMemberIterator componentMemberItr = componentsItr->value.MemberBegin(); componentMemberItr != componentsItr->value.MemberEnd(); componentMemberItr++) {

                    std::string componentMemberName = componentMemberItr->name.GetString();
                    // Handle component member override
                    if (componentMemberName != "type") {
                        const rapidjson::Value& overrideValue = componentMemberItr->value;

                        // Override the component member with the new value
                        if (overrideValue.IsString()) {
                            component[componentMemberName] = overrideValue.GetString();
                        }
                        else if (overrideValue.IsInt()) {
                            component[componentMemberName] = overrideValue.GetInt();
                        }
                        else if (overrideValue.IsDouble()) {
                            component[componentMemberName] = overrideValue.GetDouble();
                        }
                        else if (overrideValue.IsBool()) {
                            component[componentMemberName] = overrideValue.GetBool();
                        }
                    }
                }

                actor->AddComponentLifecycle(component, componentKey);
            }
        }
    }

    return actor;
}

void SceneManager::LoadScene(const std::string& name) {
    loadingNewScene = false;

    std::string scenePath = "resources/scenes/" + name + ".scene";

    if (!std::filesystem::exists(scenePath)) {
        std::cout << "error: scene " << name << " is missing";
        exit(0);
    }

    // Clear previous scene information
    actorVector.clear();

    actorVector = dontDestroyOnLoadActors;

    std::sort(std::begin(actorVector), std::end(actorVector), [](std::shared_ptr<Actor> a, std::shared_ptr<Actor> b) {
        return a->id < b->id;
    });

    actorMap.clear();

    for (std::shared_ptr<Actor> actor : actorVector) {
        actorMap[actor->name].push_back(actor);
    }

    sceneName = name;

    rapidjson::Document sceneDocument;
    ReadJsonFile(scenePath, sceneDocument);
    ParseScene(sceneDocument);
}

void SceneManager::ParseScene(const rapidjson::Document& sceneDoc) {
    const rapidjson::Value& actorsArray = sceneDoc["actors"];
    rapidjson::SizeType numActors = actorsArray.Size();
    // reserve memory in the actors array in advance
    actorVector.reserve(numActors);
    rapidjson::SizeType i;

    // Parse the actorObj into an scene actor
    for (i = 0; i < numActors; i++) {
        const rapidjson::Value& actorObj = actorsArray[i];
        std::shared_ptr<Actor> actor = ParseActor(actorObj);
        actor->id = i + totalActors;
        actorVector.push_back(actor);
        actorMap[actor->name].push_back(actor);
    }

    totalActors += i;
}

void SceneManager::RunOnStartLifecycleFunctions() {
    for (std::shared_ptr<Actor> actor : actorVector) {
        std::map<std::string, luabridge::LuaRef>& onStartComponents = actor->onStartComponents;

        for (auto& pair : onStartComponents) {
            // If the actor gets disabled, don't finish running its components
            if (actor->enabled == false)
                break;

            luabridge::LuaRef component = pair.second;

            if (component["enabled"] == false)
                continue;
            try {
                component["OnStart"](component);
            }
            catch (luabridge::LuaException e) {
                std::string errorMessage = e.what();
                std::replace(std::begin(errorMessage), std::end(errorMessage), '\\', '/');
                std::cout << "\033[31m" << actor->GetName() << " : " << errorMessage << "\033[0m" << std::endl;
            }
        }

        onStartComponents.clear();
    }
}

void SceneManager::RunOnUpdateLifecycleFunctions() {
    for (std::shared_ptr<Actor> actor : actorVector) {
        std::map<std::string, luabridge::LuaRef>& onUpdateComponents = actor->onUpdateComponents;

        for (auto& pair : onUpdateComponents) {
            // If the actor gets disabled, don't finish running its components
            if (actor->enabled == false)
                break;
            luabridge::LuaRef component = pair.second;
            if (component["enabled"] == false)
                continue;
            try {
                component["OnUpdate"](component);
            }
            catch (luabridge::LuaException e) {
                std::string errorMessage = e.what();
                std::replace(std::begin(errorMessage), std::end(errorMessage), '\\', '/');
                std::cout << "\033[31m" << actor->GetName() << " : " << errorMessage << "\033[0m" << std::endl;
            }
        }
    }
}

void SceneManager::RunOnLateUpdateLifecycleFunctions() {
    for (std::shared_ptr<Actor> actor : actorVector) {
        std::map<std::string, luabridge::LuaRef>& onLateUpdateComponents = actor->onLateUpdateComponents;

        for (auto& pair : onLateUpdateComponents) {
            // If the actor gets disabled, don't finish running its components
            if (actor->enabled == false)
                break;

            luabridge::LuaRef component = pair.second;
            if (component["enabled"] == false)
                continue;
            try {
                component["OnLateUpdate"](component);
            }
            catch (luabridge::LuaException e) {
                std::string errorMessage = e.what();
                std::replace(std::begin(errorMessage), std::end(errorMessage), '\\', '/');
                std::cout << "\033[31m" << actor->GetName() << " : " << errorMessage << "\033[0m" << std::endl;
            }
        }
    }
}

void SceneManager::RunOnDestroyLifecycleFunctions() {
    for (std::shared_ptr<Actor> actor : actorVector) {
        actor->ProcessDestroyedComponents();
    }
}

luabridge::LuaRef SceneManager::GetActor(const std::string& actorName) {
    const auto& it = actorMap.find(actorName);
    luabridge::LuaRef nil(luaState);

    if (it == std::end(actorMap)) {
        return nil;
    }
        
    // Returns actor ptr as a LuaRef
    std::vector<std::shared_ptr<Actor>>& actors = it->second;

    if (actors.size() == 0)
        return nil;

    // Don't return an actor that will be removed
    if (actorsToRemove.find(actors[0].get()) != std::end(actorsToRemove))
        return nil;

    luabridge::push(luaState, actors[0].get());
    luabridge::LuaRef actorRef = luabridge::LuaRef::fromStack(luaState, -1);
    lua_pop(luaState, -1);

    return actorRef;
}

luabridge::LuaRef SceneManager::GetAllActors(const std::string& actorName) {
    luabridge::LuaRef resultTable = luabridge::newTable(luaState);
    luabridge::LuaRef nil(luaState);

    const auto& it = actorMap.find(actorName);

    if (it == std::end(actorMap)) {
        return resultTable; // Empty table
    }

    std::vector<std::shared_ptr<Actor>>& actors = it->second;

    int i = 1;
    for (std::shared_ptr<Actor> actorPtr : actors) {
        if (actorPtr) {
            // Don't return an actor that will be removed
            if (actorsToRemove.find(actorPtr.get()) != std::end(actorsToRemove))
                return nil;

            luabridge::push(luaState, actorPtr.get());
            luabridge::LuaRef actorRef = luabridge::LuaRef::fromStack(luaState, -1);
            lua_pop(luaState, -1);
            resultTable[i] = actorRef;

        }
        else {
            resultTable[i] = nil;
        }
        i++;
    }

    return resultTable;
}

void SceneManager::UpdateAllActorComponents() {
    for (std::shared_ptr<Actor> actor : actorVector) {
        actor->ProcessComponentQueues();
    }
}

Actor* SceneManager::InstantiateActor(const std::string& templateName) {
    std::shared_ptr<Actor> actor = std::make_shared<Actor>();;
    std::shared_ptr<Actor> actorTemplate = TemplateManager::LoadTemplate(templateName);
    *actor = *actorTemplate;

    actor->id = totalActors;
    totalActors++;;

    actorsToAdd.push_back(actor);
    actorMap[actor->name].push_back(actor); // It's ok to add it to the actor map

    return actor.get();
}

void SceneManager::ProcessActorQueues() {
    for (std::shared_ptr<Actor> actor : actorsToAdd) {
        // Add actor to data structures
        actorVector.push_back(actor);

        for (std::pair<std::string, luabridge::LuaRef> componentPair : actor->components) {
            // Set up actor's components
            luabridge::LuaRef component = componentPair.second;
            actor->InjectConvenienceReference(component);
            std::string componentKey = componentPair.first;
            actor->AddComponentLifecycle(component, componentKey);
        }
    }
    actorsToAdd.clear();

    if (actorsToRemove.empty())
        return;

    int numActorsLeft = actorsToRemove.size();

    for (auto it = std::begin(actorVector); it != std::end(actorVector);) {
        Actor* actorPtr = it->get();
        if (actorsToRemove.find(actorPtr) != std::end(actorsToRemove)) {
            // Remove from actorVector
            it = actorVector.erase(it);
            auto& vec = actorMap[actorPtr->GetName()];
            for (auto it2 = std::begin(vec); it2 != std::end(vec);) {
                Actor* actorPtr2 = it2->get();
                if (actorPtr2 == actorPtr) {
                    vec.erase(it2);
                    numActorsLeft--;
                    if (numActorsLeft <= 0)
                        break;
                }
                else {
                    ++it2;
                }
            }
            if (numActorsLeft <= 0)
                break;
        }
        else {
            ++it; // Only increment if no erase was performed
        }
    }

    actorsToRemove.clear();
}

void SceneManager::DestroyActor(Actor* actor) {
    actorsToRemove.insert(actor);
    actor->SetupForDestruction();
    actor->enabled = false;
}

std::string SceneManager::GetCurrentSceneName() {
    return sceneName;
}

void SceneManager::DontDestroy(Actor* actor) {
    std::vector<std::shared_ptr<Actor>> vector = actorMap[actor->name];

    for (std::shared_ptr<Actor> a : vector) {
        if (a.get() == actor)
            dontDestroyOnLoadActors.push_back(a);
    }
}

void SceneManager::LoadSceneRuntime(const std::string& sceneName) {
    for (std::shared_ptr<Actor> actor : actorVector)
    {
        actorsToRemove.insert(actor.get());
    }

    loadingNewScene = true;
    nextSceneName = sceneName;
}

bool SceneManager::IsActorFlaggedForRemoval(Actor* actor) {
    const auto& it = actorsToRemove.find(actor);

    if (it == std::end(actorsToRemove))
        return false;

    return true;
}

void SceneManager::SetLuaState(lua_State* L) {
    luaState = L;
}
