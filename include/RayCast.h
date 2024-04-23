#pragma once

#include "box2d/box2d.h"
#include "lua/lua.hpp"
#include "LuaBridge/LuaBridge.h"
#include <memory>

class Actor;

struct HitResult {
	Actor* actor;
	b2Vec2 point;
	b2Vec2 normal;
	bool is_trigger;
};

class ClosestHitCallback : public b2RayCastCallback {
public:
	ClosestHitCallback() : fixture(nullptr), closestFraction(1) {}

	float ReportFixture(b2Fixture* _fixture, const b2Vec2& _point, const b2Vec2& _normal, float fraction) override {
		if (_fixture->GetFilterData().maskBits == 0)
			return closestFraction;

		if (fraction < closestFraction) {
			fixture = _fixture;
			point = _point;
			normal = _normal;
			closestFraction = fraction;
		}

		return closestFraction;
	}
	
	float closestFraction;
	b2Fixture* fixture;
	b2Vec2 point;
	b2Vec2 normal;
};

struct RayHit {
	b2Fixture* fixture;
	b2Vec2 point;
	b2Vec2 normal;
	float fraction;

	RayHit(b2Fixture* fix, const b2Vec2& pt, const b2Vec2& norm, float frac)
		: fixture(fix), point(pt), normal(norm), fraction(frac) {}
};

class AllHitsCallback : public b2RayCastCallback {
public:
	float ReportFixture(b2Fixture* _fixture, const b2Vec2& _point, const b2Vec2& _normal, float fraction) override {
		if (_fixture->GetFilterData().maskBits == 0)
			return 1;
		hits.emplace_back(_fixture, _point, _normal, fraction);
		return 1; // Get all hits
	}
	std::vector<RayHit> hits;
};

class RayCast {
public:
	static void SetLuaState(lua_State* L) {
		luaState = L;
	}

	static void SetPhysicsWorld(std::shared_ptr<b2World> w) {
		world = w;
	}

	static luabridge::LuaRef Raycast(b2Vec2 startPosition, b2Vec2 direction, float distance) {
		ClosestHitCallback callback;

		b2Vec2 endPosition = startPosition + distance * direction;

		world->RayCast(&callback, startPosition, endPosition);

		// No hit detected
		if (callback.closestFraction == 1) {
			return luabridge::LuaRef(luaState);
		}

		HitResult result;
		result.actor = reinterpret_cast<Actor*>(callback.fixture->GetUserData().pointer);
		result.point = callback.point;
		result.normal = callback.normal;
		result.is_trigger = callback.fixture->IsSensor();

		luabridge::push(luaState, result);
		luabridge::LuaRef resultRef = luabridge::LuaRef::fromStack(luaState, -1);
		lua_pop(luaState, -1);

		return resultRef;
	}

	static luabridge::LuaRef RaycastAll(b2Vec2 startPosition, b2Vec2 direction, float distance) {
		AllHitsCallback callback;

		b2Vec2 endPosition = startPosition + distance * direction;

		world->RayCast(&callback, startPosition, endPosition);

		luabridge::LuaRef results = luabridge::newTable(luaState);

		std::sort(callback.hits.begin(), callback.hits.end(), [](const RayHit& a, const RayHit& b) {
			return a.fraction < b.fraction;
		});

		for (int i = 0; i < callback.hits.size(); i++) {
			const RayHit& hit = callback.hits[i];
			HitResult result;
			result.actor = reinterpret_cast<Actor*>(hit.fixture->GetUserData().pointer);
			result.point = hit.point;
			result.normal = hit.normal;
			result.is_trigger = hit.fixture->IsSensor();

			luabridge::push(luaState, result);
			luabridge::LuaRef resultRef = luabridge::LuaRef::fromStack(luaState, -1);
			lua_pop(luaState, -1);

			results[i + 1] = resultRef;
		}

		return results;
	}

private:
	static inline lua_State* luaState;
	static inline std::shared_ptr<b2World> world;
};


