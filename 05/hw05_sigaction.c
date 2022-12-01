#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

 //자식, 부모 프로세스에 사용할 초
 //#define 사용해서 상수 써라
int child = 5;
int parent = 2;
int i=5;
int j=2;

//자식프로세스에서 시그널 보내면 실행할 childTimeout함수
void childTimeout(int sig)
{
    if(sig==SIGALRM)           
        printf("[Child] time out: %2d, elapsed time: %2d seconds\n", child, i);
    i+=child;
    alarm(5);
}

//부모프로세스에서 시그널 보내면 실행할 parentTimeout함수
void parentTimeout(int sig)
{
    if(sig==SIGALRM)           
        printf("<Parent> timeout: %2d, elapsed time: %2d seconds\n", parent, j);
    j+=parent;
    alarm(2);
}

//자식프로세스 종료되면 실행할 childExit함수
void childExit(int sig)
{
   int status;
   pid_t id = waitpid(-1, &status, WNOHANG);
   if(WIFEXITED(status)){
        printf("Child id: %d, sent: %d\n", id, WEXITSTATUS(status));
   }
}

int main(int argc, char* argv[])
{
    //pid 생성
    pid_t pid;

    //자식 프로세스 종료되면 실행 시그널 등록
    struct sigaction childFin;
    childFin.sa_handler = childExit;
    sigemptyset(&childFin.sa_mask);
    childFin.sa_flags = 0;
    sigaction(SIGCHLD, &childFin, 0);

    //fork로 자식 프로세스 생성
    printf("\n");
    pid = fork();
    
    //자식 프로세스와 부모 프로세스에서 사용할 act 선언
    struct sigaction act;
    sigemptyset(&act.sa_mask);
    act.sa_flags=0;

    if(pid==-1){ //-1이면 잘못된 값이므로 강제 종료
        exit(1);
    }
    else if(pid==0){ //자식 프로세스
        act.sa_handler = childTimeout;
        sigaction(SIGALRM, &act, 0);
        alarm(5);
        for(int k = 0; k<20; k++){
            sleep(1);
        }    
        return 5;
    }
    else{ //부모 프로세스
        act.sa_handler = parentTimeout;
        sigaction(SIGALRM, &act, 0);
        alarm(2);
        while(1){
            sleep(1);
        } 
    }
    return 0;
}
