#include <stdbool.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include "cliente.h"

#define MAXBUF 1024
#define USERNAME_BUFFER 40
#define TAMANO_IP 15

u_int16_t PORT;

int Burst1, Burst2, tasaCreacion1, tasaCreacion2;
int Prioridad;
bool corriendo = true;
int sockfd = 0;
char num[USERNAME_BUFFER];
int pId = 0;
int main(int argc, char const *argv[])
{
    printf("",sizeof(15));
    if (argc < 3)
    {
        printf("Se debe especificar el puerto y la direccion IP:...\n");
        printf("cliente_texto <IP> <PUERTO>:...\n");
        exit(1);
    }

    if ((PORT = atoi(argv[2])) <= 0)
    {
        printf("El puerto debe ser un número entero\n");
        exit(1);
    }
    char *IP = malloc(TAMANO_IP); // xxx.xxx.xxx.xxx
    for (int i = 0; i < TAMANO_IP; i++)
    {
        *(IP + i) = *(argv[1] + i);
    }
    
    struct sockaddr_in serv_addr;
    int  des_fd;
    int client_len, read_len, file_read_len;
    char buf[MAXBUF];

    /* socket() */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        return 1;
    }
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(IP);
    serv_addr.sin_port = htons(PORT);

    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
    {
        perror("connect : ");
        printf("fail to connect.\n");
        close(sockfd);
        return 1;
    }

    printf("Conectado con el servidor\n");
    printf("Direccion IP: %s\n", IP);
    printf("Puerto: %i\n", PORT);
    menu();
    return 0;
}

void menu(){
    printf("\nEscoge el algoritmo: \n");
    printf("0. FIFO\n");
    printf("1. SJF\n");
    printf("2. HPF\n");
    printf("3. Round Robin\n");
    printf("4. Salir\n");
    printf("Digite el numero de la accion deseada: ");
    int opcion = -1;
    scanf("%i", &opcion);
    int bytesEnviados = 0;
    if(opcion == 0){
        bytesEnviados = send(sockfd, "FIFO", MAXBUF, 0);
    }
    else if(opcion == 1){
        bytesEnviados = send(sockfd, "SJF", MAXBUF, 0);
    }
    else if(opcion == 2){
        bytesEnviados = send(sockfd, "HPF", MAXBUF, 0);
    }
    else if(opcion == 3){
        bytesEnviados = send(sockfd, "Round Robin", MAXBUF, 0);
        printf("Digite el quamtum deseado: ");
        scanf("%i", &opcion);
        if(bytesEnviados > 0){
            char quantum[USERNAME_BUFFER];
            sprintf(quantum, "%d", opcion);
            bytesEnviados = send(sockfd, quantum, MAXBUF, 0);
        }
    }
    else if(opcion == 4){
        exit(1);
    }
    else{
        printf("La opcion no existe.\n");
        menu();
        return;
    }

    if (bytesEnviados <= 0)
    {
        printf("No se pudo enviar la informcaion del algoritmo.\n");
        menu();
        return;
    }
        
    printf("\nEscoge alguna de las opciones: \n");
    printf("1. Cliente Manual\n");
    printf("2. Cliente Automatico\n");
    printf("3. Salir\n");
    printf("Digite el numero de la accion deseada: ");
    opcion = -1;
    scanf("%i", &opcion);
    if(opcion == 1){
        clienteManual();
    }
    else if(opcion == 2){
        clienteAutomatico();
    }
    else{
        exit(1);
    }
}

void *crearProceso(){
    int burst = (rand() % (Burst2+1-Burst1)) + Burst1;
    int prioridad = (rand() % 5) + 1;
    int bytesEnviados = 0;
    sprintf(num, "%d", burst);
    bytesEnviados = send(sockfd, num, MAXBUF, 0);
    if (bytesEnviados <= 0)
    {
        printf("No se pudo enviar la instruccion de descarga al servidor\n");
        return -1;
    }
    bzero(num, USERNAME_BUFFER);
    *num = prioridad + '0';
    bytesEnviados = send(sockfd, num, MAXBUF, 0);
    if (bytesEnviados <= 0)
    {
        printf("No se pudo enviar el nombre del archivo al servidor\n");
        return -1;
    }
    printf("Se ha recibido correctamente el proceso con el id: %d\n",pId);
    pId++;
    bzero(num, USERNAME_BUFFER);
}
void *crearProcesoManual(){
    int burst = Burst1;
    int priori = Prioridad;
    int bytesEnviados =0;
    sprintf(num, "%d", burst);
    bytesEnviados = send(sockfd, num, MAXBUF, 0);
    if (bytesEnviados <= 0)
    {
        printf("No se pudo enviar la instruccion de descarga al servidor\n");
        return -1;
    }
    bzero(num, USERNAME_BUFFER);
    *num = priori + '0';
    bytesEnviados = send(sockfd, num, MAXBUF, 0);
    if (bytesEnviados <= 0)
    {
        printf("No se pudo enviar el nombre del archivo al servidor\n");
        return -1;
    }
    printf("Se ha recibido correctamente el proceso con el id: %d\n",pId);
    pId++;
    bzero(num, USERNAME_BUFFER);
}

void *generarHilosProcesos(void *arg){
    time_t t;
    srand((unsigned) time(&t));
    while(corriendo){
        pthread_t hilo;
        if( 0 == pthread_create(&hilo, NULL, crearProceso, NULL))
            pthread_join(hilo,NULL);
        sleep((rand() % (tasaCreacion2+1-tasaCreacion1)) + tasaCreacion1);
    }
}

void *generarHilosProcesosManual(void *arg){
    pthread_t hilo;
    if( 0 == pthread_create(&hilo, NULL, crearProcesoManual, NULL))
            pthread_join(hilo,NULL);
    sleep(2);
}

void clienteAutomatico(){
    pthread_t hilo;
    printf("Rango de los valores del Burst ingrese los numeros separados por espacio: ");
    scanf("%d %d", &Burst1, &Burst2);
    printf("Ingrese el rango para el valor de la tasa de creacion: ");
    scanf("%d %d", &tasaCreacion1, &tasaCreacion2);
    if( 0 != pthread_create(&hilo, NULL, generarHilosProcesos, NULL)){
        return -1;
    }
    int opcion;
    printf("\nIngrese 0 para detener la simulacion: \n");
    scanf("%i", &opcion);
    if(opcion == 0){
        int bytesEnviados = 0;
        bytesEnviados = send(sockfd, "Detener", MAXBUF, 0);
        corriendo = false;
    }
    pthread_join(hilo,NULL);
}
void clienteManual(){
    int bytesEnviados=0;
    char mystring [1000];
    FILE* pFile;
    char c;
    char d;
    pFile = fopen ("./Manual.txt" , "r");
    if (pFile == NULL)
        exit(EXIT_FAILURE);
    while(fgets( mystring, 1000, pFile) != NULL){
      pthread_t hilo;
      int jj = -1;
      while(++jj < strlen(mystring)) {
        if ((c = mystring[jj]) != ' ') break;
      }
      int segundo = 0;
      while(++segundo < strlen(mystring)) {
        if ((d = mystring[segundo]) != ' ') break;
      }
      Burst1= (int)(c);
      Burst1 = Burst1-48;
      Prioridad=(int)(d);
      Prioridad= Prioridad-48;
      if( 0 != pthread_create(&hilo, NULL, generarHilosProcesosManual, NULL)){
            return -1;
        }
      sleep((rand() % (8-3+1))+3);
      pthread_join(hilo,NULL);
    }
    fclose (pFile);
    bytesEnviados = send(sockfd, "Detener", MAXBUF, 0);
}