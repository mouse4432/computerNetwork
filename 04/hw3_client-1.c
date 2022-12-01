#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#define BUF_SIZE 100
#define SEQ_START 1000

typedef struct {
int seq; // SEQ number
int ack; // ACK number
int buf_len; // File read/write bytes
char buf[BUF_SIZE];
}Packet;

void error_handling(char *message);

int main(int argc, char* argv[])
{
    int sock;
    struct sockaddr_in serv_addr;
    Packet packet;

    if(argc!=3){
        printf("Usage : %s <IP> <port>\n", argv[0]);
        exit(1);
    }

    // 소켓 생성
    sock=socket(PF_INET, SOCK_STREAM, 0);
    
    if(sock == -1)
     error_handling("socket() error");
    memset(&serv_addr, 0, sizeof(serv_addr));

    serv_addr.sin_family=AF_INET;
    serv_addr.sin_addr.s_addr=inet_addr(argv[1]);
    serv_addr.sin_port=htons(atoi(argv[2]));
   
    // 연결 요청
    if(connect(sock, (struct sockaddr*)&serv_addr,sizeof(serv_addr))==-1)
            error_handling("connect() error!");
    
    int recv_len = 0;
    int str_len = 0;
    int recv_cnt = 0;
    int fd2;
    int total = 0;
    char check[BUF_SIZE] = "\0";
    char data[BUF_SIZE];
    char message[BUF_SIZE] ="\0";
    
        printf("Input file name : ");
        fgets(message, BUF_SIZE, stdin);    // 포트 받기
        
        printf("[Client] request %s\n", message);
        message[(strlen(message)-1)]='\0';

        write(sock, message, strlen(message));    //파일 이름 전달하기
        
        int check_num = read(sock, check, BUF_SIZE);
        
        if(!strcmp(check, "File Not Found")){
            printf("%s", check);
            exit(1);
        }else{
            fd2=open(message, O_CREAT|O_WRONLY|O_TRUNC, 0644);  // 작성할 파일 열기 
        }

        recv_len = 0;
        while(1){

            read(sock, (char*)(&packet.seq), 4); // seq 크기 받아오기
            read(sock, (char*)(&packet.buf_len), 4); // 데이터 크기 받아오기

            total = total + packet.buf_len;

            printf("[Client] Rx SEQ: %d,\t len: %d bytes\n", packet.seq, packet.buf_len);
            packet.ack = packet.seq + packet.buf_len + 1;
          
            write(sock, (char*)(&packet.ack), 4); //ack 계산값 넘겨주기

            recv_cnt = read(sock, &packet.buf, packet.buf_len);  // 데이터 받아오기
            write(fd2, &packet.buf, packet.buf_len);

           // printf("%s \n\n", packet.buf); 
            memset(&packet.buf, '\0', BUF_SIZE);

            if(packet.buf_len!=100){
                printf("%s received (%d Bytes)\n", message, total);
                break;
            }else{
                printf("[Client] Tx ACK: %d\n\n", packet.ack);
                
            }
            
            packet.seq = packet.ack;
        }

        
    
    close(sock);
    return 0;
}

void error_handling(char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}