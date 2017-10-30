#ifndef TERM_H
#define TERM_H

#include <common.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <termios.h>

#define ESC 0x1B

typedef struct append_buf
{
	char *buf;
	size_t len;
	size_t size;
} APPEND_BUF;

void init_append_buf(void);
void release_append_buf(void);
void add_append_buf(const char *s, size_t count);
void flush_append_buf(void);

//ssize_t write(int fd, const void *buf, size_t nbytes);

static inline size_t digit_count(size_t num)
{
	size_t ret = 0;
	while (num != 0)
	{
		num /= 10;
		++ret;
	}
	return ret;
}

#define FLUSH 1

void term_clear_screen(char flags);
int term_raw_mode_on(void);
void term_raw_mode_off(void);
void term_reset(char flags);
void term_line_wrap_on(char flags);
void term_line_wrap_off(char flags);
void term_cursor_up(int count, char flags);
void term_cursor_down(int count, char flags);
void term_cursor_forward(int count, char flags);
void term_cursor_backward(int count, char flags);

void term_cursor_pos(int y, int x, char flags);

void term_erase_screen(char flags);
void term_print_screen(char flags);


#endif
