#include "process.h"


Process * createProcess(int pid,int burst,int priority,float wt,float tat,float arrivalTime,float departureTime,char *state){
    PCB* pcb = (PCB * )malloc(sizeof(PCB));
    pcb->burst = burst;
    pcb->priority = priority;
    pcb->wt = wt;
    pcb->tat = tat;
    pcb->arrivalTime = arrivalTime;
    pcb->departureTime = departureTime;
    Process* process = (Process * )malloc(sizeof(Process));
    process->pid = pid;
    process->pcb = pcb;
    process->state = state;
    return process;
}

