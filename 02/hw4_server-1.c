//김근찬 2021112751
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <fcntl.h> //file_open,close

#define BUF_SIZE 100
#define SEQ_START 1000
typedef struct{
    int seq; //SEQ number
    int ack; //ACK number 
    int buf_len; //File read/write bytes
    char buf[BUF_SIZE]; //파일이름 또는 파일 내용 전송
}Packet;

void error_handling(char *message);

int main(int argc, char *argv[])
{
	int serv_sock;
	int clnt_sock;
    int str_len;
    int file;
	int total = 0;
	char file_name[20];
	char message[20];

	struct sockaddr_in serv_addr;
	struct sockaddr_in clnt_addr;
	socklen_t clnt_addr_size;

    Packet data;
	
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
	
	printf("--------------------------------------\n");
    printf("     File Transmission Server\n");
    printf("--------------------------------------\n");
	
	// 3단계: accept()
	clnt_addr_size=sizeof(clnt_addr);  
	clnt_sock=accept(serv_sock, (struct sockaddr*)&clnt_addr,&clnt_addr_size);
	if(clnt_sock==-1)
		error_handling("accept() error");  
	
   

    //클라이언트로부터 요청된 파일 이름 받기
    memset(&data.buf, '\0', BUF_SIZE); //초기화 꼭 해주자
    str_len = read(clnt_sock, data.buf, BUF_SIZE);
	strcpy(file_name, data.buf);

	//file 읽기모드로 생성
    file = open(data.buf, O_RDONLY); 
	if(file==-1){
        printf("%s File Not Found\n", file_name);
		strcpy(message, "FileNotFound");
        write(clnt_sock, message, sizeof(message)-1);
    }
    else{
 		printf("[Server] sending %s\n", data.buf);
		strcpy(message, "FileFound");
		write(clnt_sock, message, sizeof(message)-1);

		 //seq, ack 초기 설정
    	data.seq = SEQ_START;
		data.ack = SEQ_START;
    	memset(data.buf, 0, BUF_SIZE);

    	//파일 읽기 + 내용 보내기
    	while((data.buf_len = read(file, data.buf, BUF_SIZE))>=0)
		{
			//총 바이트 수
			total += data.buf_len;

			//서버에서 클라이언트로 보낼 때 SEQ 와 데이터 크기
        	printf("[Server] Tx: SEQ: %d, %d byte data\n", data.seq, data.buf_len);
			write(clnt_sock, &data, sizeof(data));
		
			//클라이언트로부터 변경된 data 받기
			str_len = read(clnt_sock, &data, sizeof(data)); 

			//보낸 후 데이터 리셋
			memset(data.buf, 0, BUF_SIZE);
		
			//read 에러
	    	if(str_len==-1)
				error_handling("read() error!");
	      
			if(data.buf_len<100)
				break;
			else{
				//클라이언트로부터 ack 번호 받은거 출력
				printf("[Server] Rx ACK: %d\n\n", data.ack);

				//seq를 ack로 변경
				data.seq = data.ack;
			}
		}
		//파일이름 및 총 bytes수 출력    
		printf("%s sent (%d Bytes)\n", file_name, total);
	}

	
    close(clnt_sock);
	close(serv_sock);
	return 0;
}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
