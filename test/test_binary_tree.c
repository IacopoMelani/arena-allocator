#include "../src/arena.h"
#include "../src/memdump.h"
#include "../src/utils.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef struct Node {
    struct Node *left;
    struct Node *right;
    int value;
} Node;

void node_insert(Node *node, Node *new) {
    if (new->value >= node->value) {
        if (node->right == NULL) {
            node->right = new;
        } else {
            node_insert(node->right, new);
        }
    } else {
        if (node->left == NULL) {
            node->left = new;
        } else {
            node_insert(node->left, new);
        }
    }
}

void node_free(Node *node, Allocator *allocator) {
    if (node == NULL) {
        return;
    }
    node_free(node->left, allocator);
    node_free(node->right, allocator);
    release(Node, 1, node, (*allocator));
}

typedef struct {
    Node *root;
    size_t height;
    size_t size;
    Allocator *allocator;
} BinaryTree;

BinaryTree binary_tree_new(Allocator *allocator) {
    return (BinaryTree){
        .root = NULL,
        .height = 0,
        .size = 0,
        .allocator = allocator,
    };
}

void binary_tree_insert(BinaryTree *tree, int value) {
    Node *node = make(Node, 1, (*tree->allocator));
    node->value = value;
    node->left = NULL;
    node->right = NULL;
    if (tree->root == NULL) {
        tree->root = node;
        return;
    }
    node_insert(tree->root, node);
}

static inline void binary_tree_free(BinaryTree *tree, Allocator *allocator) { node_free(tree->root, allocator); }

int main(void) {

    size_t size = sizeof(BinaryTree) + sizeof(Node) * 20;

    void *buffer = malloc(size);

    Arena arena = arena_init(buffer, size, DEFAULT_ALLIGNMENT, BestFit);

    Allocator allocator = arena_alloc_init(&arena);

    BinaryTree tree = binary_tree_new(&allocator);

    binary_tree_insert(&tree, 2);
    binary_tree_insert(&tree, 0);
    binary_tree_insert(&tree, 1);
    binary_tree_insert(&tree, 4);
    binary_tree_insert(&tree, 3);
    binary_tree_insert(&tree, 7);
    binary_tree_insert(&tree, 5);

    // Current tree:
    //  2 -> 0  -> NULL
    //  |    |
    //  |    1 -> NULL
    //  |
    //  4 -> 3 -> NULL
    //  |
    //  7 -> 5 -> NULL
    //
    // Height: 3
    // Size: 7

    hexDump("arena", buffer, arena.size);

    binary_tree_free(&tree, &allocator);

    assert(allocated(allocator) == 0, "Memory leak detected, allocated: %zu\n", allocated(allocator));

    arena_free_all(&arena);

    free(buffer);
    buffer = NULL;

    info("Binary tree test passed\n");

    return 0;
}
