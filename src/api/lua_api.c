#include "lua_api.h"

#include <string.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include "api.h"

#include "../backend/txt.h"

static lua_State *_lua = NULL;

// The following static functions are the lua api handlers of the native api functions
// The return value is the number of return values that are pushed to the lua stack
static int _lua_cls(lua_State *lua) {
	if (lua_gettop(lua) == 1) {
		if (lua_isnumber(lua, 1)) {
			computer_t *computer = get_global_computer();
			int color = (int)lua_tonumber(lua, 1) % PALETTE_SIZE;

			api_cls(computer->ram, color);
		} else {
			luaL_error(lua, "cls() argument should be an integer");
		}
	} else if (lua_gettop(lua) == 0) {
		computer_t *computer = get_global_computer();
		api_cls(computer->ram, 0);
	} else {
		luaL_error(lua, "cls() expects exactly 0 or 1 arguments");
	}

	return 0;
}

static int _lua_spr(lua_State *lua) {
	if (lua_gettop(lua) == 5) {
		if (lua_isnumber(lua, 1) && lua_isnumber(lua, 2) && lua_isnumber(lua, 3) && lua_isnumber(lua, 4) && lua_isnumber(lua, 5)) {
			computer_t *computer = get_global_computer();
			int sprite_index = (int)lua_tonumber(lua, 1);
			int x = (int)lua_tonumber(lua, 2);
			int y = (int)lua_tonumber(lua, 3);
			int width = (int)lua_tonumber(lua, 4);
			int height = (int)lua_tonumber(lua, 5);

			// TODO: add support for scale
			// api_spr(computer, sprite_index, x, y, width, height, 1);
			api_spr(computer->ram, sprite_index, x, y, width, height);
		}
	}

	return 0;
}

static int _lua_circ(lua_State *lua) {
	if (lua_gettop(lua) == 4) {
		if (lua_isnumber(lua, 1) && lua_isnumber(lua, 2) && lua_isnumber(lua, 3) && lua_isnumber(lua, 4)) {
			computer_t *computer = get_global_computer();
			int x = (int)lua_tonumber(lua, 1);
			int y = (int)lua_tonumber(lua, 2);
			int radius = (int)lua_tonumber(lua, 3);
			int color = (int)lua_tonumber(lua, 4);

			api_circ(computer->ram, x, y, radius, color);
		}
	}

	return 0;
}

static int _lua_map(lua_State *lua) {
	if (lua_gettop(lua) == 7) {
		if (lua_isnumber(lua, 1) && lua_isnumber(lua, 2) && lua_isnumber(lua, 3) && lua_isnumber(lua, 4) && lua_isnumber(lua, 5) && lua_isnumber(lua, 6) && lua_isnumber(lua, 7)) {
			computer_t *computer = get_global_computer();

			int layer = (int)lua_tonumber(lua, 1);

			int x = (int)lua_tonumber(lua, 2);
			int y = (int)lua_tonumber(lua, 3);

			int cell_x = (int)lua_tonumber(lua, 4);
			int cell_y = (int)lua_tonumber(lua, 5);
			
			int cell_w = (int)lua_tonumber(lua, 6);
			int cell_h = (int)lua_tonumber(lua, 7);

			api_map(computer->ram, layer, x, y, cell_x, cell_y, cell_w, cell_h);
		}
	}

	return 0;
}

static int _lua_ticks(lua_State *lua) {
	computer_t *computer = get_global_computer();
	int ticks = api_ticks(computer->ram);
	lua_pushinteger(lua, ticks);
	
	return 1;
}

static int _lua_key(lua_State *lua) {
	computer_t *computer = get_global_computer();
	if (lua_gettop(lua) == 1) {
		int key = (int)lua_tonumber(lua, 1);

		bool pressed = api_key(computer->ram, key);
		lua_pushboolean(lua, pressed);
		return 1;
	}

	lua_pushboolean(lua, false);
	return 1;
}

static int _lua_print(lua_State *lua) {
	computer_t *computer = get_global_computer();

	term_putchar(computer->ram, '\n', COLOR_BLACK, COLOR_WHITE);

	int args_amount = lua_gettop(lua);
	for (int i = 1; i <= args_amount; i++) {
		const char *buffer = lua_tostring(lua, i);
		
		if (buffer) {
			term_printc(computer->ram, STR(buffer), COLOR_BLACK, COLOR_WHITE);
		}
	}

	return 0;
}

// The following is copy-pasted and edited from the Lua docs and has some parts of the standard library commented out
// so that the game cannot do dangerous things to the host system
static const luaL_Reg loadedlibs[] = {
	{"_G", luaopen_base},
	{LUA_LOADLIBNAME, luaopen_package},
	// {LUA_COLIBNAME, luaopen_coroutine},
	{LUA_TABLIBNAME, luaopen_table},
	// {LUA_IOLIBNAME, luaopen_io},
	// {LUA_OSLIBNAME, luaopen_os},
	{LUA_STRLIBNAME, luaopen_string},
	{LUA_MATHLIBNAME, luaopen_math},
	// {LUA_UTF8LIBNAME, luaopen_utf8},
	// {LUA_DBLIBNAME, luaopen_debug},
#if defined(LUA_COMPAT_BITLIB)
	{LUA_BITLIBNAME, luaopen_bit32},
#endif
	{NULL, NULL}
};

static void _open_safe_libs(lua_State *lua) {
	const luaL_Reg *lib;
	/* "require" functions from 'loadedlibs' and set results to global table */
	for (lib = loadedlibs; lib->func; lib++) {
		luaL_requiref(lua, lib->name, lib->func, 1);
		lua_pop(lua, 1);  /* remove lib */
	}
}

void lua_init(computer_t *computer, string_t code) {
	_lua = luaL_newstate();
	_open_safe_libs(_lua);

	// TODO: handle this in a loop based on the api metas
	// maybe not because then I have issues with circular dependency
	lua_register(_lua, "print", _lua_print);
	lua_register(_lua, api_metas[API_FUNC_CLS].name, _lua_cls);
	lua_register(_lua, api_metas[API_FUNC_SPR].name, _lua_spr);
	lua_register(_lua, api_metas[API_FUNC_CIRC].name, _lua_circ);
	lua_register(_lua, api_metas[API_FUNC_TICKS].name, _lua_ticks);
	lua_register(_lua, api_metas[API_FUNC_MAP].name, _lua_map);
	lua_register(_lua, api_metas[API_FUNC_KEY].name, _lua_key);

	if (luaL_loadbuffer(_lua, code.data, code.len, "all_code") != LUA_OK) {
		term_printc(computer->ram, STR("\nSyntax error: "), COLOR_BLACK, COLOR_RED);
		term_printc(computer->ram, STR(lua_tostring(_lua, -1)), 0, 12);
		lua_pop(_lua, 1);
	} else if (lua_pcall(_lua, 0, LUA_MULTRET, 0) != LUA_OK) {
		term_printc(computer->ram, STR("\nRuntime error: "), COLOR_BLACK, COLOR_RED);
		term_printc(computer->ram, STR(lua_tostring(_lua, -1)), 0, 12);
		lua_pop(_lua, 1);
	}
}

void lua_call_init() {
	computer_t *computer = get_global_computer();

	lua_getglobal(_lua, "_init");

	if (lua_isfunction(_lua, -1)) {
		if (lua_pcall(_lua, 0, 0, 0) != LUA_OK) {
			term_printc(computer->ram, STR("\nRuntime error in _init: "), COLOR_BLACK, COLOR_RED);
			term_printc(computer->ram, STR(lua_tostring(_lua, -1)), COLOR_BLACK, COLOR_RED);
			lua_pop(_lua, 1);
		} 
	} else {
		lua_pop(_lua, 1);
	}
}

void lua_call_update() {
	computer_t *computer = get_global_computer();

	lua_getglobal(_lua, "_update");

	if (lua_isfunction(_lua, -1)) {
		if (lua_pcall(_lua, 0, 0, 0) != LUA_OK) {
			term_printc(computer->ram, STR("\nRuntime error in _update: "), COLOR_BLACK, COLOR_RED);
			term_printc(computer->ram, STR(lua_tostring(_lua, -1)), COLOR_BLACK, COLOR_RED);
			lua_pop(_lua, 1);
		} 
	} else {
		lua_pop(_lua, 1);
	}
}

void lua_call_draw() {
	computer_t *computer = get_global_computer();

	lua_getglobal(_lua, "_draw");

	if (lua_isfunction(_lua, -1)) {
		if (lua_pcall(_lua, 0, 0, 0) != LUA_OK) {
			term_printc(computer->ram, STR("\nRuntime error in _draw: "), COLOR_BLACK, COLOR_RED);
			term_printc(computer->ram, STR(lua_tostring(_lua, -1)), COLOR_BLACK, COLOR_RED);
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