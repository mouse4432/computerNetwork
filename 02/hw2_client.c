//2021112751 김근찬
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

typedef struct{
    int cmd; //0: request, 1: response, 2: quit
    char addr[20]; //dotted-decimal address 저장 (20bytes)
    struct in_addr iaddr; //inet_aton() result 저장
    int result; //0: Error, 1: Success
} Packet;


void error_handling(char *message);

int main(int argc, char* argv[])
{
	int sock;
	struct sockaddr_in serv_addr;
	Packet adr; //주소를 담을 구조체
	int str_len;
	
	if(argc!=3){
		printf("Usage : %s <IP> <port>\n", argv[0]);
		exit(1);
	}
	
    //소켓 생성
	sock=socket(PF_INET, SOCK_STREAM, 0); 

	if(sock == -1)
		error_handling("socket() error");
	
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_addr.s_addr=inet_addr(argv[1]);
	serv_addr.sin_port=htons(atoi(argv[2]));

    //서버와 연결	
	if(connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr))==-1) 
		error_handling("connect() error!");
	
    //서버와 통신
    while(1){

    //클라이언트 기능
        printf("Input dotted-decimal address: ");
        scanf("%s", adr.addr); //dotted-decimal 형태의 주소를 입력 받기
        if(!strcmp(adr.addr, "quit")){ //종료 조건
            adr.cmd = 2;
            printf("[TX] cmd: %d(QUIT)\n", adr.cmd);
            write(sock, &adr, sizeof(adr));
            break;
        }
        adr.cmd = 0; 
        printf("[TX] cmd: %d, addr: %s\n", adr.cmd, adr.addr);

        write(sock, &adr, sizeof(adr)); //서버로 PACKET adr 전송
	    str_len=read(sock, &adr, sizeof(adr)); //서버에서 변경된 adr 받아옴
	    if(str_len==-1)
	        error_handling("read() error!");
        if(adr.result==0) //잘못된 주소 기입한 경우
            printf("[RX] cmd: %d, Address conversion fail! (result: %d)\n\n", adr.cmd, adr.result);
        else{ //정확하게 기입한 경우
            printf("[RX] cmd: %d, Address conversion: %#x (result: %d)\n\n", adr.cmd, adr.iaddr.s_addr, adr.result);
        }
	
    }
	
	close(sock);

    printf("Clients socket close and exit\n");
	return 0;
}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
