#include "ContactListener.h"


void ContactListener::BeginContact(b2Contact* contact) {
	b2Fixture* fixtureA = contact->GetFixtureA();
	b2Fixture* fixtureB = contact->GetFixtureB();

	Actor* actorA = reinterpret_cast<Actor*>(fixtureA->GetUserData().pointer);
	Actor* actorB = reinterpret_cast<Actor*>(fixtureB->GetUserData().pointer);

	b2WorldManifold manifold;
	contact->GetWorldManifold(&manifold);

	Collision collision;

	collision.relative_velocity = fixtureA->GetBody()->GetLinearVelocity() - fixtureB->GetBody()->GetLinearVelocity();
	// Collisions
	if (!fixtureA->IsSensor() && !fixtureB->IsSensor()) {
		std::map<std::string, luabridge::LuaRef>& onCollisionEnterComponents = actorA->onCollisionEnterComponents;
		collision.other = actorB;
		collision.point = manifold.points[0];
		collision.normal = manifold.normal;
		for (auto& pair : onCollisionEnterComponents) {
			// If the actor gets disabled, don't finish running its components
			if (actorA->enabled == false)
				break;

			luabridge::LuaRef component = pair.second;

			if (component["enabled"] == false)
				continue;
			try {
				component["OnCollisionEnter"](component, collision);
			}
			catch (luabridge::LuaException e) {
				std::string errorMessage = e.what();
				std::replace(std::begin(errorMessage), std::end(errorMessage), '\\', '/');
				std::cout << "\033[31m" << actorA->GetName() << " : " << errorMessage << "\033[0m" << std::endl;
			}
		}

		onCollisionEnterComponents = actorB->onCollisionEnterComponents;
		collision.other = actorA;
		for (auto& pair : onCollisionEnterComponents) {
			// If the actor gets disabled, don't finish running its components
			if (actorB->enabled == false)
				break;

			luabridge::LuaRef component = pair.second;

			if (component["enabled"] == false)
				continue;
			try {
				component["OnCollisionEnter"](component, collision);
			}
			catch (luabridge::LuaException e) {
				std::string errorMessage = e.what();
				std::replace(std::begin(errorMessage), std::end(errorMessage), '\\', '/');
				std::cout << "\033[31m" << actorB->GetName() << " : " << errorMessage << "\033[0m" << std::endl;
			}
		}
	}

	// Triggers
	else if (fixtureA->IsSensor() && fixtureB->IsSensor()) {
		collision.point = b2Vec2(-999.0f, -999.0f);
		collision.normal = b2Vec2(-999.0f, -999.0f);
		std::map<std::string, luabridge::LuaRef>& onTriggerEnterComponents = actorA->onTriggerEnterComponents;
		collision.other = actorB;
		for (auto& pair : onTriggerEnterComponents) {
			// If the actor gets disabled, don't finish running its components
			if (actorA->enabled == false)
				break;

			luabridge::LuaRef component = pair.second;

			if (component["enabled"] == false)
				continue;
			try {
				component["OnTriggerEnter"](component, collision);
			}
			catch (luabridge::LuaException e) {
				std::string errorMessage = e.what();
				std::replace(std::begin(errorMessage), std::end(errorMessage), '\\', '/');
				std::cout << "\033[31m" << actorA->GetName() << " : " << errorMessage << "\033[0m" << std::endl;
			}
		}

		onTriggerEnterComponents = actorB->onTriggerEnterComponents;
		collision.other = actorA;
		for (auto& pair : onTriggerEnterComponents) {
			// If the actor gets disabled, don't finish running its components
			if (actorB->enabled == false)
				break;

			luabridge::LuaRef component = pair.second;

			if (component["enabled"] == false)
				continue;
			try {
				component["OnTriggerEnter"](component, collision);
			}
			catch (luabridge::LuaException e) {
				std::string errorMessage = e.what();
				std::replace(std::begin(errorMessage), std::end(errorMessage), '\\', '/');
				std::cout << "\033[31m" << actorB->GetName() << " : " << errorMessage << "\033[0m" << std::endl;
			}
		}
	}
}

void ContactListener::EndContact(b2Contact* contact) {
	b2Fixture* fixtureA = contact->GetFixtureA();
	b2Fixture* fixtureB = contact->GetFixtureB();

	Actor* actorA = reinterpret_cast<Actor*>(fixtureA->GetUserData().pointer);
	Actor* actorB = reinterpret_cast<Actor*>(fixtureB->GetUserData().pointer);

	b2WorldManifold manifold;
	contact->GetWorldManifold(&manifold);

	Collision collision;
	collision.relative_velocity = fixtureA->GetBody()->GetLinearVelocity() - fixtureB->GetBody()->GetLinearVelocity();

	// Collisions
	if (!fixtureA->IsSensor() && !fixtureB->IsSensor()) {
		collision.other = actorB;
		collision.point = b2Vec2(-999.0f, -999.0f);
		collision.normal = b2Vec2(-999.0f, -999.0f);
		std::map<std::string, luabridge::LuaRef>& onCollisionExitComponents = actorA->onCollisionExitComponents;

		for (auto& pair : onCollisionExitComponents) {
			// If the actor gets disabled, don't finish running its components
			if (actorA->enabled == false)
				break;

			luabridge::LuaRef component = pair.second;

			if (component["enabled"] == false)
				continue;
			try {
				component["OnCollisionExit"](component, collision);
			}
			catch (luabridge::LuaException e) {
				std::string errorMessage = e.what();
				std::replace(std::begin(errorMessage), std::end(errorMessage), '\\', '/');
				std::cout << "\033[31m" << actorA->GetName() << " : " << errorMessage << "\033[0m" << std::endl;
			}
		}

		onCollisionExitComponents = actorB->onCollisionExitComponents;
		collision.other = actorA;
		for (auto& pair : onCollisionExitComponents) {
			// If the actor gets disabled, don't finish running its components
			if (actorB->enabled == false)
				break;

			luabridge::LuaRef component = pair.second;

			if (component["enabled"] == false)
				continue;
			try {
				component["OnCollisionExit"](component, collision);
			}
			catch (luabridge::LuaException e) {
				std::string errorMessage = e.what();
				std::replace(std::begin(errorMessage), std::end(errorMessage), '\\', '/');
				std::cout << "\033[31m" << actorB->GetName() << " : " << errorMessage << "\033[0m" << std::endl;
			}
		}
	}

	// Triggers
	else if (fixtureA->IsSensor() && fixtureB->IsSensor()) {
		collision.point = b2Vec2(-999.0f, -999.0f);
		collision.normal = b2Vec2(-999.0f, -999.0f);
		std::map<std::string, luabridge::LuaRef>& onTriggerExitComponents = actorA->onTriggerExitComponents;
		collision.other = actorB;
		for (auto& pair : onTriggerExitComponents) {
			// If the actor gets disabled, don't finish running its components
			if (actorA->enabled == false)
				break;

			luabridge::LuaRef component = pair.second;

			if (component["enabled"] == false)
				continue;
			try {
				component["OnTriggerExit"](component, collision);
			}
			catch (luabridge::LuaException e) {
				std::string errorMessage = e.what();
				std::replace(std::begin(errorMessage), std::end(errorMessage), '\\', '/');
				std::cout << "\033[31m" << actorA->GetName() << " : " << errorMessage << "\033[0m" << std::endl;
			}
		}

		onTriggerExitComponents = actorB->onTriggerExitComponents;
		collision.other = actorA;
		for (auto& pair : onTriggerExitComponents) {
			// If the actor gets disabled, don't finish running its components
			if (actorB->enabled == false)
				break;

			luabridge::LuaRef component = pair.second;

			if (component["enabled"] == false)
				continue;
			try {
				component["OnTriggerExit"](component, collision);
			}
			catch (luabridge::LuaException e) {
				std::string errorMessage = e.what();
				std::replace(std::begin(errorMessage), std::end(errorMessage), '\\', '/');
				std::cout << "\033[31m" << actorB->GetName() << " : " << errorMessage << "\033[0m" << std::endl;
			}
		}
	}

}