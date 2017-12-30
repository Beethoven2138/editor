#include <tui.h>

int init_tui(FILE_BUFFER *buffer)
{
/*	WIN_DESC *win = buffer->win_desc;
	if (!win)
	return -1;*/

	//initscr();
	init_append_buf();
	term_raw_mode_on();
	term_erase_screen(1);

/*	buffer->lines->lines = LINES;
	buffer->lines->cols = COLS;*/
	//term_get_win_size(&buffer->lines->lines, &buffer->lines->cols);
	
	//keypad(stdscr, TRUE);
	//cbreak();
	//noecho();

	return EXIT_SUCCESS;
}

void end_tui(WIN_DESC *win)
{
	//nocbreak();
	//endwin();
	term_raw_mode_off();
	release_append_buf();
}

int get_char(void)
{
	/*int ch = getch();
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
	return ESCAPE_KEY;*/
	return term_get_input();
}

void print_lines(FILE_BUFFER *buffer)
{
	term_erase_screen(0);
	LINE_TABLE *l_table = buffer->lines;
	memset(buffer->rendered, 0, l_table->rows * l_table->cols);
	get_rendered_output(buffer->rendered, buffer);
	size_t start = /*l_table->start_lineno+*/l_table->used_above/*-1*/;

	for (size_t i = 0; i < l_table->used; ++i)
	{
		//move(i+1, 0);
		term_cursor_pos(i+1, 0, 0);
		add_append_buf(buffer->rendered+l_table->cols*i, l_table->lines[start + i].length);
	}

	term_print_screen(1);
}


void move(int y, int x)
{
	term_cursor_pos(y, x, 1);
}
