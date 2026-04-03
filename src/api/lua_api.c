#include "lua_api.h"

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include "api.h"

#include "../backend/txt.h"
#include "../common/io.h"

static lua_State *_lua = NULL;

// TODO: throw errors when amount of arguments is not correct

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

static int _lua_keyp(lua_State *lua) {
	computer_t *computer = get_global_computer();
	if (lua_gettop(lua) == 1) {
		int key = (int)lua_tonumber(lua, 1);

		bool pressed = api_keyp(computer->ram, key);
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

static void _serialize_lua_value(lua_State *lua, int index, string_builder_t *builder) {
	// Make index absolute
	if (index < 0) {
		index = lua_gettop(lua) + index + 1;
	}
	
	int type = lua_type(lua, index);

	switch (type) {
		case LUA_TSTRING: {
			string_builder_append(builder, STR("\""));
			string_builder_append(builder, STR(lua_tostring(lua, index)));
			string_builder_append(builder, STR("\""));
			
			break;
		}

		case LUA_TNUMBER: {
			// Using sprintf here because I can't be bothered to implement my own float to string function right now
			char number_buffer[64];
			sprintf(number_buffer, "%g", lua_tonumber(lua, index));
			string_builder_append(builder, STR(number_buffer));
			
			break;
		}

		case LUA_TBOOLEAN: {
			int value = lua_toboolean(lua, index);
			if (value) {
				string_builder_append(builder, STR("true"));
			} else {
				string_builder_append(builder, STR("false"));
			}
			
			break;
		}

		case LUA_TTABLE: {
			string_builder_append(builder, STR("{"));

			lua_pushnil(lua);
			while (lua_next(lua, index) != 0) {
				// Index -1: value
				// Index -2: key

				string_builder_append(builder, STR("["));

				_serialize_lua_value(lua, -2, builder);

				string_builder_append(builder, STR("]"));
				string_builder_append(builder, STR("="));
				_serialize_lua_value(lua, -1, builder);
				string_builder_append(builder, STR(","));

				lua_pop(lua, 1);
			}

			string_builder_append(builder, STR("}"));
			
			break;
		}

		case LUA_TNIL: {
			string_builder_append(builder, STR("nil"));

			break;
		}
	}
}

static int _lua_save_to_slot(lua_State *lua) {
	computer_t *computer = get_global_computer();

	if (lua_gettop(lua) == 2) {
		int slot_index = (int)lua_tonumber(lua, 1);
		if (slot_index < 0 || slot_index > 9) {
			return luaL_error(lua, "Save slot index is out of bounds");
		}

		if (!lua_istable(lua, 2)) {
			return luaL_error(lua, "Second argument is not a table");
		}

		string_builder_t builder = {0};
		string_builder_init(&builder, get_heap_allocator(), 128);
		string_builder_append(&builder, STR("return "));
		_serialize_lua_value(lua, 2, &builder);

		string_t game_name = path_truncate_extension(path_get_filename(computer->game_path));
		string_t game_saves_dir = path_append(get_temp_allocator(), STR("saves"), game_name);
		create_directory(game_saves_dir);

		string_t savefile_name = path_append(get_temp_allocator(), game_saves_dir, int_to_string(get_temp_allocator(), slot_index));
		savefile_name = string_concat(get_temp_allocator(), savefile_name, STR(".lua"));
		string_t absolute_path = get_absolute_path(get_temp_allocator(), savefile_name);

		file_write_string(absolute_path, builder.string);

		string_builder_deinit(&builder);
	} else {
		return luaL_error(lua, "Expected 2 arguments");
	}

	return 0;
}

// Returns nil when there is an error in dofile or if the value in the file is not a table
static int _lua_load_from_slot(lua_State *lua) {
	computer_t *computer = get_global_computer();

	if (lua_gettop(lua) == 1) {
		int slot_index = (int)lua_tonumber(lua, 1);
		if (slot_index < 0 || slot_index > 9) {
			return luaL_error(lua, "Save slot index is out of bounds");
		}

		string_t game_name = path_truncate_extension(path_get_filename(computer->game_path));
		string_t game_saves_dir = path_append(get_temp_allocator(), STR("saves"), game_name);
		create_directory(game_saves_dir);

		string_t savefile_name = path_append(get_temp_allocator(), game_saves_dir, int_to_string(get_temp_allocator(), slot_index));
		savefile_name = string_concat(get_temp_allocator(), savefile_name, STR(".lua"));
		string_t absolute_path = get_absolute_path(get_temp_allocator(), savefile_name);

		// If the return value is equal to LUA_OK it will push the table in the file to the lua stack
		// Thus there is no explicit lua_pushwhatever
		if (luaL_dofile(lua, string_to_c_string(get_temp_allocator(), absolute_path)) != LUA_OK) {
			lua_pushnil(lua);
			return 1;
		}

		if (!lua_istable(lua, -1)) {
			lua_pushnil(lua);
			return 1;
		}

		return 1;
	}

	return luaL_error(lua, "Expected 1 argument");
}

static int _lua_get_ents(lua_State *lua) {
	computer_t *computer = get_global_computer();
	

	int size = 0;
	for (size_t i = 0; i < MAX_ENTITIES; i++) {
		if (computer->ram->entities.entities[i].id[0] == '\0') {
			size = i;
			break;
		}
	}

	lua_createtable(lua, size, 0);

	for (size_t i = 0; i < size; i++) {
		lua_createtable(lua, 0, 5);

		lua_pushinteger(lua, computer->ram->entities.entities[i].x);
		lua_setfield(lua, -2, "x");

		lua_pushinteger(lua, computer->ram->entities.entities[i].y);
		lua_setfield(lua, -2, "y");

		lua_pushinteger(lua, computer->ram->entities.entities[i].sprite);
		lua_setfield(lua, -2, "sprite");

		lua_pushinteger(lua, computer->ram->entities.entities[i].w);
		lua_setfield(lua, -2, "w");

		lua_pushinteger(lua, computer->ram->entities.entities[i].h);
		lua_setfield(lua, -2, "h");

		lua_rawseti(lua, -2, i + 1);
	}

	return 1;
}

// TODO: use internal functions, but rn I can't be bothered yet
static int _lua_norm(lua_State *lua) {
	if (lua_gettop(lua) == 2) {
		float x = lua_tonumber(lua, 1);
		float y = lua_tonumber(lua, 2);

		// Get magnitude
		float magnitude = sqrtf(x * x + y * y);
		if (magnitude == 0.0f) {
			lua_pushnumber(lua, 0.0f);
			lua_pushnumber(lua, 0.0f);
			return 2;
		}

		x /= magnitude;
		y /= magnitude;

		lua_pushnumber(lua, x);
		lua_pushnumber(lua, y);
	} else {
		return luaL_error(lua, "Expected 2 arguments");
	}
	
	return 2;
}

static int _lua_sfx(lua_State *lua) {
	if (lua_gettop(lua) == 1) {
		int index = (int)lua_tonumber(lua, 1);
		api_sfx(get_global_computer()->ram, index);
	}
	
	return 0;
}

static int _lua_mget(lua_State *lua) {
	if (lua_gettop(lua) == 3) {
		int layer = (int)lua_tonumber(lua, 1);
		int x = (int)lua_tonumber(lua, 2);
		int y = (int)lua_tonumber(lua, 3);

		int index = api_mget(get_global_computer()->ram, layer, x, y);
		printf("mget index: %d\n", index);

		lua_pushinteger(lua, index);
	}

	return 1;
}

static int _lua_fmatch(lua_State *lua) {
	if (lua_gettop(lua) == 2) {
		int index = (int)lua_tonumber(lua, 1);
		const char *buffer = lua_tostring(lua, 2);

		if (buffer) {
			bool result = api_fmatch(get_global_computer()->ram, index, STR(buffer));
			printf("result: %d\n", result);
			lua_pushboolean(lua, result);
		} else {
			printf("no buffer\n");
		}
	}

	return 1;
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

static void _print_lua_error(ram_t *ram, string_t error_string) {
	term_printc(ram, STR("\nLua Error: "), COLOR_BLACK, COLOR_RED);
	term_printc(ram, error_string, COLOR_BLACK, COLOR_RED);
	term_putchar(ram, '\n', 0, 0);
}

int lua_init(computer_t *computer) {
	_lua = luaL_newstate();
	_open_safe_libs(_lua);

	// TODO: handle this in a loop based on the api metas
	// maybe not because then I have issues with circular dependency
	lua_register(_lua, api_metas[API_FUNC_PRINT].name, _lua_print);
	lua_register(_lua, api_metas[API_FUNC_CLS].name, _lua_cls);
	lua_register(_lua, api_metas[API_FUNC_SPR].name, _lua_spr);
	lua_register(_lua, api_metas[API_FUNC_CIRC].name, _lua_circ);
	lua_register(_lua, api_metas[API_FUNC_TICKS].name, _lua_ticks);
	lua_register(_lua, api_metas[API_FUNC_MAP].name, _lua_map);

	lua_register(_lua, api_metas[API_FUNC_KEY].name, _lua_key);
	lua_register(_lua, api_metas[API_FUNC_KEYP].name, _lua_keyp);
	
	lua_register(_lua, api_metas[API_FUNC_SAVE_TO_SLOT].name, _lua_save_to_slot);
	lua_register(_lua, api_metas[API_FUNC_LOAD_FROM_SLOT].name, _lua_load_from_slot);
	
	lua_register(_lua, api_metas[API_FUNC_GET_ENTS].name, _lua_get_ents); // TODO: make this a global variable instead of a function?
	lua_register(_lua, api_metas[API_FUNC_NORM].name, _lua_norm);
	lua_register(_lua, api_metas[API_FUNC_SFX].name, _lua_sfx);

	lua_register(_lua, api_metas[API_FUNC_MGET].name, _lua_mget);
	lua_register(_lua, api_metas[API_FUNC_FMATCH].name, _lua_fmatch);

	for (size_t i = 0; i < computer->active_files_amount; i++) {
		string_t file_string = computer->files[i].string;
		char *file_name = string_to_c_string(get_temp_allocator(), file_get_name(&computer->files[i]));

		// 'file ': 5 bytes
		// file index: 2 bytes
		// ': ' 2 bytes
		// file name: 10 bytes
		// null terminator: 1 byte
		// so 5 + 2 + 2 + 10 + 1 = 20 bytes
		char chunk_name[20] = {0};
		sprintf(chunk_name, "file %zu: %s", i, file_name);

		if (luaL_loadbuffer(_lua, file_string.data, file_string.len, chunk_name) != LUA_OK) {
			_print_lua_error(computer->ram, STR(lua_tostring(_lua, -1)));
			lua_pop(_lua, 1);
			return 1;
		} else if (lua_pcall(_lua, 0, LUA_MULTRET, 0) != LUA_OK) {
			_print_lua_error(computer->ram, STR(lua_tostring(_lua, -1)));
			lua_pop(_lua, 1);
			return 1;
		}
	}

	return 0;
}

int lua_call_init() {
	computer_t *computer = get_global_computer();

	lua_getglobal(_lua, "_init");

	if (lua_isfunction(_lua, -1)) {
		if (lua_pcall(_lua, 0, 0, 0) != LUA_OK) {
			_print_lua_error(computer->ram, STR(lua_tostring(_lua, -1)));
			lua_pop(_lua, 1);
			return 1;
		} 
	} else {
		lua_pop(_lua, 1);
		return 1;
	}

	return 0;
}

int lua_call_update() {
	computer_t *computer = get_global_computer();

	lua_getglobal(_lua, "_update");

	if (lua_isfunction(_lua, -1)) {
		if (lua_pcall(_lua, 0, 0, 0) != LUA_OK) {
			_print_lua_error(computer->ram, STR(lua_tostring(_lua, -1)));
			lua_pop(_lua, 1);
			return 1;
		} 
	} else {
		lua_pop(_lua, 1);
		return 1;
	}

	return 0;
}

int lua_call_draw() {
	computer_t *computer = get_global_computer();

	lua_getglobal(_lua, "_draw");

	if (lua_isfunction(_lua, -1)) {
		if (lua_pcall(_lua, 0, 0, 0) != LUA_OK) {
			_print_lua_error(computer->ram, STR(lua_tostring(_lua, -1)));
			lua_pop(_lua, 1);
			return 1;
		} 
	} else {
		lua_pop(_lua, 1);
		return 1;
	}
	
	return 0;
}

void lua_quit() {
	lua_settop(_lua, 0);
	lua_close(_lua);
	
	_lua = NULL;
}
