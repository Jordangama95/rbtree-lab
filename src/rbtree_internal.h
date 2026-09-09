#ifndef RBTREE_INTERNAL_H
#define RBTREE_INTERNAL_H
#include "rbtree.h"

typedef enum { RB_RED, RB_BLACK } rb_color_t;

typedef struct rb_node {
    char *key;
    void *value;
    rb_color_t color;
    struct rb_node *parent, *left, *right;
} rb_node_t;

struct rbtree {
    rb_node_t *root;
    rb_node_t *nil;   /* sentinel: always black; parent/left/right self-looped, unused */
    rb_value_free_fn value_free;
    size_t size;
};

#endif
