#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/select.h>

#define BUF_SIZE 2048
void error_handling(char *buf);

int main(int argc, char *argv[])
{
    int serv_sock, clnt_sock, send, receive; //clnt_sock에 기능마다 send인지 receive인지 분별
    struct sockaddr_in serv_adr, clnt_adr;
    struct timeval timeout; 
    fd_set reads, cpy_reads; //파일 디스크립터 저장한 곳

    socklen_t adr_sz;
    int fd_max, str_len, fd_num, i;
    char buf[BUF_SIZE]; //통신할 때 주고받을 buf
    int func; //clnt type정해주는 func, 기능

    memset(buf, 0, strlen(buf)); //buf 초기화

    if(argc != 2) {
        printf("Usage: %s <port>\n", argv[0]);
        exit(1);
    }

    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_adr.sin_port = htons(atoi(argv[1]));

    if(bind(serv_sock, (struct sockaddr*)&serv_adr, sizeof(serv_adr)) == -1)
        error_handling("bind() error");

    if(listen(serv_sock, 5) == -1)
        error_handling("listen() error");
    
    FD_ZERO(&reads); //reads초기화
    FD_SET(serv_sock, &reads); //serv_sock 등록
    fd_max = serv_sock; 

    while(1)
    {
        cpy_reads = reads; //cpy_reads에 복사
        //timeout 설정
        timeout.tv_sec = 5; 
        timeout.tv_usec = 5000;

        if((fd_num = select(fd_max+1, &cpy_reads, 0, 0, &timeout))== -1)
            break;
        if(fd_num == 0)
            continue;
        
        if(FD_ISSET(serv_sock, &cpy_reads))
        {      
            adr_sz = sizeof(clnt_adr);
            clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_adr, &adr_sz); //clnt연결
            printf("connected client: %d\n", clnt_sock); 
            FD_SET(clnt_sock, &reads); //clnt 등록
            if(fd_max < clnt_sock)
                fd_max = clnt_sock;
            str_len = read(clnt_sock, &func, sizeof(func)); //clnt로부터 기능 입력
            if(func == 1)
                send = clnt_sock; 
            else if(func == 2)
                receive = clnt_sock;
            else{
                //clnt종료
                 FD_CLR(clnt_sock, &reads);
                 close(clnt_sock);
            }
        }
        else if(FD_ISSET(send, &cpy_reads)){ //send에 무슨 일 발생
            str_len = read(send, buf, BUF_SIZE); //send로부터 buf 받아옴
            if(str_len == 0){
                //send 종료
                FD_CLR(send, &reads);
                close(send);
                printf("closed client: %d\n", send);
            }
            else{
                //forward 출력 후 receive로 buf 보냄
                printf("Forward [%d] ---> [%d]\n", send, receive);
                write(receive, buf, str_len);
            }
        }
        else if(FD_ISSET(receive, &cpy_reads)){ //receive에 무슨 일 발생
            str_len = read(receive, buf, BUF_SIZE); //receive로부터 buf 받아옴
            if(str_len == 0){
                //receive 종료
                FD_CLR(receive, &reads);
                close(receive);
                printf("closed client: %d\n", receive);
            }
            else{
                //backward 출력 및 send로 다시 buf 보냄
                printf("Backward [%d] <--- [%d]\n", receive, send);
                write(send, buf, str_len);
            }
        }
    }

    //serv종료
    FD_CLR(serv_sock, &reads);
    close(serv_sock);
   
    return 0;
}

void error_handling(char *buf)
{
    fputs(buf, stderr);
    fputc('\n', stderr);
    exit(1);
}
