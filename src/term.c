#include <term.h>

static struct termios old_term;
APPEND_BUF a_buf;

void init_append_buf(void)
{
        a_buf.buf = (char*)malloc(10);
	a_buf.size = 10;
	a_buf.len = 0;
}

void release_append_buf(void)
{
	free(a_buf.buf);
	a_buf.len = 0;
	a_buf.size = 0;
}

void add_append_buf(const char *s, size_t count)
{
	if (count > a_buf.size - a_buf.len)
	{
	        a_buf.size = a_buf.len + count;
		a_buf.buf = (char*)(a_buf.buf, a_buf.size);
	}
	for (size_t i = 0; i < count; ++i)
		*(a_buf.buf + a_buf.len++) = s[i];
}

void flush_append_buf(void)
{
	write(STDOUT_FILENO, a_buf.buf, a_buf.len);
	a_buf.len = 0;
}

int term_raw_mode_on(void)
{
	struct termios term;
	atexit(term_raw_mode_off);
	if (tcgetattr(STDIN_FILENO, &term) == -1)
		return -1;

	memcpy(&old_term, &term, sizeof(struct termios));

	term.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
	term.c_oflag &= ~(OPOST);/*no post processing*/
	term.c_cflag |= (CS8);
	term.c_lflag &= ~(ECHO |/*non-canonical input*/ICANON | IEXTEN | ISIG);
	term.c_cc[VMIN] = 0;
	term.c_cc[VTIME] = 1;
	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &term) == -1)
		return -1;
	return 0;
}

// Low level terminal handling
void term_raw_mode_off(void)
{
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &old_term);
	//write(STDIN_FILENO, "\033[2J\033[H\033[?1049l", 15);
}

void term_reset(char flags)
{
	char tmp[2];
	tmp[0] = ESC; tmp[1] = 'C';

	add_append_buf(tmp, 2);
	if (flags & FLUSH)
		flush_append_buf();
}

void term_line_wrap_on(char flags)
{
	char tmp[4];
	tmp[0] = ESC; tmp[1] = '[';tmp[2] = '7'; tmp[3] = 'h';
	add_append_buf(tmp, 4);
	if (flags & FLUSH)
		flush_append_buf();
}

void term_line_wrap_off(char flags)
{
	char tmp[4];
	tmp[0] = ESC; tmp[1] = '[';tmp[2] = '7'; tmp[3] = 'l';
        add_append_buf(tmp, 4);
	if (flags & FLUSH)
		flush_append_buf();
}

void term_cursor_up(int count, char flags)
{
        add_append_buf("\x1b[", 2);
	if (count < 10)
	{
		char tmp[2];
		tmp[0] = count + '0'; tmp[1] = 'A';
		add_append_buf(tmp, 2);
	}
	else
	{
		size_t digits = digit_count(count);
		char tmp[digits + 1];
		sprintf(tmp, "%d", count);
		tmp[digits] = 'A';
		add_append_buf(tmp, digits+1);
	}
	if (flags & FLUSH)
		flush_append_buf();
}

void term_cursor_down(int count, char flags)
{
	add_append_buf("\033[", 2);
	if (count < 10)
	{
		char tmp[2];
		tmp[0] = count + '0'; tmp[1] = 'B';
		add_append_buf(tmp, 2);
	}
	else
	{
		size_t digits = digit_count(count);
		char tmp[digits + 1];
		sprintf(tmp, "%d", count);
		tmp[digits] = 'B';
		add_append_buf(tmp, digits+1);
	}
	if (flags & FLUSH)
		flush_append_buf();
}

void term_cursor_forward(int count, char flags)
{
	/*char tmp[4];
	tmp[0] = ESC; tmp[1] = '['; tmp[2] = count + '0'; tmp[3] = 'C';
        add_append_buf(tmp, 4);*/

	add_append_buf("\033[", 2);
	if (count < 10)
	{
		char tmp[2];
		tmp[0] = count + '0'; tmp[1] = 'C';
		add_append_buf(tmp, 2);
	}
	else
	{
		size_t digits = digit_count(count);
		char tmp[digits + 1];
		sprintf(tmp, "%d", count);
		tmp[digits] = 'C';
		add_append_buf(tmp, digits+1);
	}
	if (flags & FLUSH)
		flush_append_buf();
}

void term_cursor_backward(int count, char flags)
{
	add_append_buf("\033[", 2);
	if (count < 10)
	{
		char tmp[2];
		tmp[0] = count + '0'; tmp[1] = 'D';
		add_append_buf(tmp, 2);
	}
	else
	{
		size_t digits = digit_count(count);
		char tmp[digits + 1];
		sprintf(tmp, "%d", count);
		tmp[digits] = 'D';
		add_append_buf(tmp, digits+1);
	}
	if (flags & FLUSH)
		flush_append_buf();
}

void term_clear_screen(char flags)
{
	char tmp[3];
	tmp[0] = ESC; tmp[1] = '['; tmp[2] = '2';
	add_append_buf(tmp, 3);
	if (flags & FLUSH)
		flush_append_buf();
}

void term_cursor_pos(int y, int x, char flags)
{
	add_append_buf("\033[", 2);
	size_t digits = digit_count(y);
	char tmp[digits + 1];
	sprintf(tmp, "%d", y);
	tmp[digits] = ';';
	add_append_buf(tmp, digits+1);

	digits = digit_count(x);
	char tmp1[digits + 1];
	sprintf(tmp1, "%d", x);
	tmp1[digits] = 'H';
	add_append_buf(tmp1, digits+1);
	if (flags && FLUSH)
		flush_append_buf();
}

void term_erase_screen(char flags)
{
	add_append_buf("\033[2J", 4);
	if (flags & FLUSH)
		flush_append_buf();
}

void term_print_screen(char flags)
{
	add_append_buf("hello", 5);
	add_append_buf("\033[i", 3);
	if (flags & FLUSH)
		flush_append_buf();
}

//ssize_t read(int fd, void *buf, size_t count);

char term_get_char(void)
{
	char ret;
	read(STDIN_FILENO, &ret, 1);
	return ret;
}

int term_get_input(void)
{
	int nread;
	char c, seq[3];
	while ((nread = read(fd,&c,1)) == 0);
	if (nread == -1) exit(1);

	while(1)
	{
		switch(c)
		{
		case ESC:    /* escape sequence */
			/* If this is just an ESC, we'll timeout here. */
			if (read(fd,seq,1) == 0)
				return ESC;
			if (read(fd,seq+1,1) == 0)
				return ESC;

			/* ESC [ sequences. */
			if (seq[0] == '[')
			{
				if (seq[1] >= '0' && seq[1] <= '9')
				{
					/* Extended escape, read additional byte. */
					if (read(fd,seq+2,1) == 0) return ESC;
					if (seq[2] == '~')
					{
						switch(seq[1])
						{
						case '3': return DEL_KEY;
						case '5': return PAGE_UP;
						case '6': return PAGE_DOWN;
						}
					}
				} else
				{
					switch(seq[1])
					{
					case 'A': return ARROW_UP;
					case 'B': return ARROW_DOWN;
					case 'C': return ARROW_RIGHT;
					case 'D': return ARROW_LEFT;
					case 'H': return HOME_KEY;
					case 'F': return END_KEY;
					}
				}
			}

			/* ESC O sequences. */
			else if (seq[0] == 'O')
			{
				switch(seq[1])
				{
				case 'H': return HOME_KEY;
				case 'F': return END_KEY;
				}
			}
			break;
		default:
			return c;
		}
	}
}

int term_get_win_size(size_t *rows, size_t *cols)
{
	struct winsize win;
	if (ioctl(1, TIOCGWINSZ, &win) == -1 || win.ws_col == 0)
		return -1;
	*cols = win.ws_col;
	*row = win.ws_row;
}

int get_cursor_pos(size_t *y, size_t *x)
{
	
        char buf[32];
        unsigned int i = 0;

        // report cursor location
        if (write(ofd, "\x1b[6n", 4) != 4) return -1;

        // read the response: ESC [ rows; columns R
        while(i < sizeof(buf)-1)
	{
		if(read(ifd, buf+i, 1) != 1)
			break;
		if(buf[i] == 'R')
			break;
		i++;
        }
        buf[i] = '\0';

        // parse it
        if (buf[0] != ESC || buf[1] != '[')
		return -1;
        if (sscanf(buf+2,"%d;%d",rows,columns) != 2)
		return -1;
	return 0;
}
