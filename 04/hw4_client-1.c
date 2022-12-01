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
	int sock;
	struct sockaddr_in serv_addr;
	char message[20];
	char file_name[20];
	Packet data; 
	int buf_total;
	int str_len;
	int total = 0;

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
	
	
	memset(&data.buf, '\0', BUF_SIZE);
	//파일 이름 입력 
	fputs("Input file name: ", stdout);
	scanf("%s", data.buf);
	printf("[Client] request %s\n\n", data.buf);
	strcpy(file_name, data.buf);
    
    //파일 이름 보내기
    write(sock, data.buf, strlen(data.buf));

	//파일이 있는 지 없는지 read
	str_len = read(sock, message, sizeof(message));
	
    if(!strcmp(message, "FileNotFound")){//서버로부터 파일이 없는 경우
        printf("File Not Found\n");
    }
	else{//서버로부터 파일이 있는 경우	
		//file 쓰기모드 생성 
		int file = open(data.buf, O_CREAT | O_WRONLY | O_TRUNC, 0644); 
		if(file==-1)
			error_handling("fileopen() error");
		//data.buf 초기화 -> 파일 내용 입력 받을거여서 
		memset(data.buf, 0, BUF_SIZE);
		data.ack = 0;
		//서버로부터 일단 계속 입력 받을게요
    	while(1){
			//파이팅

			//data를 서버로부터 받아옴 -> 파일 내용, seq, buf_len
			str_len = read(sock, &data, sizeof(data));

			//read 에러
			if(str_len==-1)
				error_handling("read() error!");

			//seq, buf_len 프린트
			printf("[Client] Rx SEQ: %d, len: %d bytes\n", data.seq, data.buf_len);
			
			//총 바이트 수
			total += data.buf_len;

		
			//생성한 파일에 data.buf 내용을 적어줌
			str_len = write(file, data.buf, data.buf_len);

			//ack 설정
			data.ack = data.ack + data.buf_len + 1;

			if(data.buf_len<100){ //파일 마지막 부분을 보낸 경우
				break;
			}
			else{
				//설정된 ack 다시 서버로 보냄
				write(sock, &data, sizeof(data));
				printf("[Client] Tx ACK: %d\n\n", data.ack);
			}	
    	}
		printf("%s received (%d Bytes)\n", file_name, total);
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
