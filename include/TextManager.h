// TextManager.h
#pragma once

#include "SDL2/SDL.h"
#include "SDL_ttf/SDL_ttf.h"
#include <string>
#include <unordered_map>
#include <queue>

struct TextDrawRequest {
	TextDrawRequest(const std::string& _content, float _x, float _y, const std::string& _fontName, float _fontSize, float _r, float _g, float _b, float _a)
		: content(_content), x(_x), y(_y), fontName(_fontName), fontSize(static_cast<int>(_fontSize)), r(static_cast<int>(_r)), g(static_cast<int>(_g)), b(static_cast<int>(_b)), a(static_cast<int>(_a)) {}

	std::string content;
	float x;
	float y;
	std::string fontName;
	int fontSize;
	int r;
	int g;
	int b;
	int a;
};

class TextManager
{
public:
	static void Initialize(SDL_Renderer* renderer);
	static void DrawRequestQueue();
	static void SubmitTextDrawRequest(const std::string& content, float x, float y, const std::string& fontName, int fontSize, float r, float g, float b, float a);
private:
	static inline std::unordered_map<std::string, std::unordered_map<int, TTF_Font*>> fonts;
	static inline std::string fontFolderPath = "resources/fonts/";
	static inline std::queue<TextDrawRequest> textDrawRequestQueue;
	static inline SDL_Renderer* renderer;

	TextManager();
	static TTF_Font* LoadFont(const std::string& fontName, int fontSize);
	static void DrawText(const TextDrawRequest& request);
};

