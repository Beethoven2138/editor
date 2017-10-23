#ifndef LOOP_H
#define LOOP_H

#include <buffer.h>
#include <common.h>
#include <tui.h>

#define ERROR 0x80000000

#define F_KEY 0x40000000

#define HOME_KEY 0x20000000
#define END_KEY 0x10000000

#define BACKSPACE_KEY 0x8000000
#define DELETE_KEY 0x4000000
#define TAB_KEY 0x2000000
#define ENTER_KEY 0x1000000
#define ESCAPE_KEY 0x800000
#define ARROW 0x400000

#define NORMAL 0x200000

void loop(FILE_BUFFER *buffer);

#endif
