#pragma once

#include "SDL2/SDL.h"
#include <unordered_map>
#include <vector>
#include <string>
#include "glm/glm.hpp"

enum INPUT_STATE {
	INPUT_STATE_UP,
	INPUT_STATE_JUST_BECAME_DOWN,
	INPUT_STATE_DOWN,
	INPUT_STATE_JUST_BECAME_UP
};

class Input
{
public:
	static bool GetKey(const std::string& keycode);
	static bool GetKeyDown(const std::string& keycode);
	static bool GetKeyUp(const std::string& keycode);
	static bool GetMouseButton(int buttonNum);
	static bool GetMouseButtonDown(int buttonNum);
	static bool GetMouseButtonUp(int buttonNum);
	static glm::vec2 GetMousePosition();
	static float GetMouseScrollDelta();

	static bool GetControllerButton(const std::string& buttonAlias);
	static bool GetControllerButtonDown(const std::string& buttonAlias);
	static bool GetControllerButtonUp(const std::string& buttonAlias);
	static float GetControllerAxis(const std::string& axisAlias);

	static void Init();
	static void ProcessEvent(const SDL_Event& e);
	static void LateUpdate();

private:
	static inline std::unordered_map<SDL_Scancode, INPUT_STATE> keyStates;
	static inline std::unordered_map<SDL_GameControllerButton, INPUT_STATE> buttonStates;
	static inline std::unordered_map<SDL_GameControllerAxis, float> axisStates;
	static inline std::vector<INPUT_STATE> mouseButtonStates; // 0 : Left button, 1 : Middle Button, 2 : Right Button
	static inline std::vector<SDL_Scancode> justBecameUpKeys, justBecameDownKeys;
	static inline std::vector<SDL_GameControllerButton> justBecameUpButtons, justBecameDownButtons;
	static inline std::vector<SDL_GameController*> controllers;
	static inline glm::vec2 mousePosition;
	static inline float mouseScrollDelta;

	static void AddController(int id);
	static void RemoveController(int id);

	static inline const std::unordered_map<std::string, SDL_Scancode> keycodeToScancode = {
		// Directional (arrow) Keys
		{"up", SDL_SCANCODE_UP},
		{"down", SDL_SCANCODE_DOWN},
		{"right", SDL_SCANCODE_RIGHT},
		{"left", SDL_SCANCODE_LEFT},

		// Misc Keys
		{"escape", SDL_SCANCODE_ESCAPE},

		// Modifier Keys
		{"lshift", SDL_SCANCODE_LSHIFT},
		{"rshift", SDL_SCANCODE_RSHIFT},
		{"lctrl", SDL_SCANCODE_LCTRL},
		{"rctrl", SDL_SCANCODE_RCTRL},
		{"lalt", SDL_SCANCODE_LALT},
		{"ralt", SDL_SCANCODE_RALT},

		// Editing Keys
		{"tab", SDL_SCANCODE_TAB},
		{"return", SDL_SCANCODE_RETURN},
		{"enter", SDL_SCANCODE_RETURN},
		{"backspace", SDL_SCANCODE_BACKSPACE},
		{"delete", SDL_SCANCODE_DELETE},
		{"insert", SDL_SCANCODE_INSERT},

		// Character Keys
		{"space", SDL_SCANCODE_SPACE},
		{"a", SDL_SCANCODE_A},
		{"b", SDL_SCANCODE_B},
		{"c", SDL_SCANCODE_C},
		{"d", SDL_SCANCODE_D},
		{"e", SDL_SCANCODE_E},
		{"f", SDL_SCANCODE_F},
		{"g", SDL_SCANCODE_G},
		{"h", SDL_SCANCODE_H},
		{"i", SDL_SCANCODE_I},
		{"j", SDL_SCANCODE_J},
		{"k", SDL_SCANCODE_K},
		{"l", SDL_SCANCODE_L},
		{"m", SDL_SCANCODE_M},
		{"n", SDL_SCANCODE_N},
		{"o", SDL_SCANCODE_O},
		{"p", SDL_SCANCODE_P},
		{"q", SDL_SCANCODE_Q},
		{"r", SDL_SCANCODE_R},
		{"s", SDL_SCANCODE_S},
		{"t", SDL_SCANCODE_T},
		{"u", SDL_SCANCODE_U},
		{"v", SDL_SCANCODE_V},
		{"w", SDL_SCANCODE_W},
		{"x", SDL_SCANCODE_X},
		{"y", SDL_SCANCODE_Y},
		{"z", SDL_SCANCODE_Z},
		{"0", SDL_SCANCODE_0},
		{"1", SDL_SCANCODE_1},
		{"2", SDL_SCANCODE_2},
		{"3", SDL_SCANCODE_3},
		{"4", SDL_SCANCODE_4},
		{"5", SDL_SCANCODE_5},
		{"6", SDL_SCANCODE_6},
		{"7", SDL_SCANCODE_7},
		{"8", SDL_SCANCODE_8},
		{"9", SDL_SCANCODE_9},
		{"/", SDL_SCANCODE_SLASH},
		{";", SDL_SCANCODE_SEMICOLON},
		{"=", SDL_SCANCODE_EQUALS},
		{"-", SDL_SCANCODE_MINUS},
		{".", SDL_SCANCODE_PERIOD},
		{",", SDL_SCANCODE_COMMA},
		{"[", SDL_SCANCODE_LEFTBRACKET},
		{"]", SDL_SCANCODE_RIGHTBRACKET},
		{"\\", SDL_SCANCODE_BACKSLASH},
		{"'", SDL_SCANCODE_APOSTROPHE}
	};

	static inline const std::unordered_map<std::string, SDL_GameControllerButton> buttonAliasToButton = {
		{"invalid", SDL_CONTROLLER_BUTTON_INVALID},
		{"south", SDL_CONTROLLER_BUTTON_A},
		{"east", SDL_CONTROLLER_BUTTON_B},
		{"west", SDL_CONTROLLER_BUTTON_X},
		{"north", SDL_CONTROLLER_BUTTON_Y},
		{"back", SDL_CONTROLLER_BUTTON_BACK},
		{"guide", SDL_CONTROLLER_BUTTON_GUIDE},
		{"start", SDL_CONTROLLER_BUTTON_START},
		{"lstick", SDL_CONTROLLER_BUTTON_LEFTSTICK},
		{"rstick", SDL_CONTROLLER_BUTTON_RIGHTSTICK},
		{"lshoulder", SDL_CONTROLLER_BUTTON_LEFTSHOULDER},
		{"rshoulder", SDL_CONTROLLER_BUTTON_RIGHTSHOULDER},
		{"dpad_up", SDL_CONTROLLER_BUTTON_DPAD_UP},
		{"dpad_down", SDL_CONTROLLER_BUTTON_DPAD_DOWN},
		{"dpad_left", SDL_CONTROLLER_BUTTON_DPAD_LEFT},
		{"dpad_right", SDL_CONTROLLER_BUTTON_DPAD_RIGHT},
		{"misc1", SDL_CONTROLLER_BUTTON_MISC1},
		{"paddle1", SDL_CONTROLLER_BUTTON_PADDLE1},
		{"paddle2", SDL_CONTROLLER_BUTTON_PADDLE2},
		{"paddle3", SDL_CONTROLLER_BUTTON_PADDLE3},
		{"paddle4", SDL_CONTROLLER_BUTTON_PADDLE4},
		{"touchpad", SDL_CONTROLLER_BUTTON_TOUCHPAD}
	};

	static inline const std::unordered_map<std::string, SDL_GameControllerAxis> axisAliasToAxis = {
		{"invalid", SDL_CONTROLLER_AXIS_INVALID},
		{"left_horizontal", SDL_CONTROLLER_AXIS_LEFTX},
		{"left_vertical", SDL_CONTROLLER_AXIS_LEFTY},
		{"right_horizontal", SDL_CONTROLLER_AXIS_RIGHTX},
		{"right_vertical", SDL_CONTROLLER_AXIS_RIGHTY},
		{"left_trigger", SDL_CONTROLLER_AXIS_TRIGGERLEFT},
		{"right_trigger", SDL_CONTROLLER_AXIS_TRIGGERRIGHT}
	};
};
