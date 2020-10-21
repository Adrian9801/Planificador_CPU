#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void)
{
    char mystring [1000];
    FILE* pFile;
    char c;
    char d;
    int a;
    int b;
    pFile = fopen ("/media/sf_Proyecto1/Manual.txt" , "r");
    if (pFile == NULL)
        exit(EXIT_FAILURE);
    while(fgets( mystring, 1000, pFile) != NULL){
      int jj = -1;
      while(++jj < strlen(mystring)) {
        if ((c = mystring[jj]) != ' ') break;
      }
      int segundo = 0;
      while(++segundo < strlen(mystring)) {
        if ((d = mystring[segundo]) != ' ') break;
      }
      printf("%c", c);
      printf("\t");
      printf("%c",d);
      a=(int)(c);
      b=(int)(d);
      a=a-48;
      b=b-48;
      printf("\t");
      printf("%d",a+b);
      printf("\n");
    }

    fclose (pFile);
}