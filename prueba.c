#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void)
{
    

  FILE* filePointer;
  int bufferLength = 255;
  char buffer[bufferLength];

  filePointer = fopen("./Manual.txt", "r");

  while(fgets(buffer, bufferLength, filePointer)) {
    //printf("%s\n", buffer);
    char * token = strtok(buffer, " ");
   // loop through the string to extract all other tokens
   while( token != NULL ) {
      printf( " %s\n", token ); //printing each token
      token = strtok(NULL, " ");
   }

  }

  fclose(filePointer);
}