#pragma once

#include <SDL2/SDL.h>
#include <SDL_image/SDL_image.h>
#include <string>
#include <unordered_map>
#include <vector>
#include <queue>


struct ScreenSpaceDrawRequest {
	ScreenSpaceDrawRequest(const std::string& _imageName, float _x, float _y, float _rotationDegrees, float _scaleX, float _scaleY, float _pivotX, float _pivotY, float _r, float _g, float _b, float _a, float _sortingOrder)
		: imageName(_imageName), x(_x), y(_y), scaleX(_scaleX), scaleY(_scaleY), pivotX(_pivotX), pivotY(_pivotY), rotationDegrees(static_cast<int>(_rotationDegrees)), r(static_cast<int>(_r)), g(static_cast<int>(_g)), b(static_cast<int>(_b)), a(static_cast<int>(_a)), sortingOrder(static_cast<int>(_sortingOrder)) {}

	ScreenSpaceDrawRequest(const std::string& _imageName, float _x, float _y)
		: imageName(_imageName), x(_x), y(_y), scaleX(1), scaleY(1), pivotX(0.5), pivotY(0.5), rotationDegrees(0), r(255), g(255), b(255), a(255), sortingOrder(0) {}

	std::string imageName;
	float x;
	float y;
	float scaleX;
	float scaleY;
	float pivotX;
	float pivotY;
	int rotationDegrees;
	int r;
	int g;
	int b;
	int a;
	int sortingOrder;
};

struct PixelDrawRequest {
	PixelDrawRequest(float _x, float _y, float _r, float _g, float _b, float _a)
		: x(static_cast<int>(_x)), y(static_cast<int>(_y)), r(static_cast<int>(_r)), g(static_cast<int>(_g)), b(static_cast<int>(_b)), a(static_cast<int>(_a)) {}

	int x;
	int y;
	int r;
	int g;
	int b;
	int a;
};

class ImageManager {
public:
    static void Initialize(SDL_Renderer* _renderer);

	static void SubmitUIDrawRequest(const std::string& imageName, float x, float y);
	static void SubmitUIExDrawRequest(const std::string& imageName, float x, float y, float r, float g, float b, float a, float sortingOrder, float rotationDegrees = 0.0f, float scaleX = 1.0f, float scaleY = 1.0f, float pivotX = 0.5f, float pivotY = 0.5f);
	static void SubmitScreenSpaceDrawRequest(const std::string& imageName, float x, float y);
	static void SubmitScreenSpaceExDrawRequest(const std::string& imageName, float x, float y, float rotationDegrees, float scaleX, float scaleY, float pivotX, float pivotY, float r, float g, float b, float a, float sortingOrder);
	static void SubmitPixelDrawRequest(float x, float y, float r, float g, float b, float a);

	static void DrawUIRequestQueue();
	static void DrawScreenSpaceRequestQueue();
	static void DrawPixelRequestQueue();

private:
    ImageManager();
	static SDL_Texture* LoadImage(const std::string& imageName);
	static SDL_RendererFlip GetRendererFlip(bool horizontalFlip, bool verticalFlip);
	static void DrawScreenSpace(SDL_Texture* texture, const ScreenSpaceDrawRequest& request, bool moveWithCamera = true);

    static inline SDL_Renderer* renderer; // SDL_Renderer reference
    static inline std::unordered_map<std::string, SDL_Texture*> textures; // Map to store textures
    static inline std::string imageFolderPath = "resources/images/";
	static inline std::vector<ScreenSpaceDrawRequest> UIRequestQueue;
	static inline std::vector<ScreenSpaceDrawRequest> screenSpaceRequestQueue;
	static inline std::vector<PixelDrawRequest> pixelRequestQueue;
};