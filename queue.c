#include <stdlib.h>
#include <stdio.h>

#include "./headers/queue.h"

Node* makeNode(queueType *value, Node *next, Node *before) {
    Node *node;
    if (!(node = (Node *)malloc(sizeof(Node)))) {
        printf("%s\n", "Failed to allocate memory for Node");
        exit(1);
    }
    node -> value = value;
    node -> next = next;
    node -> before = before;
    return node;
}

void printQueue(Queue *queue, void (*printFunction) (queueType *)) {
    Node *node = queue -> head;
    printf("Queue Length: %d\n", queue->length);
    if (queue -> length == 0) return;
    while (node != NULL) {
        printf("Item: \n");
        (*printFunction)(node -> value);
        node = node -> next;
    }
}

Queue* makeQueue() {
    Queue *queue;
    if (!(queue = (Queue *)malloc(sizeof(Queue)))) {
        printf("%s\n", "Failed to allocate memory for queue");
        exit(1);
    }
    queue -> head = NULL;
    queue -> tail = NULL;
    queue -> length = 0;
    return queue;
}

void queueAdd(Queue *queue, queueType *value) {
    Node *node = makeNode(value, NULL, queue->tail);
    if (queue -> length > 0) {
        queue -> tail -> next = node;
        queue -> tail = node;
    } else {
        queue -> head = node;
        queue -> tail = node;
    }
    queue -> length++;
}

queueType* dequeue(Queue *queue) {
    if (queue -> length == 0) {
        return NULL;
    }
    queueType *value = queue -> head -> value;
    Node *next = queue -> head -> next;
    free(queue -> head);
    queue -> head = next;
    queue -> length--;
    return value;
}

void queueRemove(Queue *queue, Node *node) {
    if (queue -> length == 0) return;
    if (queue -> head == node) queue -> head = node -> next;
    else node -> before -> next = node -> next;

    if (queue -> tail == node) queue -> tail = node -> before;
    else node -> next -> before = node -> before;

    queue -> length--;
    free(node -> value);
    free(node);
}
