#ifndef BUFFER_H
#define BUFFER_H

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>

#include <common.h>
#include <tui.h>

#include <red_black_tree.h>

#define FILE_SIZE(a) (a->status->st_size)
#define FILE_NAME(a) (a->name)
#define FILE_NAME_LEN(a) (a->name_len)

#define FILE_BUF(a) (a->file_buf)
#define ADD_BUF(a) (a->add_buf)
#define GET_BUFFER(a, b) (((b & IN_FILE) == 0) ? a->add_buf->buffer : a->file_buf)

#define FILE_PIECE_DESC(a) (a->piece_desc)

#define SIZE(a) (a->buffer_size)

#define GET_NEXT_PIECE(a, b) ((PIECE*)TreeSuccessor(b->piece_desc->tree, a->node)->info)
#define GET_PREV_PIECE(a, b) ((PIECE*)TreePredecessor(b->piece_desc->tree, a->node)->info)
#define GET_FIRST_PIECE(a) ((PIECE*)TreeMin(a->piece_desc->tree)->info)
#define GET_LAST_PIECE(a) ((PIECE*)TreeMax(a->piece_desc->tree)->info)

typedef struct _text
{
	char text[NR_TEXT];
	size_t len;
} TEXT;

#define INFO 1
#define KEY 2
#define ABS_OFF 4 /*if checked, size_left holds absolute offset*/
#define IN_FILE 8
#define IN_ADD 0

#define P_INFILE(a) ((a->flags & IN_FILE) != 0)
#define P_INADD(a) ((a->flags & IN_FILE) == 0)

typedef struct rb_red_blk_node rb_red_blk_node;
typedef struct rb_red_blk_tree rb_red_blk_tree;

typedef struct _file_buffer FILE_BUFFER;

#define CURRENT(a) (a->piece_desc->cache[0].piece)

#define GAP_OFF(a) (a->gap - a->span1)

typedef struct _piece
{
	char flags;
	size_t offset; //Offset into file or add span

	size_t size_left;
	size_t size_right;
	size_t size;

	rb_red_blk_node *node;

	FILE_BUFFER *file;
} PIECE;

typedef struct _p_cache
{
	PIECE *piece;
	size_t offset;
} P_CACHE;

#define CACHE_CURR 0
#define CACHE_PREV 1
#define CACHE_NEXT 2

#define CURR_PIECE(a) (a->cache[0])
#define PREV_PIECE(a) (a->cache[CACHE_PREV])
#define NEXT_PIECE(a) (a->cache[CACHE_NEXT])


typedef struct _piece_desc
{
	rb_red_blk_tree *tree;
	//PIECE *current;
	P_CACHE cache[3];
} PIECE_DESC;

//The size of the page is defined in common.h (normally 4096 bytes)
typedef struct _page
{
	char *page;
	//size_t offset;
	struct _page *prev;
	struct _page *next;
} PAGE;

typedef struct _buffer
{
	//Linked list of pages
	/*PAGE *head;
	  PAGE *tail;*/
	char *buffer;
	size_t offset; //Offset of next free character into the buffer
	size_t size;   //Total size of the buffer
} BUFFER;

typedef struct _line
{
	//char buf[NR_COLS+1];/*only +1 for null-termination*/
	//size_t len;
	//bool new_line; /* Is there a new line character at the end?*/
	PIECE *start_piece;
	size_t start_offset;
	PIECE *end_piece;
	size_t end_offset;
	char flags;
	size_t length;
	size_t lineno;
	size_t start_abs_offset;
} LINE;

typedef struct _line_table
{
	LINE *lines;
	size_t lines_count;
	size_t rows;
	size_t cols;
	size_t used; /*number of lines in use.
		       will normally be == NR_LINES
		       unless at bottom of file.
		      */
	size_t used_above;
	size_t used_bellow;
	size_t start_lineno;
	size_t total_lines;
} LINE_TABLE;

typedef struct _win_desc WIN_DESC;

typedef struct _file_buffer
{
	char *file_buf;
	BUFFER *add_buf;  //Append only. Contains the new items added

	BUFFER *user_cache; /*Append only. Contains the text that the
			      user types that hasn't been added to the
			      piece table yet*/
	struct stat *status; //Status of the file
	size_t buffer_size;
	char *name; //File name
	size_t name_len; //Length of the file name
	PIECE_DESC *piece_desc;
	WIN_DESC *win_desc;
	LINE_TABLE *lines;
	/*for text*/
	size_t x;
	size_t y;

	char *rendered;
} FILE_BUFFER;

FILE_BUFFER* init_buffer(char *input_file);
void release_buffer(FILE_BUFFER *buffer);

int save_buffer(FILE_BUFFER *buffer, const char *file_name);

size_t add_buffer_append(const char *new_item, size_t len, FILE_BUFFER *buffer);

int insert_item(const char *new_item, size_t len, size_t abs_size_left, FILE_BUFFER *buffer);
int delete_item(size_t offset, size_t len/*amount to delete*/, FILE_BUFFER *buffer);

static PIECE* add_piece(size_t abs_offset, size_t size_right,
			size_t size, bool buf, size_t off/*into buffer*/, FILE_BUFFER *buffer);
static void delete_piece(PIECE *piece, FILE_BUFFER *buffer);

void print_buffer(FILE_BUFFER *buffer);

size_t piece_offset(const PIECE *key);

PIECE *find_containing_piece(size_t offset, FILE_BUFFER *buffer, size_t *ret_off);

int piece_compare(const void *a, const void *b);
void info_destroy(void *info);
void key_destroy(void *key);

void fix_sizes(PIECE *piece, int change);

size_t abs_off_to_size_left(PIECE *piece);

ssize_t buffer_read(char *dest, size_t offset, size_t count, FILE_BUFFER *buffer);

void fill_lines(FILE_BUFFER *buffer, size_t lineno);

PIECE *piece_insert_left(size_t size, char flags, size_t span_off, FILE_BUFFER *buffer);
PIECE *piece_insert_right(size_t size, char flags, size_t span_off, FILE_BUFFER *buffer);

void change_current(PIECE *new_current, FILE_BUFFER *buffer);

static P_CACHE *find_in_cache_off(size_t offset, FILE_BUFFER *buffer)
{
	P_CACHE *cache = buffer->piece_desc->cache;
	for (size_t i = 0; i < CACHE_SIZE; ++i)
	{
		if (cache[i].piece == 0)
			return NULL;
		if (cache[i].offset <= offset &&
		    cache[i].offset + cache[i].piece->size > offset)
			return (cache + i);
	}
	return NULL;
}

static size_t find_offset_in_cache(const PIECE *piece, FILE_BUFFER *buffer)
{
	P_CACHE *cache = buffer->piece_desc->cache;
	if (piece == NULL)
		return MAGIC;
	for (size_t i = 0; i < CACHE_SIZE; ++i)
	{
		if (cache[i].piece == NULL)
			return MAGIC;
		if (cache[i].piece == piece)
			return (cache[i].offset);
	}
        return MAGIC;
}

void line_gap_add(const char new_item, size_t *y_pos, size_t *x_pos, FILE_BUFFER *buffer);
void inc_line_gap(size_t *y, size_t *x, FILE_BUFFER *buffer);
void dec_line_gap(size_t *y, size_t *x, FILE_BUFFER *buffer);
void goto_next_line(size_t *y, size_t *x, FILE_BUFFER *buffer);
void goto_prev_line(size_t *y, size_t *x, FILE_BUFFER *buffer);
void move_line_gap_fwd(size_t count, FILE_BUFFER *buffer);
void move_line_gap_back(size_t count, FILE_BUFFER *buffer);

void line_delete_char(size_t *y, size_t *x, FILE_BUFFER *buffer);

static inline char piece_read_c(PIECE *piece, size_t offset, FILE_BUFFER *buffer)
{
	return GET_BUFFER(buffer, piece->flags)[offset];
}

static inline size_t piece_read(char *dest, PIECE *piece, size_t offset, size_t count, FILE_BUFFER *buffer)
{
	char *buf = GET_BUFFER(buffer, piece->flags);
	memcpy(dest, buf + offset, count);
	return count;
}

void get_rendered_output(char *dest, FILE_BUFFER *buffer);

void inc_lineno(FILE_BUFFER *buffer);
void dec_lineno(FILE_BUFFER *buffer);


size_t get_line_count(FILE_BUFFER *buffer);

size_t fill_lines_offset(FILE_BUFFER *buffer, size_t lineno, size_t table_offset, size_t count);


void add_char_to_line(char c, FILE_BUFFER *buffer);


#define CURRENT_LINE(a) (a->y + a->lines->start_lineno + a->lines->used_above)
#define CURSOR_X(a) (a->x)
#define CURSOR_Y(a) (a->y)
#define LINE_INDEX(a, y) (a->used_above + y)
#define WIDTH(a) (a->cols)
#define HEIGHT(a) (a->rows)
#define TOTAL_LINES(a) (a->lines_count)

#endif
