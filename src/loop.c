#include <loop.h>

void loop(FILE_BUFFER *buffer)
{
	print_lines(buffer);
	int c;
	size_t *y = &buffer->y;
	size_t *x = &buffer->x;
	size_t *width = &buffer->lines->cols;
	size_t *height = &buffer->lines->rows;
	move(*y, *x);
	while (c = get_char())
	{
		if (c >= 32 && c <= 126)
		{
			add_char_to_line(c, buffer);
		}
		else if (c >= ARROW_UP && c <= ARROW_LEFT)
		{
			if (c == ARROW_UP)
			{
				if (*y > 0)
				        --(*y);
				else
					dec_lineno(buffer);
			}
			else if (c == ARROW_DOWN)
			{
				if (*y < *height)
					++(*y);
				else
					inc_lineno(buffer);
			}
		}
		else if (c == 32)
			break;
		print_lines(buffer);
		move(*y,*x);
	}
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
