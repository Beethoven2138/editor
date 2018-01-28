#include <loop.h>

void test(void)
{
	int a = 5;
}

void loop(FILE_BUFFER *buffer)
{
	print_lines(buffer);
	int c;
	size_t *y = &buffer->y;
	size_t *x = &buffer->x;
	*y = 1;
	*x = 1;
	size_t *width = &buffer->lines->cols;
	size_t *height = &buffer->lines->rows;
	move(*y, *x);
	while (c = get_char())
	{
		if ((c >= 32 && c <= 126) || c == 9 || c == 10)
		{
			add_char_to_line(c, buffer);
		}
		else if (c >= F1_KEY && c <= F12_KEY)
		{
			if (c == F5_KEY)
				save_buffer(buffer);
		}
		else if (c >= ARROW_UP && c <= ARROW_LEFT)
		{
			if (c == ARROW_UP)
			{
				if (*y > 1)
				        --(*y);
				else
					dec_lineno(buffer);
				LINE *current = CURRENT_LINE(buffer->lines, *y);
				if (*x >= current->length)
					*x = current->length+1;
			}
			else if (c == ARROW_DOWN)
			{
				if (*y < buffer->lines->used)
					++(*y);
				else
					inc_lineno(buffer);
				LINE *current = CURRENT_LINE(buffer->lines, *y);
				if (*x >= current->length)
					*x = current->length+1;
			}
			else if (c == ARROW_LEFT)
			{
				--(*x);
				if (*x == 0)
				{
					if (*y > 1 || buffer->lines->start_lineno > 1 ||
					    buffer->lines->used_above > 0)
					{
						if (*y > 2)
							--(*y);
						else
							dec_lineno(buffer);
						LINE *current = CURRENT_LINE(buffer->lines, *y);
						*x = current->length+1;
					}
					else ++(*x);
				}
			}
			else if (c == ARROW_RIGHT)
			{
				if (buffer->rendered[(*y)*(*width)+(*x)-1] != 9)
					++(*x);
				else
					*x += buffer->settings.tab_length;
				if (CURRENT_LINE(buffer->lines, *y)->length+1 < *x)
				{
					if (++(*y) >= *height)
					{
						if (inc_lineno(buffer) == -1)
						{
							*y = *height;
						}
						else
						{
							//--(*y);
							*x = 1;
						}
					}
					else
					{
						*x = 1;
					}
				}
			}
		}
		else if (c == ESC_KEY)
			break;
		else if (c == END_KEY)
		{
			test();
			//int a = 4;
		}
		print_lines(buffer);
		move(*y,*x);
	}
	term_erase_screen(0);
	move(1,1);
/*	while (c = get_char())
	{
		move(y,x);
	        if (c >= F2_KEY && c <= F12_KEY)
		{

		}
		else if (c == HOME_KEY)
		{

		}
		else if (c == END_KEY)
		{

		}
		else if (c == BACKSPACE_KEY)
		{
			line_delete_char(&y, &x, buffer);
		}
		else if (c == DEL_KEY)
		{

		}
		else if (c == TAB_KEY)
		{

		}
		else if (c == ENTER_KEY)
		{
			break;
		}
		else if (c == ESCAPE_KEY)
		{
			break;
		}
		else if (c >= ARROW_UP && c <= ARROW_LEFT)
		{
			switch (c)
			{
			case ARROW_DOWN:
			{
				goto_next_line(&y, &x, buffer);
				break;
			}
			case ARROW_UP:
			{
				goto_prev_line(&y, &x, buffer);
				break;
			}
			case ARROW_LEFT:
			{
				dec_line_gap(&y, &x, buffer);
				break;
			}
			case ARROW_RIGHT:
			{
				inc_line_gap(&y, &x, buffer);
				break;
			}
			default:
				assert(0);
			}
		}
		else
		{
			line_gap_add((char)c, &y, &x, buffer);
		}
		print_lines(buffer->lines);
		move(y,x);
		}*/
}
