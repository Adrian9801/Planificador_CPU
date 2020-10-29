#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void)
{
    

  FILE* filePointer;
  int bufferLength = 255;
  char buffer[bufferLength];
  int contador=0;
  int a;
  int b;

  filePointer = fopen("/media/sf_Proyecto1/Manual.txt", "r");

  while(fgets(buffer, bufferLength, filePointer)) {
    char * token = strtok(buffer, " ");
   while( token != NULL ) {
      if(contador==0){
      printf("Primero de la Linea ");
      //printf("%s",token ); //printing each token
      a=atoi(token);
      printf("%d",a);
      printf("\n");
      token = strtok(NULL, " ");
      contador++;
      }
      if(contador==1){
        printf("Segundo de la Linea ");
        //printf("%s",token ); //printing each token
        b=atoi(token);
        printf("%d",b);
        printf("\n");
        token = strtok(NULL, " ");
        contador=0;
      }
   }

  }

  fclose(filePointer);
}