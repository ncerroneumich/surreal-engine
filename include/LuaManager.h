#pragma once

#include "lua/lua.hpp"
#include "LuaBridge/LuaBridge.h"
#include <string>

class LuaManager {
public:
	static inline lua_State* lua_state;

	static void InitializeState() {
		lua_state = luaL_newstate();
		luaL_openlibs(lua_state);
	}

	static int DoString(const char* code) {
		return luaL_dostring(lua_state, code);
	}

	static int DoFile(const char* filename) {
		return luaL_dofile(lua_state, filename);
	}

private:
	LuaManager() {}
};
