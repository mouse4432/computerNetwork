//2021112751 김근찬
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>

#define BUF_SIZE 120
#define TTL 64
void error_handling(char *message);

int main(int argc, char *argv[])
{
	//recv_sock 관련 변수들
    int recv_sock;
    int option = 1; //다시 사용할 때 option값 1로 줘서 true로 설정
    struct sockaddr_in adr;
    struct ip_mreq join_adr;

	//send_sock 관련 변수들
    int send_sock;
    struct sockaddr_in mul_adr;
    int time_live = TTL; //64로 설정
	
	//buf, r_buf 관련 변수들 설정 -> 주고 받을 공간들
    char buf[BUF_SIZE];
    char r_buf[BUF_SIZE];
    char a[2] = "[\0";
    char b[3] = "] \0";

    //입력 받을 곳
    char message[100];
    int str_len;

    //fork 받을 곳
    pid_t pid;

    //입력 올바르지 아니하면 강제 종료
    if(argc != 4) {
        printf("Usage : %s <GroupIP> <PORT> <Name>\n", argv[0]);
        exit(1);
    }

    //recv_sock 설정
    recv_sock = socket(PF_INET, SOCK_DGRAM, 0);
    memset(&adr, 0, sizeof(adr));
    adr.sin_family = AF_INET;
    adr.sin_addr.s_addr = htonl(INADDR_ANY);
    adr.sin_port = htons(atoi(argv[2]));

    //동시 bind할 수 있도록 처리
    setsockopt(recv_sock, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

    if(bind(recv_sock, (struct sockaddr*)&adr, sizeof(adr)) == -1)
        error_handling("bind() error");

    join_adr.imr_multiaddr.s_addr = inet_addr(argv[1]);    // 가입할 멀티캐스트 그룹 주소 
    join_adr.imr_interface.s_addr = htonl(INADDR_ANY);      // 멀티캐스트 그룹에 가입할 자신의 IP 주소
    
    // 멀티캐스트 그룹 가입 
    setsockopt(recv_sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (void*)&join_adr, sizeof(join_adr));

    //send_sock 설정
    send_sock = socket(PF_INET, SOCK_DGRAM, 0);
    memset(&mul_adr, 0, sizeof(mul_adr));
    mul_adr.sin_family = AF_INET;
    mul_adr.sin_addr.s_addr = inet_addr(argv[1]);
    mul_adr.sin_port=htons(atoi(argv[2]));

    setsockopt(send_sock, IPPROTO_IP, IP_MULTICAST_TTL, (void*)&time_live, sizeof(time_live));
  
   //fork()로 자식 부모 프로세스 만듦
    pid = fork();
    
    //r_buf 초기화 
    memset(&r_buf, 0, BUF_SIZE);

    if(pid==-1){
        printf("fork fail\n");
        exit(1);
    }
    else if(pid==0){
        while(1){
            //자식 프로세스 -> recv역할

            //받아옴
            str_len = recvfrom(recv_sock, r_buf, BUF_SIZE, 0, NULL, 0);

            //q나 Q받으면 나감
            if(!strcmp(r_buf, "q\0") || !strcmp(r_buf, "Q\0")){
                
                //그룹도 나감
                setsockopt(recv_sock, IPPROTO_IP, IP_DROP_MEMBERSHIP, (void*)&join_adr, sizeof(join_adr));
                break;
            }
            
            //출력
            printf("Received Message: %s\n", r_buf);
	       
            //받고나서 r_buf 초기화
            memset(&r_buf, 0, BUF_SIZE);
        }
    }
    else{
        while(1){

            //buf 사이즈 초기화
            memset(&buf, 0, BUF_SIZE);

            //message 사이즈 초기화
	        memset(&message, 0, BUF_SIZE);

            //buf 앞부분 형태로 [name]으로 만드는 과정
            strcpy(buf, a);
            strcat(buf, argv[3]);
            strcat(buf, b);

            //입력받음
            fgets(message, 100, stdin);

            //갱문자까지 입력 받으므로 끝을 다르게 처리
            message[strlen(message)-1] = '\0';

            //마지막으로 buf와 message 연결
            strcat(buf, message);
            
            //q와 Q 입력 받으면 종료
            if(!strcmp(message, "q\0") || !strcmp(message, "Q\0")){
                //message 보냄
            	sendto(send_sock, message, strlen(message), 0, (struct sockaddr*)&mul_adr, sizeof(mul_adr));
                printf("Leave Multicast group\n");
                break;
            }

            //buf보냄
            sendto(send_sock, buf, strlen(buf), 0, (struct sockaddr*)&mul_adr, sizeof(mul_adr));
        }
    } 
    close(send_sock);
    close(recv_sock);
    return 0;
}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
