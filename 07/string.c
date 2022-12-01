#include <stdio.h>
#include <string.h>

int main(int argc, char* argv[]){
    char buf[50];
    char message[30];
    char a[2] = "[\0";
    char b[5] = "] \0";
    strcpy(buf, a);
    strcat(buf, argv[1]);
    strcat(buf, b);
    fgets(message, 30, stdin);
    message[strlen(message)-1] = '\0';
    strcat(buf, message);
    printf("%s", buf);
}