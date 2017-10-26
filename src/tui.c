#include <tui.h>

int init_tui(FILE_BUFFER *buffer)
{
	WIN_DESC *win = buffer->win_desc;
	if (!win)
		return -1;

	initscr();

	buffer->lines->lines = LINES;
	buffer->lines->cols = COLS;

	keypad(stdscr, TRUE);
	cbreak();
	noecho();

	return EXIT_SUCCESS;
}

void end_tui(WIN_DESC *win)
{
	nocbreak();
	endwin();
}

int get_char(void)
{
	int ch = getch();
	if (ch == KEY_DOWN || ch == KEY_UP || ch == KEY_LEFT || ch == KEY_RIGHT)
		return ARROW | ch;
	if (ch >= 32 && ch <= 126)
		return NORMAL | ch;
	if (ch >= KEY_F(2) && ch <= KEY_F(7))
		return F_KEY | ch;
	if (ch == KEY_HOME)
		return HOME_KEY;
	if (ch == KEY_END)
		return END_KEY;
	if (ch == KEY_BACKSPACE)
		return BACKSPACE_KEY;
	if (ch == KEY_DL)
		return DELETE_KEY;
	if (ch == 9)
		return TAB_KEY;
	if (ch == 10 || ch == KEY_ENTER)
		return ENTER_KEY;
	if (ch == 0x1B)
		return ESCAPE_KEY;
}

void print_lines(LINE_TABLE *l_table)
{
	/*for (size_t y = 0; y < l_table->used; ++y)
	{
	        mvprintw(y, 0, l_table->line[y].buf);
		}*/
	size_t abs = 0;
	for (size_t y = 0; y < l_table->used; ++y)
	{
		move(y, 0);
		for (size_t i = 0; i < l_table->line[y].len; ++i)
		{
			if (abs + l_table->span1 == l_table->gap)
				abs += l_table->gap_len;
			addch(*(l_table->span1 + abs));
			++abs;
		}
	}
}

void redraw_line(size_t line, LINE_TABLE *l_table)
{
	size_t len = l_table->line[line].len;
	size_t off = line_off(line, l_table);
	if (off >= l_table->gap - l_table->span1)
		off += l_table->gap_len;
	for (size_t i = 0; i < l_table->cols; i++)
	{
		if (i < len)
		{
			if (off == l_table->gap - l_table->span1)
				off += l_table->gap_len;
			mvaddch(line, i, l_table->span1[off++]);
		}
		else
		{
			mvaddch(line, i, ' ');
		}
	}
}
