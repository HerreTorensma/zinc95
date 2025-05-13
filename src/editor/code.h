/*
Code editor

this header/source file should only be responsible for user input and drawing the editor,
the rest should be handled in backend/text_file
*/

#pragma once

#include "../computer.h"

void code_editor_init(computer_t *computer);

void code_editor_update(computer_t *computer);

void code_editor_draw(computer_t *computer);