#ifndef PROCESSSTRUCT
#define PROCESSSTRUCT
#include "pcb.h"

typedef struct Process
{   
    int pid;
    struct PCB* pcb;
    char *state;
}Process;

Process * createProcess(int pid,int burst,int priority,float wt,float tat,float arrivalTime,float departureTime,char *state );


#endif
