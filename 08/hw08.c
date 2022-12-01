//2021112751 김근찬
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#define BUF_SIZE 100

void error_handling(char *message);

int main(int argc, char* argv[])
{
    int fd1, fd2;
    FILE *fp1;
    FILE *fp2;
    char buf1[BUF_SIZE];
    char buf2[BUF_SIZE];
    int idx = 0;

    fd1 = open("data1.txt", O_RDONLY);
    if(fd1==-1)
        error_handling("fileopen() error");
    
    fd2 = dup(fd1);

    fp1 = fdopen(fd1, "r");

    fp2 = fdopen(fd2, "r");

    while(!feof(fp1)){
        if(idx%2==0){
            fgets(buf1, BUF_SIZE, fp1);
            fputs(buf1, stdout);
            fflush(fp1);
        }
        else{
            fgets(buf2, BUF_SIZE, fp2);
            fputs(buf2, stdout);
            fflush(fp2);
        }
        idx++;
    }

    fclose(fp1);
    fclose(fp2);
    close(fd1);
    close(fd2);
    return 0;
}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}