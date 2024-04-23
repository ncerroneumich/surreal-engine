// Camera2D.h
#pragma once

#include "glm/glm.hpp"
#include <string>
#include <vector>

class Camera2D {
public:
    static void Initialize(int x, int y);
    static glm::vec2 GetPosition();
    static float GetPositionX();
    static float GetPositionY();
    static void SetPosition(float x, float y);
    static glm::ivec2 GetSize();

    static glm::vec2 GetRelativePosition(const glm::vec2& worldPosition);
    static glm::vec2 GetScreenPosition(const glm::vec2& worldPosition);
    static glm::vec2 GetScreenPostionUnmodified(const glm::vec2& worldPosition);

    static void SetZoomFactor(float z);
    static float GetZoomFactor();

    static void SetEaseFactor(float ease);
    static float GetEaseFactor();

    static void UpdateCameraPosition(float deltaTime);

    static inline float zoomFactor = 1.0;
    static inline glm::vec2 position = { 0, 0 };
    static inline glm::vec2 targetPosition = { 0, 0 }; // Target position for easing
    static inline glm::ivec2 size;
    static inline glm::vec2 zoomOffset;
    static inline float easeFactor;
};
