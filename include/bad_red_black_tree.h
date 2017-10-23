#ifndef RED_BLACK_TREE_H
#define RED_BLACK_TREE_H

#include <stdlib.h>

#define RED 0
#define BLACK 1

#define LEFT 0
#define RIGHT 1


typedef struct _RB_NODE
{
	char color;
	struct _RB_NODE *child[2];
	struct _RB_NODE *parent;
	void *value;
	void *key;
} RB_NODE;

typedef struct _RB_TREE
{
	RB_NODE *root;

	/* Compare(a,b) should return 1 if *a > *b, -1 if *a < *b, and 0 otherwise */
	int (*compare)(const void* a, const void* b);
	void (*destroy_key)(void* a);
	void (*destroy_value)(void* a);
} RB_TREE;

RB_TREE* rb_tree_create(int (*compare)(const void*, const void*),
			void (*destroy_key)(void*), 
			void (*destroy_info)(void*));


RB_NODE* rb_tree_insert(RB_TREE *tree, void *key, void *info);
void rb_node_delete(RB_TREE *tree, RB_NODE *node);
void rb_tree_delete(RB_TREE *tree);

/* no duplicate keys allowed!*/
RB_NODE *rb_find_node(RB_TREE *tree, void *key);

RB_NODE *rb_find_min(RB_TREE *tree);
RB_NODE *rb_find_max(RB_TREE *tree);

/* Returns new parent*/
RB_NODE *rb_single_rotate_with_left(RB_NODE *parent);
RB_NODE *rb_single_rotate_with_right(RB_NODE *parent);

#endif
