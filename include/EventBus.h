#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include "lua/lua.hpp"
#include "LuaBridge/LuaBridge.h"

struct Subscription {
	std::string eventType;
	luabridge::LuaRef component;
	luabridge::LuaRef callback;

	Subscription(const std::string& _eventType, luabridge::LuaRef _component, luabridge::LuaRef _callback) : 
		eventType(_eventType), component(_component), callback(_callback) {}
};

class EventBus
{
public:
	// When an Event.Publish() call occurs, all functions that have been Event.Subscribe()’d
	// to the specified event_type will be called in the order of their sub.
	// When these functions are called, they will be passed a reference to their component 
	// (allowing self to work) and event_object (allowing event publishers to provide additional
	// information / context about an event) as parameters.
	// The subscribed functions will be called in the order they subscribed.
	static void Publish(const std::string&, luabridge::LuaRef eventObject);
	static void Subscribe(const std::string&, luabridge::LuaRef component, luabridge::LuaRef function);
	// May be used by scripts to prevent a component-function pair from ever being called 
	// (even if a subscribed-to event is published again).
	static void Unsubscribe(const std::string&, luabridge::LuaRef component, luabridge::LuaRef function);

	static void HandleSubscriptionQueues();

private:
	static inline std::unordered_map<std::string, std::vector<Subscription>> eventTypeToSubscribers;
	static inline std::vector<Subscription> subscribeQueue, unsubscribeQueue;
};
