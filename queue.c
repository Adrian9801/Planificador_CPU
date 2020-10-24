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

void printQueue(Queue* queueN){
    if(queueN->first){
        NodeQueue* auxNode =  queueN->first;
        while (auxNode->next != NULL){
            printf("PID is : %d \n",auxNode->process->pid);
            auxNode = auxNode->next;
        }
        printf("PID is : %d \n",auxNode->process->pid);
    }
}


void searchHighest(Queue* queueN,char *mode){
    if(queueN->first){
        NodeQueue* auxNode =  queueN->first;
        NodeQueue* auxHighest =  queueN->first;
        NodeQueue* auxBack  ;
        int flag = 0;
        if(mode == "SJF" ){
            int value = auxNode->process->pcb->burst;
            while (auxNode->next != NULL)
            {
                if(value > auxNode->next->process->pcb->burst){
                    value = auxNode->next->process->pcb->burst;
                    auxHighest = auxNode->next;
                    auxBack = auxNode;
                    auxNode = auxNode->next;
                    flag++;
                }else{
                    auxNode = auxNode->next;
                }
            }  
            if (flag > 0){

                if(auxHighest->next == NULL){
                    queueN->last = auxBack;
                }
                auxBack->next = auxHighest->next;
                auxHighest->next = queueN->first;
                queueN->first = auxHighest;
            }   
        }
    
        else if(mode == "HPF"){
            int value = auxNode->process->pcb->priority;
                while (auxNode->next != NULL)
                {
                    if(value > auxNode->next->process->pcb->priority){
                        value = auxNode->next->process->pcb->priority;
                        auxHighest = auxNode->next;
                        auxBack = auxNode;
                        auxNode = auxNode->next;
                        flag++;
                    }else{
                        auxNode = auxNode->next;
                    }
                }  
                if (flag > 0){

                    if(auxHighest->next == NULL){
                        queueN->last = auxBack;
                    }
                    auxBack->next = auxHighest->next;
                    auxHighest->next = queueN->first;
                    queueN->first = auxHighest;
                }
        }   
    }else
    {
        printf("No hay elementos en la cola!");
    } 
}


