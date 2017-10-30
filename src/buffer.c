/*
    editor is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    editor is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    You should have received a copy of the GNU General Public License
    along with editor.  If not, see <http://www.gnu.org/licenses/>.
*/


/*AUTHOR: SAXON SUPPLE*/


#include <buffer.h>

FILE_BUFFER* init_buffer(char *input_file)
{
	FILE *in_filp = fopen(input_file, "r");
	if (!in_filp)
		return NULL;

	FILE_BUFFER *ret = (FILE_BUFFER*)malloc(sizeof(FILE_BUFFER));

	ret->name_len = strlen(input_file);
	ret->name = (char*)malloc(ret->name_len);
	strncpy(ret->name, input_file, ret->name_len);

	ret->status = (struct stat*)malloc(sizeof(struct stat));
	if (stat(input_file, ret->status) == -1)
		return NULL;

	ret->buffer_size = ret->status->st_size;

	ret->file_buf = mmap(NULL, ret->status->st_size,
			     PROT_READ, MAP_PRIVATE/* | MAP_HUGETLB*/, fileno(in_filp), 0);

	ret->add_buf = (BUFFER*)malloc(sizeof(BUFFER));
	ret->add_buf->size = PAGE_SIZE;
	ret->add_buf->offset = 0;
	//ret->add_buf->head = ret->add_buf->tail = (PAGE*)malloc(sizeof(PAGE));
	//memset(ret->add_buf->head, 0, sizeof(PAGE));
	//posix_memalign((void**)&ret->add_buf->head->page, PAGE_SIZE, PAGE_SIZE);
	ret->add_buf->buffer = (char*)malloc(PAGE_SIZE);

	ret->piece_desc = (PIECE_DESC*)malloc(sizeof(PIECE_DESC));
	ret->piece_desc->tree = RBTreeCreate(piece_compare, key_destroy, info_destroy);
	memset(ret->piece_desc->cache, 0, 3 * sizeof(P_CACHE));

	PIECE *first = (PIECE*)malloc(sizeof(PIECE));
	first->file = ret;
	first->flags = IN_FILE | INFO | KEY;
	first->offset = 0;
	first->size_left = 0;
	first->size_right = 0;
	first->size = ret->status->st_size;
	first->node = RBTreeInsert(ret->piece_desc->tree, (void*)first, (void*)first);

	change_current(first, ret);

	ret->win_desc = (WIN_DESC*)malloc(sizeof(WIN_DESC));

	ret->lines = (LINE_TABLE*)malloc(sizeof(LINE_TABLE));

	memset(ret->lines->line, 0, sizeof(LINE) * NR_LINES);

	LINE_TABLE *table = ret->lines;
	table->span1 = (char*)malloc(NR_LINES * NR_COLS + GAP_SIZE);
	table->span1_len = 0;
	table->gap = table->span1;
	table->gap_len = GAP_SIZE;
	table->span2 = table->span1 + GAP_SIZE;
	table->span2_len = ret->buffer_size;
	table->gap_size = GAP_SIZE;
	/*ret->user_cache = (BUFFER*)malloc(sizeof(BUFFER));
	ret->user_cache->buffer = (char*)malloc(10);
	ret->user_cache->offset = 0;
	ret->user_cache->size = 10;*/


	init_tui(ret);

	fill_lines(ret, 0);

	fclose(in_filp);

	return ret;
}

int save_buffer(FILE_BUFFER *buffer, const char *file_name)
{
#ifdef DEBUG_ASSERT
	assert(buffer->lines->gap_len == GAP_SIZE);
#endif
	PIECE_DESC *desc = buffer->piece_desc;

	PIECE *piece = (PIECE*)TreeMin(desc->tree)->info;

	char *add_buf = buffer->add_buf->buffer;

	FILE *fp = fopen(file_name, "w+");

	if (fp == NULL)
		return -1;

        while (piece != NULL)
	{
		if ((piece->flags & 8) == 8)
		{
		        for (size_t off = 0; off < piece->size; ++off)
			{
				fputc(buffer->file_buf[piece->offset + off], fp);
			}
		}
		else
		{
			for (size_t off = 0; off < piece->size; ++off)
			{
				fputc(add_buf[piece->offset + off], fp);
			}
		}
	        piece = (PIECE*)(TreeSuccessor(desc->tree, piece->node)->info);
	}
	fclose(fp);
	return 0;
}

void release_buffer(FILE_BUFFER *buffer)
{
	if (!buffer)
		return;

	munmap(buffer->file_buf, buffer->status->st_size);


	BUFFER *add_buf = ADD_BUF(buffer);

	free(add_buf->buffer);
	free(add_buf);

	free(buffer->status);

	free(buffer->name);

	/*free(buffer->user_cache->buffer);
	free(buffer->user_cache);*/

        RBTreeDestroy(buffer->piece_desc->tree);
	free(buffer->piece_desc);

	end_tui(buffer->win_desc);

	free(buffer->win_desc);

	free(buffer->lines->span1);

	free(buffer->lines);

	free(buffer);
}
/*
void clear_user_cache(FILE_BUFFER *buffer)
{
	BUFFER *cache = buffer->user_cache;
	memset(cache->buffer, 0, cache->size);
	cache->offset = 0;
}

void user_cache_append(const char new_item, FILE_BUFFER *buffer)
{
	BUFFER *cache = buffer->user_cache;
	if (cache->offset == cache->size)
	{
		cache->size += 10;
		cache->buffer = (char*)realloc((void*)cache->buffer, cache->size);
	}
	cache->buffer[cache->offset++] = new_item;
}*/

void line_gap_add(const char new_item, size_t *y_pos, size_t *x_pos, FILE_BUFFER *buffer)
{
	LINE_TABLE *table = buffer->lines;
#ifdef DEBUG_ASSERT
	assert(table->span2_len > 0);
#endif
	*table->gap = new_item;
	--table->gap_len;
	++table->gap;
	++table->span1_len;

	if (table->gap_len == 0)
	{
#ifdef DEBUG_ASSERT
		assert(table->span1_len + table->gap_len + table->span2_len <= NR_COLS * NR_LINES);
#endif
		//table->span2 = memmove(table->span2 + GAP_SIZE, table->span2, table->span2_len);
		//table->gap_len = GAP_SIZE;
		insert_item(table->gap-table->gap_size,table->gap_size,
			    table->offset+table->span1_len-table->gap_size, buffer);
		if (table->span1_len + table->span2_len <= table->lines * table->cols)
		{
			table->span2 = memmove(table->span2+table->gap_size, table->span2, table->span2_len);
			table->gap_len = table->gap_size;
		}
		else
		{
			size_t new_size = (table->lines*table->cols)-(table->span1_len+table->span2_len);
#ifdef DEBUG_ASSERT
			assert(new_size > 0);
#endif
			table->span2 = memmove(table->span2 + new_size, table->span2, table->span2_len);
			table->gap_len = new_size;
			table->gap_size = new_size;
		}
	}
	bool line_draw = true;
	if (table->line[*y_pos].len == table->cols)
	{
		--table->line[*y_pos].len;
	        size_t i = (*y_pos)+1;
		while (i < table->lines && table->line[i].len >= table->cols-1)
		{
			++i;
		}
		if (i >= table->lines)
		{
#ifdef DEBUG_ASSERT
			assert(0);
#endif
		}
		else
		{
			++table->line[i].len;
		}
		line_draw = false;
	}

	++(*x_pos);
	++table->line[*y_pos].len;
	if (*x_pos == table->cols)
	{
		++(*y_pos);
		*x_pos = 0;
#ifdef DEBUG_ASSERT
		assert(*y_pos < table->lines);
#endif
	}
	if (line_draw)
	{
		redraw_line(*y_pos, table);
	}
	else
	{
		print_lines(table);
	}
}

void inc_line_gap(size_t *y, size_t *x, FILE_BUFFER *buffer)
{
	LINE_TABLE *table = buffer->lines;
#ifdef DEBUG_ASSERT
	assert(table->gap + table->gap_len + 1 < table->span2 + table->span2_len);
	assert(table->span2_len > 0);
#endif
	if (table->gap_len < table->gap_size)
	{
		size_t len = table->gap_size - table->gap_len;
#ifdef DEBUG_ASSERT
		assert(insert_item(table->gap-len,len,(table->gap-table->span1)+table->offset,buffer)==0);
#else
		insert_item(table->gap-len,len,table->gap-table->span1+table->offset,buffer);
#endif
		table->span2 = memmove(table->gap + table->gap_size, table->span2, table->span2_len);
		table->gap_len = table->gap_size;
#ifdef DEBUG_ASSERT
		assert(table->span2_len - len > 0);
#endif
	}
        if (++(*x) > table->line[*y].len)
	{
		if (*y >= table->used-1)
		{
			--(*x);
		}
		else
		{
			++(*y);
			(*x) = 0;
		}
	}
	else
	{
		++table->span1_len;
		++table->gap;
		--table->span2_len;
		*(table->gap - 1) = *(table->span2);
		*(table->span2++) = 0;
	}
}

void dec_line_gap(size_t *y, size_t *x, FILE_BUFFER *buffer)
{
	LINE_TABLE *table = buffer->lines;

	//if (table->gap_len < GAP_SIZE

	if (table->gap_len < table->gap_size)
	{
		size_t len = table->gap_size - table->gap_len;
#ifdef DEBUG_ASSERT
		assert(insert_item(table->gap-len,len,(table->gap-table->span1)+table->offset,buffer)==0);
#else
		insert_item(table->gap-len,len,table->gap-table->span1+table->offset,buffer);
#endif
		table->span2 = memmove(table->gap + table->gap_size, table->span2, table->span2_len);
		table->gap_len = table->gap_size;
#ifdef DEBUG_ASSERT
		assert(table->span2_len - len > 0);
#endif
	}
	if (*x == 0)
	{
		if (*y == 0)
			return;
		--(*y);
		*x = table->line[*y].len;
	}
	else
	{
		--table->span1_len;
		--table->gap;
		++table->span2_len;
		*(--table->span2) = *(table->gap);
		memmove(table->gap, table->gap+1, table->gap_len);
	        --(*x);
	}
}

void goto_next_line(size_t *y, size_t *x, FILE_BUFFER *buffer)
{
	LINE_TABLE *table = buffer->lines;
#ifdef DEBUG_ASSERT
	assert(*y < table->lines);
#endif
	if (table->gap_len < table->gap_size)
	{
		size_t len = table->gap_size - table->gap_len;
#ifdef DEBUG_ASSERT
		assert(insert_item(table->gap-len,len,(table->gap-table->span1)+table->offset,buffer)==0);
#else
		insert_item(table->gap-len,len,table->gap-table->span1+table->offset,buffer);
#endif
		table->span2 = memmove(table->gap + table->gap_size, table->span2, table->span2_len);
		table->gap_len = table->gap_size;
#ifdef DEBUG_ASSERT
		assert(table->span2_len - len > 0);
#endif
	}
	if (*y >= table->lines-1)
		return;
	size_t old_x = *x;
	if (table->line[(*y)+1].len < *x)
	{
		*x = table->line[(*y)+1].len;
	}
	++(*y);
	move_line_gap_fwd(table->line[(*y)-1].len-old_x+(*x), buffer);
}

void goto_prev_line(size_t *y, size_t *x, FILE_BUFFER *buffer)
{
	LINE_TABLE *table = buffer->lines;
#ifdef DEBUG_ASSERT
	assert(*y < table->lines);
#endif
	if (table->gap_len < table->gap_size)
	{
		size_t len = table->gap_size - table->gap_len;
#ifdef DEBUG_ASSERT
		assert(insert_item(table->gap-len,len,(table->gap-table->span1)+table->offset,buffer)==0);
#else
		insert_item(table->gap-len,len,table->gap-table->span1+table->offset,buffer);
#endif
		table->span2 = memmove(table->gap + table->gap_size, table->span2, table->span2_len);
		table->gap_len = table->gap_size;
#ifdef DEBUG_ASSERT
		assert(table->span2_len - len > 0);
#endif
	}
	if (*y == 0)
		return;
	size_t old_x = *x;
	if (table->line[(*y)-1].len < *x)
	{
		*x = table->line[(*y)-1].len;
	}
	--(*y);
	move_line_gap_back(old_x+(table->line[*y].len-(*x)), buffer);
}

void move_line_gap_fwd(size_t count, FILE_BUFFER *buffer)
{
	LINE_TABLE *table = buffer->lines;
#ifdef DEBUG_ASSERT
	assert(table->gap_len == table->gap_size);
	assert(count <= table->span2_len);
#endif
	memcpy(table->gap, table->span2, count);
	table->span2 += count;
	table->span2_len -= count;
	table->span1_len += count;
	table->gap += count;
}

void move_line_gap_back(size_t count, FILE_BUFFER *buffer)
{
	LINE_TABLE *table = buffer->lines;
#ifdef DEBUG_ASSERT
	assert(table->gap_len == table->gap_size);
	assert(count <= table->span1_len);
#endif
	memcpy(table->span2-count, table->gap-count, count);
	table->span2 -= count;
	table->span2_len += count;
	table->span1_len -= count;
	table->gap -= count;
}



void line_delete_char(size_t *y, size_t *x, FILE_BUFFER *buffer)
{
	LINE_TABLE *table = buffer->lines;
	if (*y == 0 && *x == 0 && table->offset == 0)
		return;
	/*
	  IMPORTANT!!!!!!!!!!
	  If gap_len < gap_size, then the text before gap
	  hasn't yet been added to the piece table.
	 */
	if (table->gap_len < table->gap_size)
	{
		size_t len = table->gap_size - table->gap_len;
#ifdef DEBUG_ASSERT
		assert(insert_item(table->gap-len,len-1,(table->gap-table->span1)+table->offset,buffer)==0);
#else
		insert_item(table->gap-len,len-1,table->gap-table->span1+table->offset,buffer);
#endif
		table->gap_size = table->gap_len;
	}
	else
	{
		delete_item(table->offset+(table->gap-table->span1)-1, 1, buffer);
	}
#ifdef DEBUG_ASSERT
	assert(table->span1 < table->gap);
#endif
	++table->gap_len;
	if (table->gap_len > table->gap_size)
		++table->gap_size;
	--table->gap;
	--table->span1_len;
	if (*x == 0 && *y != 0)
	{
#ifdef DEBUG_ASSERT
		assert(table->offset == 0);
#endif
		size_t gap = table->cols - table->line[(*y)-1].len;
		size_t amount = (table->line[*y].len >= gap) ? gap : table->line[*y].len;
		table->line[(*y)-1].len += amount;
		if (amount == table->line[*y].len)
		{
			size_t i;
			for (i = *y; i < table->lines-1; ++i)
			{
				table->line[i].len = table->line[i+1].len;
			}
			table->used = i;
		}
		else
		{
			table->line[*y].len -= amount;
		}
		--(*y);
		*x = table->line[*y].len;
		print_lines(table);
	}
	else
	{
		--(*x);
		--table->line[*y].len;
		redraw_line(*y, table);
	}
}



size_t add_buffer_append(const char *new_item, size_t len, FILE_BUFFER *buffer)
{
	if (buffer == NULL || new_item == NULL || len == 0)
		return 0;

	BUFFER *add_buf = ADD_BUF(buffer);

	//PAGE *current = add_buf->tail;

	size_t wrote = 0;
	register const char *ptr = new_item;

	while (len > 0)
	{
		if (add_buf->size == add_buf->offset)
		{
			add_buf->buffer = (char*)realloc(add_buf->buffer, add_buf->size += len + PAGE_SIZE);
		}

		while (add_buf->offset < add_buf->size && len > 0)
		{
			add_buf->buffer[add_buf->offset++] = *(ptr++);
			++wrote;
			--len;
		}
	}

	return wrote;
}



PIECE *find_containing_piece(size_t offset, FILE_BUFFER *buffer, size_t *ret_off)
{
	/*P_CACHE *tmp = find_in_cache_off(offset, buffer);
	if (tmp != NULL)
	return tmp->piece;*/
	//TODO: situation when len overlaps multiple pieces
	rb_red_blk_node *ret = buffer->piece_desc->tree->root->left;

	while (ret != buffer->piece_desc->tree->nil)
	{
		size_t cur_off = piece_offset((PIECE*)ret->info);
		if (cur_off < offset)
		{
			if (cur_off + ((PIECE*)ret->info)->size > offset/* + len*/)
			{
				if (ret_off != NULL)
					*ret_off = cur_off;
				return (PIECE*)ret->info;
			}
			ret = ret->right;
		}
		else if (cur_off > offset)
		{
			ret = ret->left;
		}
		else
		{
			if (ret_off != NULL)
				*ret_off = cur_off;
			return (PIECE*)ret->info;
		}
	}
	return NULL;
}

/*
  To insert an item:
  Create a piece pointing to the items before the inserted one
  Create a piece of the newly inserted item
  Create a piece of the items after the newly inserted item
*/
//TODO: FIX SIZES
/*int insert_item(const char *new_item, size_t len, size_t abs_size_left, FILE_BUFFER *buffer)
{
	size_t off;
	PIECE *cur_piece = find_containing_piece(abs_size_left, buffer, &off);
	if (cur_piece == NULL)
		return -1;
	if (cur_piece->flags & IN_FILE == 0
	    && cur_piece->size + cur_piece->offset == ADD_BUF(buffer)->offset-1)
	{
		size_t old = cur_piece->size;
		cur_piece->size += len;
		fix_sizes(cur_piece, old + len);
	}
	else
	{
		size_t old = cur_piece->size;
		size_t old_off = cur_piece->offset;
		//cur_piece->size = ;
		size_t new = abs_size_left - off;
		bool old_buf = cur_piece->flags;
		delete_piece(cur_piece, buffer);
		cur_piece = add_piece(off, 0, new, old_buf, old_off, buffer);


		//fix_sizes(cur_piece, new - old);

		cur_piece = add_piece(abs_size_left,cur_piece->size_right,
				      len, IN_ADD, ADD_BUF(buffer)->offset, buffer);

		//TODO: Check all size_rights!!!!
		cur_piece = add_piece(abs_size_left+cur_piece->size, cur_piece->size_right, old - new,
				      old_buf, old_off + new, buffer);
	}
	add_buffer_append(new_item, len, buffer);

	buffer->buffer_size += len;

	return 0;
}*/



int insert_item(const char *new_item, size_t len, size_t offset, FILE_BUFFER *buffer)
{
	size_t off;
	PIECE *cur_piece = find_containing_piece(offset, buffer, &off);

	size_t old;

	if (cur_piece == NULL)
	{
		//return -1;
#ifdef DEBUG_ASSERT
		assert(offset != 0);
#endif

		cur_piece = find_containing_piece(offset-1, buffer, &off);

		if (cur_piece != NULL)
		{
			change_current(cur_piece, buffer);
			if (P_INADD(cur_piece) && cur_piece->size + cur_piece->offset == ADD_BUF(buffer)->offset)
				goto redo;
			piece_insert_right(len, IN_ADD, ADD_BUF(buffer)->offset, buffer);
		}
		else
		{
			cur_piece = find_containing_piece(offset + 1, buffer, &off);

#ifdef DEBUG_ASSERT
			assert(cur_piece != NULL);
#endif
			change_current(cur_piece, buffer);
			piece_insert_left(len, IN_ADD, ADD_BUF(buffer)->offset, buffer);
		}
		goto exit;
	}

	if (P_INADD(cur_piece) && cur_piece->size + off == offset
	    && cur_piece->size + cur_piece->offset == ADD_BUF(buffer)->offset)
	{
	redo:
	        old = cur_piece->size;
		cur_piece->size += len;
		fix_sizes(cur_piece, len);
	}
	else if (offset - off == 0 && piece_offset(cur_piece) > 0)
	{
	        cur_piece = (PIECE*)TreePredecessor(buffer->piece_desc->tree, cur_piece->node)->info;
		if (P_INADD(cur_piece) && cur_piece->size + cur_piece->offset == ADD_BUF(buffer)->offset)
			goto redo;
		goto fail;
	}
	else
	{
	fail:
		change_current(cur_piece, buffer);
	        old = cur_piece->size;
		size_t old_off = cur_piece->offset;
		size_t new = offset - off;
		char old_flags = cur_piece->flags;
		size_t old_pos = off;
		cur_piece->size = new;
		fix_sizes(cur_piece, (int)new - (int)old);

		cur_piece = piece_insert_right(len, IN_ADD, ADD_BUF(buffer)->offset, buffer);
		change_current(cur_piece, buffer);
		piece_insert_right(/*old - offset - len*/old-new, old_flags, old_off + new, buffer);
	}
	add_buffer_append(new_item, len, buffer);

exit:
	buffer->buffer_size += len;

	return 0;
}



/*
  To delete an item:
  Create a piece pointing to the items before the deleted one
  Create a piece pointing to the items after the deleted one
*/
int delete_item(size_t offset, size_t len/*amount to delete*/, FILE_BUFFER *buffer)
{
	size_t abs_off;
	PIECE *piece = find_containing_piece(offset, buffer, &abs_off);

	if (piece == NULL)
		return -1;
	change_current(piece, buffer);
	if (offset == abs_off)
	{
		if (len < piece->size)
		{
			piece->size -= len;
			fix_sizes(piece, -(int)len);
			piece->offset += len;
		}
		else if (len == piece->size)
		{
			//fix_sizes(piece, -len);
			delete_piece(piece, buffer);
		}
		else
		{
			return -1;
		}
		goto exit;
	}
	if (offset + len == abs_off + piece->size)
	{
		piece->size -= len;
		fix_sizes(piece, -(int)len);
		//piece->offset += len;
	        goto exit;
	}
	size_t old = piece->size;
	piece->size = offset - abs_off;
	fix_sizes(piece, -((int)piece->size - (int)old));

	piece_insert_right(old-piece->size-len,piece->flags,piece->offset+piece->size+len,buffer);
exit:
	buffer->buffer_size -= len;
	return 0;
}

static void delete_piece(PIECE *piece, FILE_BUFFER *buffer/*, char flags*/)
{
	fix_sizes(piece, -(int)piece->size);
	RBDelete(buffer->piece_desc->tree, piece->node);
}



size_t piece_offset(const PIECE *key)
{
	size_t offset;
	/*if ((offset = find_offset_in_cache(key, key->file)) != MAGIC)
		return offset;*/
	offset = key->size_left;
        rb_red_blk_node *node = key->node;

	while (!IS_NIL(node->parent))
	{
		if (node->parent->right == node)
		{
			PIECE *p_info = (PIECE*)node->parent->info;
			offset += p_info->size_left +
			        p_info->size;
		}

		node = node->parent;
	}

	return offset;
}


/*used for the red-black tree*/
int piece_compare(const void *a, const void *b)
{
	const PIECE *_a = (PIECE*)a;
	const PIECE *_b = (PIECE*)b;
	size_t off1 = piece_offset(_a);
	size_t off2 = piece_offset(_b);
	if (off1 > off2)
		return 1;
	if (off1 < off2)
		return -1;
	return 0;
}

void info_destroy(void *info)
{
	PIECE *_info = (PIECE*)info;
	if ((_info->flags & KEY) == 0)
	{
		free(_info);
	}
	else
	{
		_info->flags &= ~INFO;
	}
}

void key_destroy(void *key)
{
	PIECE *_key = (PIECE*)key;
	if ((_key->flags & INFO) == 0)
	{
		free(_key);
	}
	else
	{
		_key->flags &= ~KEY;
	}
}



/*
  For when the size field in a piece changes.
  Need to update the size_left field of all its right parents.
 */
void fix_sizes(PIECE *piece, int change)
{
	rb_red_blk_node *node = piece->node;
	if (change == 0 || node->parent == NULL)
		return;
	while (!IS_ROOT(node/*->parent*/))
	{
		if (node->parent->left == node)
		{
			piece = (PIECE*)node->parent->info;
			piece->size_left += change;
		}
		else
		{
			piece = (PIECE*)node->parent->info;
			piece->size_right += change;
		}

		node = node->parent;
	}
}



void print_buffer(FILE_BUFFER *buffer)
{
	PIECE_DESC *desc = buffer->piece_desc;

	PIECE *piece = (PIECE*)TreeMin(desc->tree)->info;

	char *add_buf = buffer->add_buf->buffer;

        while (piece != NULL)
	{
		if ((piece->flags & 8) == 8)
		{
		        for (size_t off = 0; off < piece->size; ++off)
			{
				printf("%c", buffer->file_buf[piece->offset + off]);
			}
		}
		else
		{
			/*PAGE *current = buffer->add_buf->head;
			for (size_t off = 0; off < piece->size;)
			{
				printf("%c", current->page[(piece->offset + off) % PAGE_SIZE]);
				if (++off % PAGE_SIZE == 0)
					current = current->next;
					}*/
			for (size_t off = 0; off < piece->size; ++off)
			{
				printf("%c", add_buf[piece->offset + off]);
			}
		}
	        piece = (PIECE*)(TreeSuccessor(desc->tree, piece->node)->info);
	}
}



ssize_t buffer_read(char *dest, size_t offset, size_t count, FILE_BUFFER *buffer)
{
	size_t abs_off;
	PIECE *container = find_containing_piece(offset, buffer, &abs_off);
#ifdef DEBUG_ASSERT
	Assert(container != NULL, "In get_string. container == NULL");
#endif
	ssize_t read = 0;
	int chars;
	register char *ptr;
	//size_t piece_off = abs_off + container->size - offset;
	size_t piece_off = offset - abs_off;
	size_t old_size;
	while (count > 0)
	{
		//container = find_containing_piece(offset + count, buffer, &abs_off);
#ifdef DEBUG_ASSERT
		Assert(container != NULL, "In get_string. container == NULL");
#endif
		chars = (count < container->size - piece_off) ?
			count : container->size - piece_off;

		read += chars;
		count -= chars;

		ptr = piece_off + GET_BUFFER(buffer, container->flags) + container->offset;
		piece_off = 0;
		old_size = container->size;
		container = (PIECE*)TreeSuccessor(buffer->piece_desc->tree, container->node)->info;
		abs_off += old_size;
#ifdef DEBUG_ASSERT
		Assert(abs_off < buffer->buffer_size, "In get_string: abs_off < buffer->buffer_size");
#endif
		while (chars-- > 0)
			*(dest++) = *(ptr++);
	}
	return read;
}



char buffer_readc(size_t offset, FILE_BUFFER *buffer)
{
	size_t abs_off;
	PIECE *container = find_containing_piece(offset, buffer, &abs_off);

#ifdef DEBUG_ASSERT
	if (container == NULL)
	{
		printf("%d", (int)offset);
		assert(0);
	}
	//Assert(container != NULL, "In buffer_readc. container == NULL");
#endif
	size_t piece_off = offset - abs_off;

	return *(GET_BUFFER(buffer, container->flags) + piece_off + container->offset);
}



void fill_lines(FILE_BUFFER *buffer, size_t offset)
{
	size_t size = buffer->buffer_size;
#ifdef DEBUG_ASSERT
	Assert(offset < size, "In fill_lines. Offset >= size");
#endif
	LINE_TABLE *l_table = buffer->lines;
	size_t lines = l_table->lines;
	int lineno = 0;
	int i = 0;
	int abs_i = 0;
	char c;
	while (lineno < lines && offset < size)
	{
		c = buffer_readc(offset++, buffer);
		if (abs_i + l_table->span1 == l_table->gap)
			abs_i += l_table->gap_len;
		if (c == 10 /*new line character*/)
		{
			l_table->line[lineno].len = i;
			l_table->line[lineno].new_line = true;
			i = 0;
			++lineno;
			l_table->line[lineno].new_line = false;
		}
		else
		{
			//l_table->line[lineno].buf[i++] = c;
			l_table->span1[abs_i++] = c;
			++i;
			if (i >= l_table->cols)
			{
				l_table->line[lineno].len = i;
				i = 0;
				++lineno;
				l_table->line[lineno].new_line = false;
			}
		}
	}
	buffer->lines->used = lineno + 1;
}



PIECE *piece_insert_left(size_t size, char flags, size_t span_off, FILE_BUFFER *buffer)
{
	PIECE *ret = (PIECE*)malloc(sizeof(PIECE));

	ret->file = buffer;

	ret->node = insert_left(buffer->piece_desc->tree, CURRENT(buffer)->node);

	ret->node->info = ret->node->key = ret;

        PIECE *tmp = (PIECE*)ret->node->left->info;

	if (tmp != NULL)
		ret->size_left = tmp->size_left + tmp->size + tmp->size_right;
	else
		ret->size_left = 0;

	tmp = (PIECE*)ret->node->right->info;

	if (tmp != NULL)
	{
		ret->size_right = tmp->size_left + tmp->size + tmp->size_right;
	}
	else
		ret->size_right = 0;

	ret->size = size;

	fix_sizes(ret, size);

	ret->flags = flags | INFO | KEY;
	ret->offset = span_off;


	return ret;
}

PIECE *piece_insert_right(size_t size, char flags, size_t span_off, FILE_BUFFER *buffer)
{
	PIECE *ret = (PIECE*)malloc(sizeof(PIECE));

	ret->file = buffer;

	ret->node = insert_right(buffer->piece_desc->tree, CURRENT(buffer)->node);

	ret->node->info = ret->node->key = ret;

        PIECE *tmp = (PIECE*)ret->node->left->info;

	if (tmp != NULL)
		ret->size_left = tmp->size_left + tmp->size + tmp->size_right;
	else
		ret->size_left = 0;

	tmp = (PIECE*)ret->node->right->info;

	if (tmp != NULL)
	{
		ret->size_right = tmp->size_left + tmp->size + tmp->size_right;
	}
	else
		ret->size_right = 0;

	ret->size = size;

	fix_sizes(ret, size);

	ret->flags = flags | INFO | KEY;
	ret->offset = span_off;


	return ret;
}


/*
  TODO: have a large cache of pieces instead of just
  the current one.
 */
void change_current(PIECE *new_current, FILE_BUFFER *buffer)
{
	//buffer->piece_desc->current = new_current;
	P_CACHE *cache = buffer->piece_desc->cache;
	if (new_current != cache[0].piece)
	{
		/*memset(cache, 0, CACHE_SIZE * sizeof(P_CACHE));
		for (size_t i = 0; i < CACHE_SIZE && new_current != NULL; ++i)
		{
			cache[i].piece = new_current;
			cache[i].offset = (i == 0) ? piece_offset(new_current) :
				cache[i-1].offset + new_current->size;
			new_current = (PIECE*)TreeSuccessor(buffer->piece_desc->tree, new_current->node)->info;
			}*/

		cache[CACHE_CURR].piece = new_current;
		cache[CACHE_CURR].offset = piece_offset(new_current);

		cache[CACHE_PREV].piece = (PIECE*)TreePredecessor(buffer->piece_desc->tree,
								  new_current->node)->info;
		if (cache[CACHE_PREV].piece != NULL)
		{
			cache[CACHE_PREV].offset = cache[CACHE_CURR].offset - cache[CACHE_PREV].piece->size;
		}

		cache[CACHE_NEXT].piece = (PIECE*)TreeSuccessor(buffer->piece_desc->tree,
								  new_current->node)->info;
		if (cache[CACHE_NEXT].piece != NULL)
		{
			cache[CACHE_NEXT].offset = cache[CACHE_CURR].offset + new_current->size;
		}
	}
}
