#pragma once

#include "box2d/box2d.h"
#include "Actor.h"

struct Collision {
	Actor* other;
	b2Vec2 point;
	b2Vec2 relative_velocity;
	b2Vec2 normal;
};

class ContactListener : public b2ContactListener
{
	void BeginContact(b2Contact* contact) override;
	void EndContact(b2Contact* contact) override;
};
