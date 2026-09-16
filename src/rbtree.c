#include "rbtree.h"
#include "rbtree_internal.h"

#include <stdlib.h>
#include <string.h>

static rb_node_t *find_node(const rbtree_t *t, const char *key) {
    rb_node_t *n = t->root;
    /* invariant: key, if present, lies within n's subtree */
    while (n != t->nil) {
        int cmp = strcmp(key, n->key);
        if (cmp == 0) return n;
        n = cmp < 0 ? n->left : n->right;
    }
    return t->nil;
}

static rb_node_t *tree_minimum(rb_node_t *nil, rb_node_t *n) {
    /* invariant: the minimum of n's subtree lies under n->left while one exists */
    while (n->left != nil) n = n->left;
    return n;
}

static void rb_transplant(rbtree_t *t, rb_node_t *u, rb_node_t *v) {
    if (u->parent == t->nil) t->root = v;
    else if (u == u->parent->left) u->parent->left = v;
    else u->parent->right = v;
    v->parent = u->parent;
}

int rb_delete(rbtree_t *t, const char *key) {
    rb_node_t *z = find_node(t, key);
    if (z == t->nil) return -1;

    if (z->left == t->nil || z->right == t->nil) {
        /* 0/1-child case: not implemented yet. Leave the tree unchanged. */
        return -1;
    }

    /* Two-children case: copy the in-order successor's key/value into z
     * (z never moves), then splice the successor node out of the tree.
     * y has at most a right child, since it's a subtree minimum. Fixup
     * is not implemented yet, so this is only correct when y is red. */
    rb_node_t *y = tree_minimum(t->nil, z->right);

    free(z->key);
    if (t->value_free) t->value_free(z->value);
    z->key = y->key;
    z->value = y->value;
    y->key = NULL;
    y->value = NULL;

    rb_node_t *x = y->right;
    rb_transplant(t, y, x);
    free(y);

    t->size--;
    return 0;
}

size_t rb_size(const rbtree_t *t) {
    return t->size;
}

static int check_bst(rb_node_t *nil, rb_node_t *n, const char *lo, const char *hi) {
    if (n == nil) return 1;
    if (lo && strcmp(n->key, lo) <= 0) return 0;
    if (hi && strcmp(n->key, hi) >= 0) return 0;
    return check_bst(nil, n->left, lo, n->key) && check_bst(nil, n->right, n->key, hi);
}

static int black_height(rb_node_t *nil, rb_node_t *n, int *valid) {
    if (n == nil) return 1;
    if (n->color == RB_RED &&
        (n->left->color == RB_RED || n->right->color == RB_RED)) {
        *valid = 0;
    }
    int lh = black_height(nil, n->left, valid);
    int rh = black_height(nil, n->right, valid);
    if (lh != rh) *valid = 0;
    return lh + (n->color == RB_BLACK ? 1 : 0);
}

int rb_validate(const rbtree_t *t) {
    if (t->nil->color != RB_BLACK) return 1;
    if (t->root != t->nil && t->root->color != RB_BLACK) return 1;
    if (!check_bst(t->nil, t->root, NULL, NULL)) return 1;

    int valid = 1;
    black_height(t->nil, t->root, &valid);
    return valid ? 0 : 1;
}

static void free_subtree(rb_node_t *nil, rb_node_t *n, rb_value_free_fn value_free) {
    if (n == nil) return;
    free_subtree(nil, n->left, value_free);
    free_subtree(nil, n->right, value_free);
    free(n->key);
    if (value_free) value_free(n->value);
    free(n);
}

void rb_destroy(rbtree_t *t) {
    if (t == NULL) return;
    free_subtree(t->nil, t->root, t->value_free);
    free(t->nil);
    free(t);
}
