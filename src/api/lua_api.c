#include "lua_api.h"

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include "api.h"

static lua_State *_lua = NULL;

// The following static functions are the lua api handlers of the native api functions
static void lua_cls(lua_State *lua) {
	if (lua_gettop(lua) == 1) {
		if (lua_isnumber(lua, 1)) {
			computer_t *computer = get_global_computer();
			int color = lua_tointeger(lua, 1) % PALETTE_SIZE;

			api_cls(computer, color);
		} else {
			luaL_error(lua, "cls() argument should be an integer");
		}
	} else {
		luaL_error(lua, "cls() expects exactly 1 argument");
	}
}

static void lua_spr(lua_State *lua) {
	if (lua_gettop(lua) == 5) {
		if (lua_isnumber(lua, 1) && lua_isnumber(lua, 2) && lua_isnumber(lua, 3) && lua_isnumber(lua, 4) && lua_isnumber(lua, 5)) {
			computer_t *computer = get_global_computer();
			int sprite_index = (int)lua_tonumber(lua, 1);
			int x = (int)lua_tonumber(lua, 2);
			int y = (int)lua_tonumber(lua, 3);
			int width = (int)lua_tonumber(lua, 4);
			int height = (int)lua_tonumber(lua, 5);

			// TODO: add support for scale
			api_spr(computer, sprite_index, x, y, width, height, 1);
		}
	}
}

static void lua_circ(lua_State *lua) {
	if (lua_gettop(lua) == 4) {
		if (lua_isnumber(lua, 1) && lua_isnumber(lua, 2) && lua_isnumber(lua, 3) && lua_isnumber(lua, 4)) {
			computer_t *computer = get_global_computer();
			int x = (int)lua_tonumber(lua, 1);
			int y = (int)lua_tonumber(lua, 2);
			int radius = (int)lua_tonumber(lua, 3);
			int color = (int)lua_tonumber(lua, 4);

			api_circ(computer, x, y, radius, color);
		}
	}
}

static int lua_ticks(lua_State *lua) {
	int ticks = api_ticks(get_global_computer());
	lua_pushinteger(lua, ticks);
	return 1;
}

void lua_init(computer_t *computer) {
	_lua = luaL_newstate();
	luaL_openlibs(_lua);

	lua_register(_lua, "cls", lua_cls);
	lua_register(_lua, "spr", lua_spr);
	lua_register(_lua, "circ", lua_circ);
	lua_register(_lua, "ticks", lua_ticks);

	// if (luaL_dofile(_lua, "test.lua") != LUA_OK) {
	// if (luaL_dostring(_lua, computer->code->buffer) != LUA_OK) {
	if (luaL_dostring(_lua, computer->ram->code_buffer) != LUA_OK) {
		printf("Error loading Lua script: %s\n", lua_tostring(_lua, -1));
		lua_pop(_lua, 1);
	}
}

void lua_call_init() {
	lua_getglobal(_lua, "_init");

	if (lua_isfunction(_lua, -1)) {
		if (lua_pcall(_lua, 0, 0, 0) != LUA_OK) {
			printf("Error in _init: %s\n", lua_tostring(_lua, -1));
			lua_pop(_lua, 1);
		} 
	} else {
		lua_pop(_lua, 1);
	}
}

void lua_call_update() {
	lua_getglobal(_lua, "_update");

	if (lua_isfunction(_lua, -1)) {
		if (lua_pcall(_lua, 0, 0, 0) != LUA_OK) {
			printf("Error in _update: %s\n", lua_tostring(_lua, -1));
			lua_pop(_lua, 1);
		} 
	} else {
		lua_pop(_lua, 1);
	}
}

void lua_call_draw() {
	lua_getglobal(_lua, "_draw");

	if (lua_isfunction(_lua, -1)) {
		if (lua_pcall(_lua, 0, 0, 0) != LUA_OK) {
			printf("Error in _draw: %s\n", lua_tostring(_lua, -1));
			lua_pop(_lua, 1);
		} 
	} else {
		lua_pop(_lua, 1);
	}
}

void lua_quit() {
	lua_settop(_lua, 0);
	lua_close(_lua);
	
	_lua = NULL;
}