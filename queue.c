#include <stdlib.h>
#include "queue.h"

NodeQueue* createNode(Process* process){
    NodeQueue* node = (NodeQueue * )malloc(sizeof(NodeQueue));
    node->process= process;
    node->next = NULL;
    return node;
}

void destroyNode(NodeQueue* node){
    node->process = NULL;
    node->next = NULL;
    free(node);
}

Queue* createQueue(){
    Queue* newQueue = (Queue * )malloc(sizeof(Queue));
    newQueue->first = newQueue->last = NULL;
    return newQueue;
}

void push(Queue* queueN, Process* process){
    NodeQueue* node = createNode(process);
    if(!queueN->first){
        queueN->first = node;
        queueN->last = node;
    }else{
        queueN->last->next = node;
        queueN->last = node;
    }
}

Process* consult(Queue* queueN){
    if(queueN->first){
        return queueN->first->process;
    }else{
        return NULL;
    }
}

void pop(Queue* queueN){
    if(queueN->first){
        NodeQueue* deleted =  queueN->first;
        queueN->first = queueN->first->next;
        destroyNode(deleted);
        if(!queueN->first){
            queueN->last = NULL;
        }
    }
}

void destroyQueue(Queue* queueN){
    while(queueN->first){
        pop(queueN);
    }
    free(queueN);
}


