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

int main(int argc, char *argv[])
{
	int serv_sock;
	int clnt_sock;
    int str_len;

	struct sockaddr_in serv_addr;
	struct sockaddr_in clnt_addr;
	socklen_t clnt_addr_size;

    Packet adr;
	
	if(argc!=2){
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}
	
	// 1단계: socket() 소켓 생성 
	serv_sock=socket(PF_INET, SOCK_STREAM, 0);
	if(serv_sock == -1)
		error_handling("socket() error");
	
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_addr.s_addr=htonl(INADDR_ANY);
	serv_addr.sin_port=htons(atoi(argv[1]));
	
	// 2단계: bind() 주소 연결 
	if(bind(serv_sock, (struct sockaddr*) &serv_addr, sizeof(serv_addr))==-1 )
		error_handling("bind() error"); 
	
	if(listen(serv_sock, 5)==-1)
		error_handling("listen() error");
	
	// 3단계: accept()
	clnt_addr_size=sizeof(clnt_addr);  
	clnt_sock=accept(serv_sock, (struct sockaddr*)&clnt_addr,&clnt_addr_size);
	if(clnt_sock==-1)
		error_handling("accept() error");  
	
	// 4단계: 클라이언트로 메시지 전송 
    printf("--------------------------------\n");
    printf("Address Conversion Server\n");
    printf("--------------------------------\n");
    while(1){
        str_len=read(clnt_sock, &adr, sizeof(adr)); //클라이언트로부터 정보를 받아옴
	    if(str_len==-1)
	        error_handling("read() error!");
        if(adr.cmd==2){ //cmd가 2면 종료
            printf("[RX] QUIT message received\n"); 
            break;
        }
        else{
            printf("[RX] Received Dotted-Decimal Address: %s\n", adr.addr); 
            //주소 변환해줌, Packet 구조체 in_addr 구조체에 넣어줌
            if(!inet_aton(adr.addr, &adr.iaddr)){ //실패하는 경우, 즉 주소가 잘못된 경우
                adr.cmd = 1;
                adr.result = 0;
                write(clnt_sock, &adr, sizeof(adr)); //클라이언트에게 수정된 구조체 보내기
                printf("[Tx] Address conversion fail: (%s)\n\n", adr.addr);
            }
            else{ //성공하는 경우, 즉 적절한 주소인 경우
                printf("inet_aton(%s) -> %#x \n", adr.addr, adr.iaddr.s_addr);
                adr.cmd = 1;
                adr.result = 1;
                write(clnt_sock, &adr, sizeof(adr)); //클라이언트에게 수정된 구조체 보내기
                printf("[Tx] cmd: %d, iaddr: %#x, result: %d\n\n", adr.cmd, adr.iaddr.s_addr, adr.result);
            }
              
        }
    }
	
    //통신 종료
	close(clnt_sock);	
	close(serv_sock);
    printf("Server socket close and exit.\n");
	return 0;
}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
