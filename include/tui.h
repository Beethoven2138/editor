#ifndef TUI_H
#define TUI_H

//#include <ncurses.h>
#include <stdlib.h>

#include <term.h>
#include <buffer.h>
#include <loop.h>

typedef struct _file_buffer FILE_BUFFER;

typedef struct _win_desc
{
	char *name;
	size_t name_len;

	/*for overall window*/
	size_t x;
	size_t y;

	size_t width;
	size_t height;

	//WINDOW *win;
} WIN_DESC;

int init_tui(FILE_BUFFER *win);
void end_tui(WIN_DESC *win);

typedef struct _line_table LINE_TABLE;

void print_lines(FILE_BUFFER *buffer);
void redraw_line(size_t line, LINE_TABLE *l_table);

int get_char(void);

void move(int y, int x);

//TEXT *get_input(TEXT *dest, FILE_BUFFER *buffer);

#endif
