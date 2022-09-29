#ifndef __QUEUE_H
#define __QUEUE_H

#include "customer.h"

typedef Customer queueType;

typedef struct NodeStruct{
    queueType *value;
    struct NodeStruct *next;
    struct NodeStruct *before;
} Node;

typedef struct QueueStruct {
    struct NodeStruct *head;
    struct NodeStruct *tail;
    int length;
} Queue;

Node* makeNode(queueType *value, Node *next, Node *before);

Queue* makeQueue();
void printQueue(Queue *queue, void (*printFunction) (queueType *));
queueType* dequeue(Queue *queue);
void queueAdd(Queue *queue, queueType *value);
void queueRemove(Queue *queue, Node *node);

#endif