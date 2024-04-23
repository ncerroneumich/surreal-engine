// Camera2D.cpp
#include "Camera2D.h"

void Camera2D::Initialize(int x, int y) {
    zoomFactor = 1.0f;
    position = { 0.0f, 0.0f };
    size = { x, y };
    easeFactor = 0;
}

glm::vec2 Camera2D::GetPosition() {
    return position;
}

float Camera2D::GetPositionX() {
    return position.x;
}

float Camera2D::GetPositionY() {
    return position.y;
}

void Camera2D::SetPosition(float x, float y) {
    targetPosition = { x, y };
}

glm::ivec2 Camera2D::GetSize() {
    return size;
}

void Camera2D::SetZoomFactor(float z) {
    zoomFactor = z;
}

float Camera2D::GetZoomFactor() {
    return zoomFactor;
}

glm::vec2 Camera2D::GetRelativePosition(const glm::vec2& worldPosition) {
    return glm::vec2(worldPosition.x - GetPosition().x,
                      worldPosition.y - GetPosition().y);
}

glm::vec2 Camera2D::GetScreenPosition(const glm::vec2& worldPosition) {
    glm::vec2 relativePosition = GetRelativePosition(worldPosition);
    glm::vec2 unscaledPosition = relativePosition * 100.0f + static_cast<glm::vec2>(size / 2);
    zoomOffset = (static_cast<glm::vec2>(size) * (1.0f - 1.0f / zoomFactor)) / 2.0f;
    return unscaledPosition - zoomOffset;
}

glm::vec2 Camera2D::GetScreenPostionUnmodified(const glm::vec2& worldPosition) {
    glm::vec2 unscaledPosition = worldPosition * 100.0f + static_cast<glm::vec2>(size / 2);
    return unscaledPosition;
}

void Camera2D::SetEaseFactor(float ease) {
    easeFactor = ease;
}

float Camera2D::GetEaseFactor() {
    return easeFactor;
}

void Camera2D::UpdateCameraPosition(float deltaTime) {
    position = glm::mix(position, targetPosition, glm::clamp(easeFactor * deltaTime, 0.0f, 1.0f));
}