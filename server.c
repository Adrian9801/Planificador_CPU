/**
 * Para la lectura de un archivo se toma como referencia:
 * https://www.geeksforgeeks.org/c-program-list-files-sub-directories-directory/
 */
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

Queue* newQueue;

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
    newQueue = createQueue();
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
        quantum = atof(charQuantum);
        printf("El quantum es de: %s\n", quantum);
    }
    while (1)
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
        }
        else{
            sleep(1);
            /**
             * Se recibe el nombre del usuario para ingresarlo en la lista
             * En caso de que ya se encuentre pasar al envio de donde quedo
             * la descarga pasada (falta por implementar)
            */
            bzero(prioridad, TAMANO_BUFFER);
            if(strcmp(burst,"Detener") == 0){
                printf("Fin");
                break;
            }
            // recibir el nombre del usuario
            if ((bRead = read(socketCliente, prioridad, TAMANO_BUFFER)) < 0)
            {
                printf("No se pudo obtener la prioridad\n");
                continue;
            }
            else if(strcmp(prioridad,"Detener") == 0){
                printf("Fin");
                break;
            }
            ta = time(NULL);
            float tl = ta-tiempo;
            Process* process = createProcess(pId,atoi(burst),atoi(prioridad),0,0,tl,0,"en ready");
            push(newQueue,process);
            printf("El burst es de: %s, la prioridad es de: %s y el tiempo: %f\n", burst, prioridad,tl);
            pId++;
        }
    }

    close(socketCliente);
    run = false;
    return NULL;
}

void *CPUScheduler()
{
    searchHighest(newQueue,algoritmo);
    Process* prove = consult(newQueue);
    float tiempoBurst = 0;
    printf("\n");
    while(run || prove != NULL){
        if(prove != NULL){
            pop(newQueue);
            if(strcmp(algoritmo,"Round Robin") == 0){
                float restante = prove->pcb->burst - prove->pcb->departureTime;
                if(quantum < restante){
                    sleep(quantum);
                    prove->pcb->departureTime = prove->pcb->departureTime + quantum;
                    push(newQueue,prove);
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
                    prove->state = "Finished";
                    printf("proceso: %d burst: %d prioridad: %d WT: %f TAT: %f\n Tiempo llegada: %f Tiempo salida: %f estado: %s\n",prove->pid,prove->pcb->burst,prove->pcb->priority,prove->pcb->wt,prove->pcb->tat,prove->pcb->arrivalTime,prove->pcb->departureTime,prove->state);
                    push(colaFinal,prove);
                }
                
            }
            else{
                sleep(prove->pcb->burst);
                ta = time(NULL);
                prove->pcb->departureTime = ta-tiempo;
                prove->pcb->tat = prove->pcb->departureTime - prove->pcb->arrivalTime;
                prove->pcb->wt = prove->pcb->tat - prove->pcb->burst;
                prove->state = "Finished";
                cantProcesos++;
                promedioTAT += prove->pcb->tat;
                promedioWT += prove->pcb->wt;
                tiempoOcioso += prove->pcb->burst;
                push(colaFinal,prove);
                printf("proceso: %d burst: %d prioridad: %d WT: %f TAT: %f\n Tiempo llegada: %f Tiempo salida: %f estado: %s\n",prove->pid,prove->pcb->burst,prove->pcb->priority,prove->pcb->wt,prove->pcb->tat,prove->pcb->arrivalTime,prove->pcb->departureTime,prove->state);
            }
        }
        searchHighest(newQueue,algoritmo);
        prove = consult(newQueue);
    }
    ta = time(NULL);
    tiempoOcioso= (ta-tiempo)-tiempoOcioso;
    promedioTAT = promedioTAT/cantProcesos;
    promedioWT = promedioWT/cantProcesos;
}