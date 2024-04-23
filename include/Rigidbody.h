#pragma once

class Actor;

#include <string>
#include <memory>
#include "box2d/box2d.h"
#include "glm/glm.hpp"


enum Category {
    TRIGGERS = 0x0002,
    COLLIDERS = 0x0004
};

class Rigidbody
{
public:
    b2Body* body;
    b2BodyDef bodyDef;

    std::string bodyTypeString = "dynamic";
    std::string colliderType = "box";
    std::string triggerType = "box";
    float width = 1.0f;
    float height = 1.0f;
    float radius = 0.5f;
    float triggerWidth = 1.0f;
    float triggerHeight = 1.0f;
    float triggerRadius = 0.5f;
    float friction = 0.3f;
    float bounciness = 0.3f;
    float x = 0.0f, y = 0.0f;
    bool precise = true;
    float gravityScale = 1.0f;
    float density = 1.0f;
    float angularFriction = 0.3f;
    float rotationDegrees = 0.0f;
    bool hasCollider = true;
    bool hasTrigger = true;


    std::string type = "Rigidbody";
    std::string key;
    bool enabled = true;
    Actor* actor;

    void AddForce(b2Vec2 force) {
        body->ApplyForceToCenter(force, true);
    }

    void SetVelocity(b2Vec2 velocity) {
        if (body == nullptr) {
            return;
        }
        body->SetLinearVelocity(velocity);
    }

    b2Vec2 GetVelocity() {
        if (body == nullptr) {
            return b2Vec2(0.0f, 0.0f);
        }
        return body->GetLinearVelocity();
    }

    void SetPosition(b2Vec2 position) {
        if (body == nullptr) {
            x = position.x;
            y = position.y;
            return;
        }
        body->SetTransform(position, body->GetAngle());
    }

    b2Vec2 GetPosition() {
        if (body == NULL) {
            return b2Vec2(x, y);
        }

        return body->GetPosition();
    }

    void SetRotation(float degreesClockwise) {
        if (body == nullptr) {
            rotationDegrees = degreesClockwise;
            return;
        }
        float radians = degreesClockwise * (b2_pi / 180.0f);
        body->SetTransform(body->GetPosition(), radians);
    }

    float GetRotation() {
        if (body == nullptr) {
            return rotationDegrees;
        }

        float degrees = body->GetAngle() * (180.0f / b2_pi);
        return degrees;
    }

    void SetAngularVelocity(float degreesClockwise) {
        if (body == nullptr) {
            return;
        }

        float radians = degreesClockwise * (b2_pi / 180.0f);
        body->SetAngularVelocity(radians);
    }

    float GetAngularVelocity() {
        if (body == nullptr) {
            return 0.0f;
        }

        float degrees = body->GetAngularVelocity() * (180.0f / b2_pi);
        return degrees;
    }

    void SetGravityScale(float scale) {
        if (body == nullptr) {
            gravityScale = scale;
            return;
        }

        body->SetGravityScale(scale);
    }

    float GetGravityScale() {
        if (body == nullptr) {
            return gravityScale;
        }

        return body->GetGravityScale();
    }

    void SetUpDirection(b2Vec2 direction) {
        if (body == nullptr)
            return;

        direction.Normalize();
        float angle = glm::atan(direction.x, -direction.y);
        body->SetTransform(body->GetPosition(), angle);
    }

    b2Vec2 GetUpDirection() {
        if (body == nullptr)
            return b2Vec2(0, -1);

        float angleRadians = body->GetAngle();
        return b2Vec2(glm::sin(angleRadians), -glm::cos(angleRadians));
    }

    void SetRightDirection(b2Vec2 direction) {
        if (body == nullptr)
            return;

        direction.Normalize();
        float angle = glm::atan(direction.x, -direction.y) - (b2_pi / 2.0f);
        body->SetTransform(body->GetPosition(), angle);
    }

    b2Vec2 GetRightDirection() {
        if (body == nullptr)
            return b2Vec2(1, 0);

        float angle = body->GetAngle();
        return b2Vec2(glm::cos(angle), glm::sin(angle));
    }

    void OnStart() {
        bodyDef.position.Set(x, y);
        bodyDef.bullet = precise;
        bodyDef.gravityScale = gravityScale;
        bodyDef.angularDamping = angularFriction;
        bodyDef.angle = rotationDegrees * (b2_pi / 180.0f);

        if (bodyTypeString == "dynamic") {
            bodyDef.type = b2_dynamicBody;
        }
        else if (bodyTypeString == "static") {
            bodyDef.type = b2_staticBody;
        }
        else if (bodyTypeString == "kinematic") {
            bodyDef.type = b2_kinematicBody;
        }

        body = world->CreateBody(&bodyDef);

        // Handle Collider
        if (hasCollider) {
            b2FixtureDef colliderFixture;
            colliderFixture.userData.pointer = reinterpret_cast<uintptr_t>(actor);
            colliderFixture.isSensor = false;
            colliderFixture.density = density;
            colliderFixture.filter.categoryBits = COLLIDERS;
            colliderFixture.filter.maskBits = COLLIDERS;
            colliderFixture.friction = friction;
            colliderFixture.restitution = bounciness;

            if (colliderType == "box") {
                b2PolygonShape boxShape;
                boxShape.SetAsBox(width * 0.5f, height * 0.5f);
                colliderFixture.shape = &boxShape;
                body->CreateFixture(&colliderFixture);
            }
            else if (colliderType == "circle") {
                b2CircleShape circleShape;
                circleShape.m_radius = radius;
                colliderFixture.shape = &circleShape;
                body->CreateFixture(&colliderFixture);
            }
        }

        // Handle Trigger
        if (hasTrigger) {
            b2FixtureDef triggerFixture;
            triggerFixture.userData.pointer = reinterpret_cast<uintptr_t>(actor);
            triggerFixture.isSensor = true;
            triggerFixture.density = density;
            triggerFixture.filter.categoryBits = TRIGGERS;
            triggerFixture.filter.maskBits = TRIGGERS;
            triggerFixture.friction = friction;
            triggerFixture.restitution = bounciness;

            if (triggerType == "box") {
                b2PolygonShape boxShape;
                boxShape.SetAsBox(triggerWidth * 0.5f, triggerHeight * 0.5f);
                triggerFixture.shape = &boxShape;
                body->CreateFixture(&triggerFixture);
            }
            else if (triggerType == "circle") {
                b2CircleShape circleShape;
                circleShape.m_radius = triggerRadius;
                triggerFixture.shape = &circleShape;
                body->CreateFixture(&triggerFixture);
            }
        }

        // Handle Phantom
        if (!hasCollider && !hasTrigger) {
            b2PolygonShape phantomShape;
            phantomShape.SetAsBox(width * 0.5f, height * 0.5f);

            b2FixtureDef phantomFixture;
            phantomFixture.shape = &phantomShape;
            phantomFixture.density = density;
            phantomFixture.userData.pointer = reinterpret_cast<uintptr_t>(actor);
            phantomFixture.isSensor = true;
            phantomFixture.filter.categoryBits = 0;
            phantomFixture.filter.maskBits = 0; // Nothing will interact with them

            body->CreateFixture(&phantomFixture);
        }
    }


    void OnDestroy() {
        world->DestroyBody(body);
    }

    std::shared_ptr<Rigidbody> Clone(Actor* actor) const {
        // Create a new Rigidbody instance on the heap
        auto clone = std::make_shared<Rigidbody>();

        // Manually copy fields from the current object to the new instance
        clone->bodyTypeString = this->bodyTypeString;
        clone->colliderType = this->colliderType;
        clone->triggerType = this->triggerType;
        clone->width = this->width;
        clone->height = this->height;
        clone->radius = this->radius;
        clone->triggerWidth = this->triggerWidth;
        clone->triggerHeight = this->triggerHeight;
        clone->triggerRadius = this->triggerRadius;
        clone->friction = this->friction;
        clone->bounciness = this->bounciness;
        clone->x = this->x;
        clone->y = this->y;
        clone->precise = this->precise;
        clone->gravityScale = this->gravityScale;
        clone->density = this->density;
        clone->angularFriction = this->angularFriction;
        clone->rotationDegrees = this->rotationDegrees;
        clone->hasCollider = this->hasCollider;
        clone->hasTrigger = this->hasTrigger;
        clone->type = this->type;
        clone->key = this->key;
        clone->enabled = this->enabled;

        clone->actor = actor;

        clone->bodyDef.position.Set(clone->x, clone->y);
        clone->bodyDef.bullet = clone->precise;
        clone->bodyDef.gravityScale = clone->gravityScale;
        clone->bodyDef.angularDamping = clone->angularFriction;
        clone->bodyDef.angle = clone->rotationDegrees * (b2_pi / 180.0f);

        return clone;
    }

    static void SetWorld(std::shared_ptr<b2World> w) {
        world = w;
    }

private:
    static inline std::shared_ptr<b2World> world;
};
