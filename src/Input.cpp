#include "Input.h"
#include <iostream>

void Input::Init() {
	for (int code = SDL_SCANCODE_UNKNOWN; code < SDL_NUM_SCANCODES; code++) {
		keyStates[static_cast<SDL_Scancode>(code)] = INPUT_STATE_UP;
	}

	mousePosition = { 0, 0 };
	mouseButtonStates = { INPUT_STATE_UP, INPUT_STATE_UP, INPUT_STATE_UP };
	mouseScrollDelta = 0;

	SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER);

	for (int i = 0; i < SDL_NumJoysticks(); ++i) {
		if (SDL_IsGameController(i)) {
			AddController(i);
		}
	}
}

void Input::AddController(int id) {
	if (!SDL_IsGameController(id)) 
		return;
	SDL_GameController* controller = SDL_GameControllerOpen(id);
	if (controller) {
		controllers.push_back(controller);
	}
}

void Input::RemoveController(int id) {
	for (auto it = controllers.begin(); it != controllers.end(); ++it) {
		if (SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(*it)) == id) {
			SDL_GameControllerClose(*it);
			controllers.erase(it);
			break;
		}
	}
}

void Input::ProcessEvent(const SDL_Event& e) {
	if (e.type == SDL_KEYDOWN) {
		keyStates[e.key.keysym.scancode] = INPUT_STATE_JUST_BECAME_DOWN;
		justBecameDownKeys.push_back(e.key.keysym.scancode);
	}
	else if (e.type == SDL_KEYUP) {
		keyStates[e.key.keysym.scancode] = INPUT_STATE_JUST_BECAME_UP;
		justBecameUpKeys.push_back(e.key.keysym.scancode);
	}
	else if (e.type == SDL_MOUSEBUTTONDOWN) {
		if (e.button.button == SDL_BUTTON_LEFT) {
			mouseButtonStates[0] = INPUT_STATE_JUST_BECAME_DOWN;
		}
		else if (e.button.button == SDL_BUTTON_MIDDLE) {
			mouseButtonStates[1] = INPUT_STATE_JUST_BECAME_DOWN;
		}
		else if (e.button.button == SDL_BUTTON_RIGHT) {
			mouseButtonStates[2] = INPUT_STATE_JUST_BECAME_DOWN;
		}
	}
	else if (e.type == SDL_MOUSEBUTTONUP) {
		if (e.button.button == SDL_BUTTON_LEFT) {
			mouseButtonStates[0] = INPUT_STATE_JUST_BECAME_UP;
		}
		else if (e.button.button == SDL_BUTTON_MIDDLE) {
			mouseButtonStates[1] = INPUT_STATE_JUST_BECAME_UP;
		}
		else if (e.button.button == SDL_BUTTON_RIGHT) {
			mouseButtonStates[2] = INPUT_STATE_JUST_BECAME_UP;
		}
	}
	else if (e.type == SDL_MOUSEMOTION) {
		mousePosition = { e.motion.x, e.motion.y };
	}
	else if (e.type == SDL_MOUSEWHEEL) {
		mouseScrollDelta = e.wheel.preciseY;
	}
	else if (e.type == SDL_CONTROLLERDEVICEADDED) {
		AddController(e.cdevice.which);
	}
	else if (e.type == SDL_CONTROLLERDEVICEREMOVED) {
		RemoveController(e.cdevice.which);
	}
	else if (e.type == SDL_CONTROLLERBUTTONDOWN) {
		SDL_GameControllerButton button = static_cast<SDL_GameControllerButton>(e.cbutton.button);
		buttonStates[button] = INPUT_STATE_JUST_BECAME_DOWN;
		justBecameDownButtons.push_back(button);
	}
	else if (e.type == SDL_CONTROLLERBUTTONUP) {
		SDL_GameControllerButton button = static_cast<SDL_GameControllerButton>(e.cbutton.button);
		buttonStates[button] = INPUT_STATE_JUST_BECAME_UP;
		justBecameUpButtons.push_back(button);
	}
	else if (e.type == SDL_JOYAXISMOTION) {
		SDL_GameControllerAxis axis = static_cast<SDL_GameControllerAxis>(e.caxis.axis);
		axisStates[axis] = e.caxis.value;
	}
}

void Input::LateUpdate() {
	for (const SDL_Scancode& code : justBecameDownKeys) {
		keyStates[code] = INPUT_STATE_DOWN;
	}
	for (const SDL_Scancode& code : justBecameUpKeys) {
		keyStates[code] = INPUT_STATE_UP;
	}
	for (const SDL_GameControllerButton button : justBecameDownButtons) {
		buttonStates[button] = INPUT_STATE_DOWN;
	}
	for (const SDL_GameControllerButton button : justBecameUpButtons) {
		buttonStates[button] = INPUT_STATE_UP;
	}

	for (int i = 0; i < mouseButtonStates.size(); i++) {
		if (mouseButtonStates[i] == INPUT_STATE_JUST_BECAME_DOWN) {
			mouseButtonStates[i] = INPUT_STATE_DOWN;
		}
		else if (mouseButtonStates[i] == INPUT_STATE_JUST_BECAME_UP) {
			mouseButtonStates[i] = INPUT_STATE_UP;
		}
	}

	//for (auto& pair : axisStates) {
	//	pair.second = 0;
	//}

	mouseScrollDelta = 0;

	justBecameDownKeys.clear();
	justBecameUpKeys.clear();
	justBecameDownButtons.clear();
	justBecameUpButtons.clear();
}

bool Input::GetKey(const std::string& keycode) {
	const auto& it = keycodeToScancode.find(keycode);

	if (it == std::end(keycodeToScancode))
		return false;

	SDL_Scancode scancode = it->second;

	return keyStates[scancode] == INPUT_STATE_DOWN || keyStates[scancode] == INPUT_STATE_JUST_BECAME_DOWN;
}

bool Input::GetKeyDown(const std::string& keycode) {
	const auto& it = keycodeToScancode.find(keycode);

	if (it == std::end(keycodeToScancode))
		return false;

	SDL_Scancode scancode = it->second;

	return keyStates[scancode] == INPUT_STATE_JUST_BECAME_DOWN;
}

bool Input::GetKeyUp(const std::string& keycode) {
	const auto& it = keycodeToScancode.find(keycode);

	if (it == std::end(keycodeToScancode))
		return false;

	SDL_Scancode scancode = it->second;

	return keyStates[scancode] == INPUT_STATE_JUST_BECAME_UP;
}

bool Input::GetMouseButton(int buttonNum) {
	if (buttonNum < 1 || buttonNum > 3)
		return false;

	return mouseButtonStates[buttonNum - 1] == INPUT_STATE_DOWN || mouseButtonStates[buttonNum - 1] == INPUT_STATE_JUST_BECAME_DOWN;
}

bool Input::GetMouseButtonDown(int buttonNum) {
	if (buttonNum < 1 || buttonNum > 3)
		return false;

	return mouseButtonStates[buttonNum - 1] == INPUT_STATE_JUST_BECAME_DOWN;
}

bool Input::GetMouseButtonUp(int buttonNum) {
	if (buttonNum < 1 || buttonNum > 3)
		return false;

	return mouseButtonStates[buttonNum - 1] == INPUT_STATE_JUST_BECAME_UP;
}

glm::vec2 Input::GetMousePosition() {
	return mousePosition;
}

float Input::GetMouseScrollDelta() {
	return mouseScrollDelta;
}

bool Input::GetControllerButton(const std::string& buttonAlias) {
	if (controllers.size() <= 0)
		return false;

	const auto& it = buttonAliasToButton.find(buttonAlias);

	if (it == std::end(buttonAliasToButton))
		return false;

	SDL_GameControllerButton button = it->second;

	return buttonStates[button] == INPUT_STATE_DOWN || buttonStates[button] == INPUT_STATE_JUST_BECAME_DOWN;
}

bool Input::GetControllerButtonDown(const std::string& buttonAlias) {
	if (controllers.size() <= 0)
		return false;

	const auto& it = buttonAliasToButton.find(buttonAlias);

	if (it == std::end(buttonAliasToButton))
		return false;

	SDL_GameControllerButton button = it->second;

	return buttonStates[button] == INPUT_STATE_JUST_BECAME_DOWN;
}

bool Input::GetControllerButtonUp(const std::string& buttonAlias) {
	if (controllers.size() <= 0)
		return false;
	
	const auto& it = buttonAliasToButton.find(buttonAlias);

	if (it == std::end(buttonAliasToButton))
		return false;

	SDL_GameControllerButton button = it->second;

	return buttonStates[button] == INPUT_STATE_JUST_BECAME_UP;
}

float Input::GetControllerAxis(const std::string& axisAlias) {
	if (controllers.size() <= 0)
		return false;

	const auto& it = axisAliasToAxis.find(axisAlias);

	if (it == std::end(axisAliasToAxis))
		return false;

	SDL_GameControllerAxis axis = it->second;

	// Limit the range to [-1, 1]
	return std::fmin(std::fmax(axisStates[axis] / 32767.0f, -1), 1);
}