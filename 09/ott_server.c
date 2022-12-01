//김근찬 2021112751
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <fcntl.h> //file_open,close

#define MAX_CLNT 256
// Buffer size
#define BASIC_BUF 10
#define STANDARD_BUF 100
#define PREMIUM_BUF 1000
#define MAX_SIZE PREMIUM_BUF
typedef struct
{
	int command;
	int type;
	char buf[MAX_SIZE];
	int len;
} PACKET;

void *handle_clnt(void *arg);
void send_msg(PACKET data, int len);
void error_handling(char *msg);

int clnt_cnt = 0;
int total_len = 0;
int clnt_socks[MAX_CLNT];
pthread_mutex_t mutx;
PACKET data;

int main(int argc, char *argv[])
{
	int serv_sock, clnt_sock;
	struct sockaddr_in serv_adr, clnt_adr;
	int clnt_adr_sz;
	pthread_t t_id;
	if (argc != 2)
	{
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}

	pthread_mutex_init(&mutx, NULL);
	serv_sock = socket(PF_INET, SOCK_STREAM, 0);

	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family = AF_INET;
	serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_adr.sin_port = htons(atoi(argv[1]));

	if (bind(serv_sock, (struct sockaddr *)&serv_adr, sizeof(serv_adr)) == -1)
		error_handling("bind() error");
	if (listen(serv_sock, 5) == -1)
		error_handling("listen() error");
	printf("------------------------------------------\n");
	printf("\t\tK-OTT Service\n");
	printf("------------------------------------------\n");
	while (1)
	{
		clnt_adr_sz = sizeof(clnt_adr);
		clnt_sock = accept(serv_sock, (struct sockaddr *)&clnt_adr, &clnt_adr_sz);

		pthread_mutex_lock(&mutx);
		clnt_socks[clnt_cnt++] = clnt_sock;
		pthread_mutex_unlock(&mutx);

		pthread_create(&t_id, NULL, handle_clnt, (void *)&clnt_sock);
		pthread_detach(t_id);
		printf("Connected client IP: %s, clnt_sock=%d \n", inet_ntoa(clnt_adr.sin_addr), clnt_sock);
	}
	close(serv_sock);
	return 0;
}

void *handle_clnt(void *arg)
{
	int clnt_sock = *((int *)arg);
	int str_len = 0, i;
	int file;

	memset(&data, 0, sizeof(data));
	str_len = read(clnt_sock, &data, sizeof(data));
	
	if (data.command == 0)
	{
		file = open("hw06.mp4", O_RDONLY);
	
		while (1)
		{
			if (data.type == 1)
			{
				str_len = read(file, data.buf, BASIC_BUF);
				if (str_len == 0)
				{
					data.command = 2;
					send_msg(data, sizeof(data));
					break;
				}
				data.command = 1;

				data.len += str_len;
			}
			else if (data.type == 2)
			{
				str_len = read(file, data.buf, STANDARD_BUF);
				if (str_len == 0)
				{
					data.command = 2;
					send_msg(data, sizeof(data));
					break;
				}
				data.command = 1;
				data.len += str_len;
			}
			else if (data.type == 3)
			{
				str_len = read(file, data.buf, PREMIUM_BUF);
				if (str_len == 0)
				{
					data.command = 2;
					send_msg(data, sizeof(data));
					break;
				}

				data.command = 1;
				data.len += str_len;
			}
			send_msg(data, sizeof(data));
		}
		
	}
	str_len = read(clnt_sock, &data, sizeof(data));
	if (data.command == 3)
	{
		pthread_mutex_lock(&mutx);
		for (i = 0; i < clnt_cnt; i++) // remove disconnected client
		{
			if (data.type == 1)
				printf("Total Tx Bytes: %d to Client %d (Basic)\n", data.len, clnt_sock);

			else if (data.type == 2)
				printf("Total Tx Bytes: %d to Client %d (Standard)\n", data.len, clnt_sock);

			else if (data.type == 3)
				printf("Total Tx Bytes: %d to Client %d (Premium)\n", data.len, clnt_sock);

			printf("[Rx] FILE_END_ACK form Client %d => clnt_sock: %d closed\n", clnt_sock, clnt_sock);
			if (clnt_sock == clnt_socks[i])
			{
				while (i < clnt_cnt)
				{
					clnt_socks[i] = clnt_socks[i + 1];
					i++; // 클라이언트 종료시점에 무한루프 발생 문제점 수정
				}
				break;
			}
		}
		clnt_cnt--;
		pthread_mutex_unlock(&mutx);
		close(clnt_sock);
	}

	return NULL;
}
void send_msg(PACKET data, int len) // send to all
{
	int i;
	pthread_mutex_lock(&mutx);
	for (i = 0; i < clnt_cnt; i++)
		write(clnt_socks[i], &data, len);
	pthread_mutex_unlock(&mutx);
}
void error_handling(char *msg)
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}
