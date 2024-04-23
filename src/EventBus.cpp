#include "EventBus.h"

void EventBus::Publish(const std::string& eventType, luabridge::LuaRef eventObject) {
    std::vector<Subscription>& subscribers = eventTypeToSubscribers[eventType];
    std::vector<Subscription> extraneousSubscriptions;
    for (auto& subscription : subscribers) {
        try {
            if (subscription.callback.isFunction()) {
                subscription.callback(subscription.component, eventObject);
            }
            else {
                extraneousSubscriptions.push_back(subscription);
            }
        }
        catch (const luabridge::LuaException& e) {
            extraneousSubscriptions.push_back(subscription);
        }

        for (auto& subscription : extraneousSubscriptions) {
            EventBus::Unsubscribe(subscription.eventType, subscription.component, subscription.callback);
        }
    }
}

void EventBus::Subscribe(const std::string& eventType, luabridge::LuaRef component, luabridge::LuaRef function) {
    
    Subscription sub(eventType, component, function);
    subscribeQueue.push_back(sub);
}

void EventBus::Unsubscribe(const std::string& eventType, luabridge::LuaRef component, luabridge::LuaRef function) {
    Subscription sub(eventType, component, function);
    unsubscribeQueue.push_back(sub);
}

void EventBus::HandleSubscriptionQueues() {
    for (const Subscription& sub : subscribeQueue) {
        eventTypeToSubscribers[sub.eventType].push_back(sub);
    }
    subscribeQueue.clear();
    for (const Subscription& sub : unsubscribeQueue) {
        std::vector<Subscription>& subscriptions = eventTypeToSubscribers[sub.eventType];
        subscriptions.erase(std::remove_if(subscriptions.begin(), subscriptions.end(),
            [&sub](const Subscription& s) {
                return s.component == sub.component && s.callback == sub.callback;
            }), subscriptions.end());
    }
    subscribeQueue.clear();
}
