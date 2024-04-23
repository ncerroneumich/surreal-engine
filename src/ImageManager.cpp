#include "ImageManager.h"
#include <iostream>
#include <filesystem>
#include "Camera2D.h"
#include <vector>

void ImageManager::Initialize(SDL_Renderer* _renderer) {
    IMG_Init(IMG_INIT_PNG);
    renderer = _renderer;
}

SDL_Texture* ImageManager::LoadImage(const std::string& imageName) {
    std::string filePath = imageFolderPath + imageName + ".png";
    // Check if the image is already loaded
    auto it = textures.find(imageName);
    if (it != textures.end()) {
        // Image is already loaded, return the existing texture
        return it->second;
    }

    // Load the image as a texture
    SDL_Texture* texture = IMG_LoadTexture(renderer, filePath.c_str());

    if (texture == nullptr) {
        std::cout << "error: missing image " << imageName;
        exit(0);
    }

    // Store the texture in the map and return it
    textures[imageName] = texture;
    return texture;
}

void ImageManager::SubmitUIDrawRequest(const std::string& imageName, float x, float y) {
    ScreenSpaceDrawRequest request(imageName, x, y);
    UIRequestQueue.push_back(request);
}

void ImageManager::SubmitUIExDrawRequest(const std::string& imageName, float x, float y, float r, float g, float b, float a, float sortingOrder, float rotationDegrees, float scaleX, float scaleY, float pivotX, float pivotY) {
    ScreenSpaceDrawRequest request(imageName, x, y, rotationDegrees, scaleX, scaleY, pivotX, pivotY, r, g, b, a, sortingOrder);
    UIRequestQueue.push_back(request);
}


void ImageManager::SubmitScreenSpaceDrawRequest(const std::string& imageName, float x, float y) {
    ScreenSpaceDrawRequest request(imageName, x, y);
    screenSpaceRequestQueue.push_back(request);
}

void ImageManager::SubmitScreenSpaceExDrawRequest(const std::string& imageName, float x, float y, float rotationDegrees, float scaleX, float scaleY, float pivotX, float pivotY, float r, float g, float b, float a, float sortingOrder) {
    ScreenSpaceDrawRequest request(imageName, x, y, rotationDegrees, scaleX, scaleY, pivotX, pivotY, r, g, b, a, sortingOrder);
    screenSpaceRequestQueue.push_back(request);
}

void ImageManager::SubmitPixelDrawRequest(float x, float y, float r, float g, float b, float a) {
    PixelDrawRequest request(x, y, r, g, b, a);
    pixelRequestQueue.push_back(request);
}

void ImageManager::DrawUIRequestQueue() {
    if (UIRequestQueue.empty())
        return;

    std::stable_sort(UIRequestQueue.begin(), UIRequestQueue.end(), [](const ScreenSpaceDrawRequest& a, const ScreenSpaceDrawRequest& b) {
        return a.sortingOrder < b.sortingOrder;
    });
    
    for (const ScreenSpaceDrawRequest& request : UIRequestQueue) {
        SDL_Texture* texture = LoadImage(request.imageName);
        DrawScreenSpace(texture, request, false);
    }

    UIRequestQueue.clear();
}

void ImageManager::DrawScreenSpaceRequestQueue() {
    if (screenSpaceRequestQueue.empty())
        return;

    std::stable_sort(screenSpaceRequestQueue.begin(), screenSpaceRequestQueue.end(), [](const ScreenSpaceDrawRequest& a, const ScreenSpaceDrawRequest& b) {
        return a.sortingOrder < b.sortingOrder;
    });

    SDL_RenderSetScale(renderer, Camera2D::zoomFactor, Camera2D::zoomFactor);

    for (const ScreenSpaceDrawRequest& request : screenSpaceRequestQueue) {
        SDL_Texture* texture = LoadImage(request.imageName);
        DrawScreenSpace(texture, request);
    }

    SDL_RenderSetScale(renderer, 1, 1);

    screenSpaceRequestQueue.clear();
}

void ImageManager::DrawPixelRequestQueue() {
    if (pixelRequestQueue.empty())
        return;

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    for (const PixelDrawRequest& request : pixelRequestQueue) {
        SDL_SetRenderDrawColor(renderer, request.r, request.g, request.b, request.a);
        SDL_RenderDrawPoint(renderer, request.x, request. y);
    }
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

    pixelRequestQueue.clear();
}

void ImageManager::DrawScreenSpace(SDL_Texture* texture, const ScreenSpaceDrawRequest& request, bool moveWithCamera) {
    int textureWidth, textureHeight;
    SDL_QueryTexture(texture, nullptr, nullptr, &textureWidth, &textureHeight);
    
    SDL_Point pivot;
    pivot.x = static_cast<int>(request.pivotX * textureWidth * request.scaleX);
    pivot.y = static_cast<int>(request.pivotY * textureHeight * request.scaleY);

    SDL_Rect destRect;

    destRect.w = textureWidth * request.scaleX;
    destRect.h = textureHeight * request.scaleY;


    glm::vec2 position;

    if (moveWithCamera)
        position = Camera2D::GetScreenPosition(glm::vec2(request.x, request.y));
    else
        position = Camera2D::GetScreenPostionUnmodified(glm::vec2(request.x, request.y));

    destRect.x = static_cast<int>(position.x - pivot.x);
    destRect.y = static_cast<int>(position.y - pivot.y);

    bool horizontalFlip = request.scaleX > 0 ? false : true;
    bool verticalFlip = request.scaleY > 0 ? false : true;

    SDL_RendererFlip flip = GetRendererFlip(horizontalFlip, verticalFlip);

    SDL_SetTextureColorMod(texture, request.r, request.g, request.b);
    SDL_SetTextureAlphaMod(texture, request.a);

    SDL_RenderCopyEx(
        renderer,
        texture,
        NULL, // Source rect
        &destRect,
        request.rotationDegrees,
        &pivot,
        flip
    );

    SDL_SetTextureColorMod(texture, 255, 255, 255);
    SDL_SetTextureAlphaMod(texture, 255);
}

SDL_RendererFlip ImageManager::GetRendererFlip(bool horizontalFlip, bool verticalFlip) {
    int flip = 0;
    if (horizontalFlip)
        flip |= SDL_FLIP_HORIZONTAL;
    if (verticalFlip)
        flip |= SDL_FLIP_VERTICAL;
    return static_cast<SDL_RendererFlip>(flip);
}