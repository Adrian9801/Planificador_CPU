#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <strings.h>
#include <string.h>
#include <dirent.h>
#include <time.h>
#include "queue.c"
#include "process.c"
#include "server.h"
#include <stdbool.h>

#define TAMANO_BUFFER 1024
#define COLA_SERVIDOR 10

u_int16_t PUERTO_SERVIDOR;
time_t tiempo;
clock_t ta;
int pId;
bool run = true;
char algoritmo[TAMANO_BUFFER];
float quantum;
int cantProcesos;
float promedioTAT;
float promedioWT;
int tiempoOcioso;
// mutex para los hilos
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

Queue* colaReady;

Queue* colaFinal;

int main(int argc, char const *argv[])
{
    if (argc < 2)
    {
        printf("Se debe especificar el puerto...\n");
        exit(1);
    }

    if ((PUERTO_SERVIDOR = atoi(argv[1])) <= 0)
    {
        printf("El puerto debe ser un número entero\n");
        exit(1);
    }
    colaReady = createQueue();
    colaFinal = createQueue();
    cantProcesos = 0;
    promedioTAT = 0;
    promedioWT = 0;
    tiempoOcioso = 0;

    int socketServidor;
    struct sockaddr_in servaddr;
    struct dirent *de;

    // se crea el socket del servidor
    if ((socketServidor = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        printf("No se pudo crear el socket\n");

    printf("Socket para el servidor creado...\n");

    // se pone en cero la variabla de la direccion del servidor
    bzero(&servaddr, sizeof(servaddr));

    // se inicializa la variable de direccion del servidor
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PUERTO_SERVIDOR);

    // enlazar el socket con la dirección local
    if ((bind(socketServidor, (struct sockaddr *)&servaddr, sizeof(servaddr))) < 0)
    {
        printf("No se pudo enlazar el socket con la dirección\n");
        exit(1);
    }
    printf("Enlace con el puerto realizado\nPuerto: %i\n", PUERTO_SERVIDOR);

    // poner el servidor a escuchar
    if (listen(socketServidor, COLA_SERVIDOR) < 0)
    {
        printf("No se pudo comerzar a escuchar por conexiones\n");
        exit(1);
    }
    // ciclo infinito para aceptar conexiones
    int clienteFd;
    printf("Escuchando en puerto %i\n", PUERTO_SERVIDOR);

    // Aceptamos la conexion de un cliente
    clienteFd = accept(socketServidor, (struct sockaddr *)NULL, NULL);
    if (clienteFd < 0)
    {
        printf("No se pudo conectar con el cliente\n");
    }
    else{
        int *clientePtr = malloc(sizeof(int));
        *clientePtr = clienteFd;

        pthread_t clienteHilo;
        pthread_t CPUHilo;
        pId = 0;
        tiempo = time(NULL);
        pthread_create(&clienteHilo, NULL, jobScheduler, clientePtr);
        pthread_create(&CPUHilo, NULL, CPUScheduler, NULL);
        pthread_join(clienteHilo,NULL);
        pthread_join(CPUHilo,NULL);
        close(socketServidor);
    }
    return 0;
}

void *jobScheduler(void *ptrCliente)
{
    // registro de bytes enviado en cada paquete
    int enviadosPaquete = 0;

    // se crea una variable local para poder liberar el puntero
    int socketCliente = *((int *)ptrCliente);
    free(ptrCliente);

    char prioridad[TAMANO_BUFFER];
    char charQuantum[TAMANO_BUFFER];
    char burst[TAMANO_BUFFER];
    int bRead = 0;

    bRead = read(socketCliente, algoritmo, TAMANO_BUFFER);
    if (bRead < 0)
    {
        printf("No se pudo obtener el algoritmo deseado\n");
        jobScheduler(ptrCliente);
        return;
    }
    printf("El algoritmo a utilizar es: %s\n", algoritmo);
    if(strcmp(algoritmo,"Round Robin") == 0){
        bRead = read(socketCliente, charQuantum, TAMANO_BUFFER);
        if (bRead < 0)
        {
            printf("No se pudo obtener el quantum deseado\n");
            jobScheduler(ptrCliente);
            return;
        }
        quantum = atoi(charQuantum);
        printf("El quantum es de: %s\n", charQuantum);
    }
    while (run)
    {
        bzero(burst, TAMANO_BUFFER);
        // leer la solicitud del cliente
        bRead = 0;
        bRead = read(socketCliente, burst, TAMANO_BUFFER);
        if (bRead < 0)
        {
            printf("No se pudo obtener la instruccion del cliente\n");
            sleep(1);
            continue;
        }
        else if (bRead == 0)
        {
            printf("Se obtuvo un mensaje nulo del servidor\n");
            break;
        }
        else{
            /**
             * Se recibe el nombre del usuario para ingresarlo en la lista
             * En caso de que ya se encuentre pasar al envio de donde quedo
             * la descarga pasada (falta por implementar)
            */
            if(strcmp(burst,"Detener") == 0){
                printf("Fin");
                run = false;
                break;
            }
            else if(strcmp(burst,"Cola") == 0){
                pthread_t verColaHilo;
                pthread_create(&verColaHilo, NULL, verCola, NULL);
                continue;
            }
            while(true){
                bzero(prioridad, TAMANO_BUFFER);
                if ((bRead = read(socketCliente, prioridad, TAMANO_BUFFER)) < 0)
                {
                    printf("No se pudo obtener la prioridad\n");
                    run = false;
                    break;
                }
                else if(strcmp(prioridad,"Detener") == 0){
                    printf("Fin");
                    run = false;
                    break;
                }
                else if(strcmp(prioridad,"Cola") == 0){
                    pthread_t verColaHilo;
                    pthread_create(&verColaHilo, NULL, verCola, NULL);
                    continue;
                }
                ta = time(NULL);
                float tl = ta-tiempo;
                Process* process = createProcess(pId,atoi(burst),atoi(prioridad),0,0,tl,0,"en ready");
                push(colaReady,process);
                pId++;
                break;
            }
        }
    }

    close(socketCliente);
    run = false;
    return NULL;
}

void *verCola(){
    Process* prove = consultElement(colaReady,0);
    for (int i = 1 ;prove != NULL; i++)
    {
        printf("Proceso: %d Burst: %d Prioridad: %d\n",prove->pid,prove->pcb->burst,prove->pcb->priority);
        prove = consultElement(colaReady,i);
    }
}

void *CPUScheduler()
{
    searchHighest(colaReady,algoritmo);
    Process* prove = consult(colaReady);
    float tiempoBurst = 0;
    printf("\n");
    while(run){
        if(prove != NULL){
            pop(colaReady);
            if(strcmp(algoritmo,"Round Robin") == 0){
                int restante = prove->pcb->burst - prove->pcb->departureTime;
                if(quantum < restante){
                    sleep(quantum);
                    prove->pcb->departureTime = prove->pcb->departureTime + quantum;
                    push(colaReady,prove);
                    searchHighest(colaReady,algoritmo);
                    prove = consult(colaReady);
                    restante = prove->pcb->burst - prove->pcb->departureTime;
                    printf("Context switch\n");
                    printf("Proceso: %d Burst: %d Prioridad: %d entra en ejecucion\n",prove->pid,restante,prove->pcb->priority);
                }
                else
                {
                    sleep(restante);
                    ta = time(NULL);
                    prove->pcb->departureTime = ta-tiempo;
                    prove->pcb->tat = prove->pcb->departureTime - prove->pcb->arrivalTime;
                    prove->pcb->wt = prove->pcb->tat - prove->pcb->burst;
                    cantProcesos++;
                    promedioTAT += prove->pcb->tat;
                    promedioWT += prove->pcb->wt;
                    tiempoOcioso += prove->pcb->burst;
                    prove->state = "Terminado";
                    printf("\nproceso: %d burst: %d prioridad: %d WT: %f TAT: %f\n Tiempo llegada: %f Tiempo salida: %f estado: %s\n",prove->pid,prove->pcb->burst,prove->pcb->priority,prove->pcb->wt,prove->pcb->tat,prove->pcb->arrivalTime,prove->pcb->departureTime,prove->state);
                    push(colaFinal,prove);
                    searchHighest(colaReady,algoritmo);
                    prove = consult(colaReady);
                }
                
            }
            else{
                sleep(prove->pcb->burst);
                ta = time(NULL);
                prove->pcb->departureTime = ta-tiempo;
                prove->pcb->tat = prove->pcb->departureTime - prove->pcb->arrivalTime;
                prove->pcb->wt = prove->pcb->tat - prove->pcb->burst;
                prove->state = "Terminado";
                cantProcesos++;
                promedioTAT += prove->pcb->tat;
                promedioWT += prove->pcb->wt;
                tiempoOcioso += prove->pcb->burst;
                push(colaFinal,prove);
                printf("\nproceso: %d burst: %d prioridad: %d WT: %f TAT: %f\n Tiempo llegada: %f Tiempo salida: %f estado: %s\n",prove->pid,prove->pcb->burst,prove->pcb->priority,prove->pcb->wt,prove->pcb->tat,prove->pcb->arrivalTime,prove->pcb->departureTime,prove->state);
                searchHighest(colaReady,algoritmo);
                prove = consult(colaReady);
            }
        }
        else{
            searchHighest(colaReady,algoritmo);
            prove = consult(colaReady);
        }
    }
    ta = time(NULL);
    tiempoOcioso= (ta-tiempo)-tiempoOcioso;
    promedioTAT = promedioTAT/cantProcesos;
    promedioWT = promedioWT/cantProcesos;
    printf("\nCantidad de procesos terminados: %d\n",cantProcesos);
    printf("Tiempo de CPU ocioso: %d\n",tiempoOcioso);
    prove = consult(colaFinal);
    while(prove != NULL){
        pop(colaFinal);
        printf("Proceso: %d TAT: %f WT: %f\n",prove->pid,prove->pcb->tat,prove->pcb->wt);
        prove = consult(colaFinal);
    }
    printf("Promedio de TAT: %f\n",promedioTAT);
    printf("Promedio de WT: %f\n",promedioWT);
}