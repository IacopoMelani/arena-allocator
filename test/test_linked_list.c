#include "../src/arena.h"
#include "../src/memdump.h"
#include "../src/utils.h"
#include <stdlib.h>

typedef struct Node {
    struct Node *next;
    int value;
} Node;

typedef struct {
    Node *head;
    size_t size;
    Allocator *allocator;
} LinkedList;

LinkedList linked_list_init(Allocator *allocator) {
    return (LinkedList){
        .head = NULL,
        .size = 0,
        .allocator = allocator,
    };
}

int linked_list_push(LinkedList *linked_list, int value) {
    Node *node = make(Node, 1, (*linked_list->allocator));
    if (!node) {
        return -1;
    }
    node->next = linked_list->head;
    node->value = value;
    linked_list->head = node;
    linked_list->size += 1;
    return 0;
}

Node *linked_list_pop(LinkedList *linked_list) {
    if (linked_list->size == 0) {
        return NULL;
    }
    Node *node = linked_list->head;
    linked_list->head = node->next;
    linked_list->size -= 1;
    return node;
}

void linked_list_print(LinkedList *linked_list) {
    Node *node = linked_list->head;
    char buffer[1024] = {0};
    while (node) {
        sprintf(&*buffer, "%s%d ", buffer, node->value);
        node = node->next;
    }
    info("linked list: %s\n", buffer);
}

void linked_list_free(LinkedList *linked_list) {
    Node *node = linked_list->head;
    while (node) {
        Node *next = node->next;
        release(Node, 1, node, (*linked_list->allocator));
        node = next;
    }
}

int main(void) {

    size_t size = sizeof(Node) * 20;

    void *buffer = malloc(size);

    Arena arena = arena_init(buffer, size, DEFAULT_ALLIGNMENT, BestFit);

    Allocator allocator = arena_alloc_init(&arena);

    LinkedList linked_list = linked_list_init(&allocator);

    for (int i = 0; i < 20; i++) {
        int res = linked_list_push(&linked_list, i + 1);
        if (res != 0) {
            error("linked_list_push failed\n");
        }
    }

    linked_list_print(&linked_list);

    info("linked list size: %zu\n", linked_list.size);

    int res_should_fail = linked_list_push(&linked_list, -1);
    if (res_should_fail == 0) {
        error("allocator should have failed to allocate memory\n");
    }

    hexDump("arena", buffer, size);

    for (int i = 0; i < 20; i++) {
        Node *node = linked_list_pop(&linked_list);
        if (node->value != 20 - i) {
            error("expected %d, got %d\n", 20 - i, node->value);
        }
        release(Node, 1, node, allocator);
    }

    info("linked list size: %zu\n", linked_list.size);

    for (int i = 0; i < 20; i++) {
        int res = linked_list_push(&linked_list, i * 10 + 10);
        if (res != 0) {
            error("linked_list_push failed\n");
        }
    }

    linked_list_print(&linked_list);

    hexDump("arena", buffer, size);

    linked_list_free(&linked_list);

    assert(allocated(allocator) == 0, "Memory leak detected, allocated: %zu\n", allocated(allocator));

    arena_free_all(&arena);

    free(buffer);
    buffer = NULL;

    info("test_linked_list passed\n");

    return 0;
}
