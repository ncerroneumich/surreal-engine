#include "TemplateManager.h"

std::shared_ptr<Actor> TemplateManager::LoadTemplate(const std::string& templateName) {
    std::string templatePath = "resources/actor_templates/" + templateName + ".template";

    if (!std::filesystem::exists(templatePath)) {
        std::cout << "error: template " << templateName << " is missing";
        exit(0);
    }

    // If the template already exists, return it
    if (templateMap.find(templateName) != std::end(templateMap))
        return templateMap[templateName];

    rapidjson::Document templateDocument;

    ReadJsonFile(templatePath, templateDocument);
    templateMap[templateName] = ParseTemplate(templateDocument);
    return templateMap[templateName];
}

std::shared_ptr<Actor> TemplateManager::ParseTemplate(const rapidjson::Document& templateDocument) {
    std::shared_ptr<Actor> actor = std::make_shared<Actor>();

    if (templateDocument.HasMember("name")) {
        actor->name = templateDocument["name"].GetString();
    }

    if (templateDocument.HasMember("components")) {
        const rapidjson::GenericObject componentsObject = templateDocument["components"].GetObject();

        // Iterate through all components in this actor
        for (rapidjson::Value::ConstMemberIterator componentsItr = componentsObject.MemberBegin(); componentsItr != componentsObject.MemberEnd(); componentsItr++) {
            std::string componentKey = componentsItr->name.GetString();

            // Handle component type declaration
            std::string componentFileName = componentsItr->value["type"].GetString();
            luabridge::LuaRef component = ComponentManager::LoadComponent(componentKey, componentFileName);

            actor->components.insert(std::pair(componentKey, component));
            actor->componentsByType[component["type"].tostring()].insert(componentKey);

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
        }
    }

    return actor;
}