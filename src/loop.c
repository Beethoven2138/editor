#include <loop.h>

void loop(FILE_BUFFER *buffer)
{
	int c;
	print_lines(buffer->lines);
	size_t y = 0;
	size_t x = 0;
	while (c = get_char())
	{
		move(y,x);
		if (c & NORMAL)
		{
		        line_gap_add((char)c, &y, &x, buffer);
		}
	        else if (c & F_KEY)
		{

		}
		else if (c & HOME_KEY)
		{

		}
		else if (c & END_KEY)
		{

		}
		else if (c & BACKSPACE_KEY)
		{

		}
		else if (c & DELETE_KEY)
		{

		}
		else if (c & TAB_KEY)
		{

		}
		else if (c & ENTER_KEY)
		{
			break;
		}
		else if (c & ESCAPE_KEY)
		{
			break;
		}
		else if (c & ARROW)
		{
			switch (c)
			{
			case ARROW | KEY_DOWN:
			{
				goto_next_line(&y, &x, buffer);
				break;
			}
			case ARROW | KEY_UP:
			{
				goto_prev_line(&y, &x, buffer);
				break;
			}
			case ARROW | KEY_LEFT:
			{
				dec_line_gap(&y, &x, buffer);
				break;
			}
			case ARROW | KEY_RIGHT:
			{
				inc_line_gap(&y, &x, buffer);
				break;
			}
			default:
				assert(0);
			}
		}
		print_lines(buffer->lines);
		move(y,x);
	}
}
