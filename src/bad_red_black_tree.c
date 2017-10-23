#include <red_black_tree.h>

RB_TREE* rb_tree_create(int (*compare)(const void*, const void*),
			void (*destroy_key)(void*), 
			void (*destroy_info)(void*))
{
	if (compare == NULL || destroy_key == NULL || destroy_info == NULL)
		return NULL;

	RB_TREE *ret = (RB_TREE*)malloc(sizeof(RB_TREE));

	ret->root = NULL;
	ret->compare = compare;
	ret->destroy_key = destroy_key;
	ret->destroy_value = destroy_value;

	return ret;
}

/* no duplicate keys allowed!*/
RB_NODE *rb_find_node(RB_TREE *tree, void *key)
{
	RB_NODE *ret = tree->root;
	if (ret == NULL)
		return NULL;
	while (ret != NULL)
	{
		int comp = tree->compare(ret->key, ret->child[LEFT]->key);
		if (comp == 0)
			return ret;
		if (comp < 0)
		{
			ret = ret->child[LEFT];
		}
		else
		{
			ret = ret->child[RIGHT];
		}
	}
	return ret;
}

RB_NODE *rb_find_min(RB_TREE *tree)
{
	RB_NODE *ret = tree->root;
	if (ret == NULL)
		return ret;
	while (ret->child[LEFT] != NULL)
		ret = ret->child[LEFT];
	return ret;
}

RB_NODE *rb_find_max(RB_TREE *tree)
{
	RB_NODE *ret = tree->root;
	if (ret == NULL)
		return ret;
	while (ret->child[RIGHT] != NULL)
		ret = ret->child[RIGHT];
	return ret;
}

/* Returns new parent*/
RB_NODE *rb_single_rotate_with_left(RB_NODE *parent)
{
	RB_NODE *ret = parent->child[LEFT];
	if (ret == NULL)
		return ret;
	parent->child[LEFT] = ret->child[RIGHT];
	ret->child[RIGHT]->parent = parent;

	ret->child[RIGHT] = parent;
	ret->parent = parent->parent;
	parent->parent = ret;

	if (ret->parent->child[RIGHT] == parent)
		ret->parent->child[RIGHT] = ret;
	else
		ret->parent->child[LEFT] = ret;

	return ret;
}

RB_NODE *rb_single_rotate_with_right(RB_NODE *parent)
{
	RB_NODE *ret = parent->child[RIGHT];
	if (ret == NULL)
		return ret;

	parent->child[RIGHT] = ret->child[LEFT];
	ret->child[LEFT]->parent = parent;

	ret->child[LEFT] = parent;
	ret->parent = parent->parent;
	parent->parent = ret;

	if (ret->parent->child[RIGHT] == parent)
		ret->parent->child[RIGHT] = ret;
	else
		ret->parent->child[LEFT] = ret;

	return ret;
}

void binary_insert(RB_TREE *tree, RB_NODE *node)
{
	RB_NODE *cur = tree->root;
	RB_NODE *prev;
	while (cur != NULL)
	{
		prev = cur;
		/*cur->key > node->key*/
		if (tree->compare(cur->key, node->key) > 0)
			cur = cur->child[LEFT];
		/*cur->key < node->key*/
		else
			cur = cur->child[RIGHT];
	}
	node->parent = prev;
	if (prev == tree->root || tree->compare(prev->key, node->key) > 0)
		prev->child[LEFT] = node;
	else
		prev->child[RIGHT] = node;
}

RB_NODE* rb_tree_insert(RB_TREE *tree, void *key, void *value)
{
	if (tree == NULL)
		return NULL;
	RB_NODE *ret = (RB_NODE*)malloc(sizeof(RB_NODE));
	ret->key = key;
	ret->value = value;

	if (tree->root == NULL)
	{
		tree->root = ret;
		ret->parent = NULL;
		ret->child[LEFT] = NULL;
		ret->child[RIGHT] = NULL;
		return ret;
	}
	
	binary_insert(tree, ret);
	ret->color = RED;

	while (ret->parent->color == RED)
	{
		if (ret->parent == ret->parent->parent->child[LEFT])
		{
			
		}
	}
}


void rb_node_delete(RB_TREE *tree, RB_NODE *node);
void rb_tree_delete(RB_TREE *tree);
