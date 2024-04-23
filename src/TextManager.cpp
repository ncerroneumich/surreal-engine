// TextManager.cpp

#include "TextManager.h"
#include <iostream>
#include <filesystem>
#include "glm/glm.hpp"
#include "Camera2D.h"

void TextManager::Initialize(SDL_Renderer* r) {
    TTF_Init();
    renderer = r;
}

// Returns the requested font. If this is the first call for this font, it will also load the font.
TTF_Font* TextManager::LoadFont(const std::string& fontName, int fontSize) {
    std::string filePath = fontFolderPath + fontName + ".ttf";

    // Check if the font is already loaded
    auto it = fonts.find(fontName);
    if (it != std::end(fonts)) {
        std::unordered_map<int, TTF_Font*>& fontMap = it->second;
        auto it = fontMap.find(fontSize);
        if (it != std::end(fontMap)) {
            return it->second;
        }
    }

    TTF_Font* font = TTF_OpenFont(filePath.c_str(), fontSize); // 16 is font size

    if (font == nullptr) {
        std::cout << "error: font " << fontName << " missing";
        exit(0);
    }

    fonts[fontName][fontSize] = font;
    return font;
}

void TextManager::SubmitTextDrawRequest(const std::string& content, float x, float y, const std::string& fontName, int fontSize, float r, float g, float b, float a) {
    TextDrawRequest request(content, x, y, fontName, fontSize, r, g, b, a);
    textDrawRequestQueue.push(request);
}

void TextManager::DrawRequestQueue() {
    while (!textDrawRequestQueue.empty()) {
        const TextDrawRequest& request = textDrawRequestQueue.front();
        DrawText(request);
        textDrawRequestQueue.pop();
    }
}

void TextManager::DrawText(const TextDrawRequest& request) {
    TTF_Font* font = LoadFont(request.fontName, request.fontSize);
    SDL_Color color = { static_cast<Uint8>(request.r), static_cast<Uint8>(request.g), static_cast<Uint8>(request.b), static_cast<Uint8>(request.a) };
    SDL_Surface* textSurface = TTF_RenderText_Solid(font, request.content.c_str(), color);
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);

    SDL_Point pivot;
    pivot.x = static_cast<int>(0.5 * textSurface->w);
    pivot.y = static_cast<int>(0.5 * textSurface->h);

    glm::vec2 position = Camera2D::GetScreenPostionUnmodified(glm::vec2(request.x, request.y));

    SDL_Rect renderQuad;

    renderQuad.x = static_cast<int>(position.x - pivot.x);
    renderQuad.y = static_cast<int>(position.y - pivot.y);
    renderQuad.w = textSurface->w;
    renderQuad.h = textSurface->h;

    SDL_RenderCopyEx(renderer, textTexture, NULL, &renderQuad, 0.0, NULL, SDL_FLIP_NONE);
    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);
}

