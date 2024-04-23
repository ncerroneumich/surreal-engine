// TemplateManager.h
#pragma once

#include <unordered_map>
#include <string>
#include <filesystem>
#include <iostream>
#include "rapidjson/document.h"
#include "ReadJsonFile.h"
#include "Actor.h"
#include <memory>
#include "lua/lua.hpp"
#include "LuaBridge/LuaBridge.h"


class TemplateManager {
public:
    /// <summary>
    /// Returns an actor object with all the appropriate template variables set.
    /// </summary>
    /// <returns></returns>
    static std::shared_ptr<Actor> LoadTemplate(const std::string& templateName);

private:
    static inline std::unordered_map<std::string, std::shared_ptr<Actor>> templateMap;
    static std::shared_ptr<Actor> ParseTemplate(const rapidjson::Document& templateDocument);
};