#include "input.h"

#include "../computer.h"

static int _key_timers[KEY_COUNT] = {0};

void input_core_update() {
	for (size_t i = 0; i < KEY_COUNT; i++) {
		if (_key_timers[i] > 0) {
			_key_timers[i]--;
		}
	}
}

bool input_key_pressed_or_long_pressed(zinc_key_t key) {
	if (input_key_pressed(key)) {
		_key_timers[key] = LONG_PRESS_TRIGGER_TIME;
		return true;
	}
	if (!input_key_held(key)) {
		_key_timers[key] = 0;
	}
	if (_key_timers[key] == 1) {
		_key_timers[key] = LONG_PRESS_REPEAT_TIME;
		return true;
	}
	return false;
}

char input_get_as_char() {
	if (input_key_pressed_or_long_pressed(KEY_SPACE)) {
		return ' ';
	}

	if (input_key_pressed_or_long_pressed(KEY_RETURN) || input_key_pressed_or_long_pressed(KEY_NUMENTER)) {
		return '\n';
	}

	if (input_key_pressed_or_long_pressed(KEY_BACKSPACE)) {
		return '\b';
	}

	// Letters
	for (int i = KEY_A; i <= KEY_Z; i++) {
		if (input_key_pressed_or_long_pressed(i)) {
			if (input_key_held(KEY_LSHIFT) || input_key_held(KEY_RSHIFT)) {
				return 'A' + (i - KEY_A);
			} else {
				return 'a' + (i - KEY_A);
			}
		}
	}

	// Number row
	if (input_key_held(KEY_LSHIFT) || input_key_held(KEY_RSHIFT)) {
		if (input_key_pressed_or_long_pressed(KEY_1))
			return '!';

		if (input_key_pressed_or_long_pressed(KEY_2))
			return '@';
		
		if (input_key_pressed_or_long_pressed(KEY_3))
			return '#';

		if (input_key_pressed_or_long_pressed(KEY_4))
			return '$';

		if (input_key_pressed_or_long_pressed(KEY_5))
			return '%';

		if (input_key_pressed_or_long_pressed(KEY_6))
			return '^';

		if (input_key_pressed_or_long_pressed(KEY_7))
			return '&';

		if (input_key_pressed_or_long_pressed(KEY_8))
			return '*';

		if (input_key_pressed_or_long_pressed(KEY_9))
			return '(';

		if (input_key_pressed_or_long_pressed(KEY_0))
			return ')';
	} else {
		for (int i = 0; i <= 9; i++) {
			if (input_key_pressed_or_long_pressed(KEY_0 + i) || input_key_pressed_or_long_pressed(KEY_NUM0 + i)) {
				return '0' + i;
			}
		}
	}

	// Other characters
	if (input_key_held(KEY_LSHIFT) || input_key_held(KEY_RSHIFT)) {
		if (input_key_pressed_or_long_pressed(KEY_MINUS))
			return '_';

		if (input_key_pressed_or_long_pressed(KEY_EQUALS))
			return '+';

		if (input_key_pressed_or_long_pressed(KEY_LEFTBRACKET))
			return '{';

		if (input_key_pressed_or_long_pressed(KEY_RIGHTBRACKET))
			return '}';

		if (input_key_pressed_or_long_pressed(KEY_BACKSLASH))
			return '|';

		if (input_key_pressed_or_long_pressed(KEY_SEMICOLON))
			return ':';

		if (input_key_pressed_or_long_pressed(KEY_APOSTROPHE))
			return '\"';

		if (input_key_pressed_or_long_pressed(KEY_COMMA))
			return '<';

		if (input_key_pressed_or_long_pressed(KEY_PERIOD))
			return '>';

		if (input_key_pressed_or_long_pressed(KEY_SLASH))
			return '?';

		if (input_key_pressed_or_long_pressed(KEY_GRAVE))
			return '~';

	} else {
		if (input_key_pressed_or_long_pressed(KEY_MINUS) || input_key_pressed_or_long_pressed(KEY_NUMMINUS))
			return '-';

		if (input_key_pressed_or_long_pressed(KEY_EQUALS))
			return '=';

		if (input_key_pressed_or_long_pressed(KEY_LEFTBRACKET))
			return '[';

		if (input_key_pressed_or_long_pressed(KEY_RIGHTBRACKET))
			return ']';

		if (input_key_pressed_or_long_pressed(KEY_BACKSLASH))
			return '\\';

		if (input_key_pressed_or_long_pressed(KEY_SEMICOLON))
			return ';';

		if (input_key_pressed_or_long_pressed(KEY_APOSTROPHE))
			return '\'';

		if (input_key_pressed_or_long_pressed(KEY_COMMA))
			return ',';

		if (input_key_pressed_or_long_pressed(KEY_PERIOD) || input_key_pressed_or_long_pressed(KEY_NUMPERIOD))
			return '.';

		if (input_key_pressed_or_long_pressed(KEY_SLASH) || input_key_pressed_or_long_pressed(KEY_NUMDIVIDE))
			return '/';

		if (input_key_pressed_or_long_pressed(KEY_GRAVE))
			return '`';

		}
	
	// Some numpad stuff
	if (input_key_pressed_or_long_pressed(KEY_NUMMULTIPLY))
		return '*';

	if (input_key_pressed_or_long_pressed(KEY_NUMPLUS))
		return '+';

	if (input_key_pressed_or_long_pressed(KEY_TAB)) {
		return '\t';
	}

	return '\0';
}

static uint32_t _get_current_modifiers() {
	uint32_t modifiers = 0;
	
	if (input_key_held(KEY_LCTRL) || input_key_held(KEY_RCTRL)) {
		modifiers |= MODIFIER_CTRL;
	}

	if (input_key_held(KEY_LALT) || input_key_held(KEY_RALT)) {
		modifiers |= MODIFIER_ALT;
	}

	if (input_key_held(KEY_LSHIFT) || input_key_held(KEY_RSHIFT)) {
		modifiers |= MODIFIER_SHIFT;
	}

	return modifiers;
}

// Checks if only that keybind is active
bool is_keybind_pressed(keybind_t keybind) {
	if (!input_key_pressed_or_long_pressed(keybind.key)) {
		return false;
	}

	return keybind.modifiers == _get_current_modifiers();
}

// TODO: unused and untested currently, maybe ill removed
bool is_modifier_held(modifier_t mod) {
	return _get_current_modifiers() & mod;
}
