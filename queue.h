#ifndef QUEUE
#define QUEUE
#include "process.h"


typedef struct NodeQueue
{
    Process* process;
    struct NodeQueue* next;
} NodeQueue;


typedef struct Queue{
    NodeQueue* first;
    NodeQueue* last;
} Queue;


NodeQueue* createNode(Process* process);
void destroyNode(NodeQueue* node);
Queue* createQueue();
void destroyQueue(Queue* queueN);
void push(Queue* queueN, Process* process);
Process* consult(Queue* queueN);
void  pop(Queue* queueN);
void searchHigher(Queue* queueN,char *mode);
void printQueue(Queue* queueN);

#endif

