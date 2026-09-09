#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rbtree.h"
#include "../src/rbtree_internal.h"

static int g_failures = 0;

#define CHECK(cond, ...) do { \
    if (!(cond)) { \
        g_failures++; \
        fprintf(stderr, "FAIL %s:%d: ", __FILE__, __LINE__); \
        fprintf(stderr, __VA_ARGS__); \
        fprintf(stderr, "\n"); \
    } \
} while (0)

static char *dup_key(const char *s) {
    size_t n = strlen(s) + 1;
    char *p = malloc(n);
    memcpy(p, s, n);
    return p;
}

static rbtree_t *new_bare_tree(void) {
    rbtree_t *t = malloc(sizeof *t);
    t->nil = malloc(sizeof *t->nil);
    t->nil->color = RB_BLACK;
    t->nil->key = NULL;
    t->nil->value = NULL;
    t->nil->parent = t->nil->left = t->nil->right = t->nil;
    t->root = t->nil;
    t->value_free = NULL;
    t->size = 0;
    return t;
}

static rb_node_t *mk_node(rbtree_t *t, const char *key, rb_color_t color) {
    rb_node_t *n = malloc(sizeof *n);
    n->key = dup_key(key);
    n->value = NULL;
    n->color = color;
    n->parent = n->left = n->right = t->nil;
    return n;
}

/* B(M)
 *  / \
 * R(D) R(T)
 * Deleting the red leaf D needs no fixup. */
static rbtree_t *build_red_leaf(void) {
    rbtree_t *t = new_bare_tree();
    rb_node_t *m = mk_node(t, "M", RB_BLACK);
    rb_node_t *d = mk_node(t, "D", RB_RED);
    rb_node_t *x = mk_node(t, "T", RB_RED);
    m->left = d; d->parent = m;
    m->right = x; x->parent = m;
    t->root = m;
    t->size = 3;
    return t;
}

/*        B(M)
 *       /    \
 *     B(D)   R(T)
 *            /  \
 *          B(Q) B(X)
 * Deleting black leaf D (sibling T is red) triggers fixup Case 1,
 * which then chains into Case 2 (T's children are both black). */
static rbtree_t *build_black_leaf_red_sibling(void) {
    rbtree_t *t = new_bare_tree();
    rb_node_t *p = mk_node(t, "M", RB_BLACK);
    rb_node_t *z = mk_node(t, "D", RB_BLACK);
    rb_node_t *w = mk_node(t, "T", RB_RED);
    rb_node_t *c = mk_node(t, "Q", RB_BLACK);
    rb_node_t *d = mk_node(t, "X", RB_BLACK);
    p->left = z; z->parent = p;
    p->right = w; w->parent = p;
    w->left = c; c->parent = w;
    w->right = d; d->parent = w;
    t->root = p;
    t->size = 5;
    return t;
}

/*              B(H)
 *             /    \
 *           B(D)    B(L)
 *          /  \      /  \
 *        R(B) R(F) R(J) R(N)
 * Deleting D (non-root, two children B/F) exercises the successor-copy
 * path where the successor F is D's immediate right child. */
static rbtree_t *build_two_children_nonroot(void) {
    rbtree_t *t = new_bare_tree();
    rb_node_t *h = mk_node(t, "H", RB_BLACK);
    rb_node_t *d = mk_node(t, "D", RB_BLACK);
    rb_node_t *b = mk_node(t, "B", RB_RED);
    rb_node_t *f = mk_node(t, "F", RB_RED);
    rb_node_t *l = mk_node(t, "L", RB_BLACK);
    rb_node_t *j = mk_node(t, "J", RB_RED);
    rb_node_t *n = mk_node(t, "N", RB_RED);
    h->left = d; d->parent = h;
    h->right = l; l->parent = h;
    d->left = b; b->parent = d;
    d->right = f; f->parent = d;
    l->left = j; j->parent = l;
    l->right = n; n->parent = l;
    t->root = h;
    t->size = 7;
    return t;
}

/* B(M)
 *  /
 * R(D)
 * Deleting root M (one child) is the case that actually reassigns
 * t->root via transplant, unlike the two-children case above. */
static rbtree_t *build_root_one_child(void) {
    rbtree_t *t = new_bare_tree();
    rb_node_t *m = mk_node(t, "M", RB_BLACK);
    rb_node_t *d = mk_node(t, "D", RB_RED);
    m->left = d; d->parent = m;
    t->root = m;
    t->size = 2;
    return t;
}

typedef struct {
    const char *name;
    rbtree_t *(*build)(void);
    const char *delete_key;
    size_t expected_size_after;
} delete_case_t;

static const delete_case_t cases[] = {
    { "red leaf",                  build_red_leaf,               "D", 2 },
    { "black leaf, red sibling",   build_black_leaf_red_sibling, "D", 4 },
    { "node with two children",    build_two_children_nonroot,   "D", 6 },
    { "root deletion",             build_root_one_child,         "M", 1 },
};

static void run_case(const delete_case_t *c) {
    rbtree_t *t = c->build();
    int rc = rb_delete(t, c->delete_key);
    CHECK(rc == 0, "%s: rb_delete returned %d, want 0", c->name, rc);
    CHECK(rb_validate(t) == 0, "%s: rb_validate failed after delete", c->name);
    size_t sz = rb_size(t);
    CHECK(sz == c->expected_size_after,
          "%s: rb_size == %zu, want %zu", c->name, sz, c->expected_size_after);
    rb_destroy(t);
}

int main(void) {
    size_t n = sizeof cases / sizeof cases[0];
    /* invariant: every case in the table has been run by the time the loop exits */
    for (size_t i = 0; i < n; i++) run_case(&cases[i]);

    if (g_failures) {
        fprintf(stderr, "%d check(s) failed\n", g_failures);
        return 1;
    }
    printf("all checks passed\n");
    return 0;
}
