#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/select.h>
#include <fcntl.h>
#define BUF_SIZE 2048

void error_handling(char *message);

int main(int argc, char *argv[])
{
	char buf[BUF_SIZE]; //통신할 때 주고 받을 것
	int fd1, fd2; //fd1:파일 읽기 용도, fd2:서버 통신 용도
	fd_set reads, cpy_reads; //디스크립터의 정보를 모아놓을 변수들
	struct timeval timeout; //select의 timeout 설정할 변수
	int fd_max, fd_num; //디스크립터의 정보 모아놓은 max num, fd_num은 select return값 판단
	int func; //기능(1이면 sender, 2이면 receiver)
	struct sockaddr_in serv_adr;
	int len;

	if(argc!=3) {
		printf("Usage : %s <IP> <port>\n", argv[0]);
		exit(1);
	}

	FD_ZERO(&reads); //reads초기화

	printf("----------------------------------------\n");
	printf(" Choose function\n");
	printf("1. Sender, 	2: Receiver\n");
	printf("----------------------------------------\n");
	printf("=>");

	scanf("%d", &func);
	
	if(func == 1){
		//fd1 읽기모드로 생성
    	fd1 = open("rfc1180.txt", O_RDONLY); 
		if(fd1==-1)
        	error_handling("file not found");
		
		FD_SET(fd1, &reads); //fd1, reads등록
		fd_max = fd1;
	}

	fd2 = socket(PF_INET, SOCK_STREAM, 0);   
	if(fd2==-1)
		error_handling("socket() error");

	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family=AF_INET;
	serv_adr.sin_addr.s_addr=inet_addr(argv[1]);
	serv_adr.sin_port=htons(atoi(argv[2]));
	
	if(connect(fd2, (struct sockaddr*)&serv_adr, sizeof(serv_adr))==-1)
		error_handling("connect() error!");
	memset(buf, 0, BUF_SIZE);
	
	write(fd2, &func, sizeof(func));

	FD_SET(fd2, &reads); //fd2, reads에 등록
	if(fd_max < fd2)
		fd_max = fd2; 

	if(func == 1){
		//sender 기본 내용 출력
    	printf("\nFile Sender Start!\n");
		printf("Connected..........\n");
		printf("fd1: %d, fd2: %d\n", fd1, fd2);
		printf("max_fd: %d\n", fd_max);	
	}
	else if(func == 2){			
		//receiver 기본 내용 출력
		printf("\nFile Receiver Start!\n");
		printf("Connected..........\n");
		printf("fd2: %d\n", fd2);
		printf("max_fd: %d\n", fd_max);	
	}

	while(1){		
		//cpy_reads에 복사
		cpy_reads = reads;
		//시간 3초 timeout으로 설정
		timeout.tv_sec = 3;
		timeout.tv_usec = 0;

		if((fd_num = select(fd_max+1, &cpy_reads, 0, 0, &timeout))==-1) //select -1이면 break
			break;
		if(fd_num==0) //0이면 아무 일 발생 x
			continue;
		//cpy_reads에 저장된 파일들에 무슨 일 발생
		if(func == 1){ //sender 기능에서 파일들 일 발생한 것들 처리
			if(FD_ISSET(fd1, &cpy_reads)){ //파일 읽는 부분에서 일이 발생했을 때
				sleep(1); //1초 지연
				len = read(fd1, buf, BUF_SIZE);//파일로부터 BUF_SIZE만큼 입력 받고 정리
				if(len == 0){
					//fd1 clear 및 close
					FD_CLR(fd1, &reads);
					close(fd1);
				}
				write(fd2, buf, len); //buf 서버로 보냄
				memset(buf, 0, BUF_SIZE);
			}
			if(FD_ISSET(fd2, &cpy_reads)){ //서버에서 일이 발생했을 때
				len = read(fd2, buf,  BUF_SIZE); //서버로부터 정보 받고 buf 프린트
				printf("%s", buf);
				memset(buf, 0, BUF_SIZE);
			}
		}
		else{ //receiver 기능에서 파일들 일 발생한 것들 처리
			if(FD_ISSET(fd2, &cpy_reads)){ //서버에서 일이 발생했을 때
				len = read(fd2, buf, BUF_SIZE); //서버로부터 buf 정보 받고 dbuf 프린트
				printf("%s", buf);
				write(fd2, buf, len); //다시 서버로 돌려보냄
				memset(buf, 0, BUF_SIZE);
			}
		}
	}
	//fd2 clear 및 close
	FD_CLR(fd2, &reads);
	close(fd2);
	return 0;
}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}