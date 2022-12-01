#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
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

int main(int argc, char *argv[])
{
    int serv_sock;
    int clnt_sock;
    
    struct sockaddr_in serv_addr;
    struct sockaddr_in clnt_addr;
    socklen_t clnt_addr_size;
    Packet packet;
    packet.seq = SEQ_START;     // 초기값 설정

    if(argc!=2){
        printf("Usage : %s <port>\n", argv[0]);
    exit(1);
    }

    // 1단계: 소켓 생성
    serv_sock=socket(PF_INET, SOCK_STREAM, 0);
    if(serv_sock == -1)
    error_handling("socket() error");

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family=AF_INET;
    serv_addr.sin_addr.s_addr=htonl(INADDR_ANY);
    serv_addr.sin_port=htons(atoi(argv[1]));
   
    // 2단계: bind
    if(bind(serv_sock, (struct sockaddr*) &serv_addr,sizeof(serv_addr))==-1 )
        error_handling("bind() error");
    
    // 3단계: listen
    if(listen(serv_sock, 5)==-1)
        error_handling("listen() error");
    clnt_addr_size=sizeof(clnt_addr);

     

    printf("---------------------------------------------------\n\t");
    printf("File Transmission Server\t\n---------------------------------------------------\n");
    
     // 4단계: accept
    clnt_sock=accept(serv_sock, (struct sockaddr*)&clnt_addr,&clnt_addr_size);
    if(clnt_sock==-1)
        error_handling("accept() error");   

    int fd;
    int fd2;
    int num;
    int total=0;
    int check_num = 1;

    char buf2[BUF_SIZE];
    char message[BUF_SIZE] = "\0";
    int fname;

    memset(&packet.buf, '\0', BUF_SIZE);
    
    fname = read(clnt_sock, packet.buf, BUF_SIZE); //파일 이름 받기

    printf("[Server] sending %s \n\n", packet.buf);
    
    fd=open(packet.buf, O_RDONLY,0644); //읽을 파일 열기

    if(fd ==-1){
        printf("%s File Not Found\n", packet.buf);
        
        char check[BUF_SIZE]="File Not Found";
        write(clnt_sock, check, sizeof(check));

        exit(1);
    }else{
        char check[BUF_SIZE]="\0";
        write(clnt_sock, check, sizeof(check));
    }
    
    strcpy(message, packet.buf);
    memset(&packet.buf, '\0', BUF_SIZE);

    while(1){   
        
        num = read(fd, packet.buf, sizeof(packet.buf)); //파일 읽기
        total = total + num;    //총 바이트 수 저장
        
        if(num == BUF_SIZE){
            packet.buf[(strlen(packet.buf))] = '\0'; 
        }
        
        printf("[Server] Tx: SEQ:%d,\t%d byte data\n", packet.seq, num);
        write(clnt_sock, (char*)(&packet.seq), 4);     //seq 넘겨주기
        write(clnt_sock, (char*)(&num),4);              //data 크기 넘겨주기 (byte)

        int server_ack= packet.seq + num + 1;              //ACK 계산
        read(clnt_sock, (char*)(&packet.ack), 4);          //ACK 받아오기
        
        write(clnt_sock, packet.buf, num); //데이터 넘겨주기
        
       // printf("%s", packet.buf);  

        if(num!=100){
            printf("%s sent (%d Bytes)\n", message, total);
            break;
        }else{
            if(server_ack == packet.ack){
            printf("[Server] Rx ACK: %d\n\n", packet.ack);     //ACK 
            }
        }   

        packet.seq = packet.ack;
        memset(&packet.buf, '\0', BUF_SIZE);
    }
        memset(&message, '\0', BUF_SIZE);
    
    close(clnt_sock);
    close(serv_sock);
}

void error_handling(char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
}
