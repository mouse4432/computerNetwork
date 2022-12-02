//김근찬 2021112751
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>

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

void *recv_msg(void *arg);
void error_handling(char *msg);
int total_len = 0;
PACKET data;

int main(int argc, char *argv[])
{
	int func;

	int sock;
	struct sockaddr_in serv_addr;
	pthread_t rcv_thread;
	void *thread_return;
	struct timespec start, end;
	unsigned long long t1, t2;
	unsigned long long nano = 1000000000;
	if (argc != 3)
	{
		printf("Usage : %s <IP> <port>\n", argv[0]);
		exit(1);
	}

	sock = socket(PF_INET, SOCK_STREAM, 0);

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
	serv_addr.sin_port = htons(atoi(argv[2]));

	if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
		error_handling("connect() error");
	memset(&data, 0, sizeof(data));

	while (1)
	{
		memset(&data, 0, sizeof(data));
		printf("--------------------------------------------------------------\n");
		printf("\t\tK-OTT Service\n");
		printf("--------------------------------------------------------------\n");
		printf(" Choose a subscribe type\n");
		printf("--------------------------------------------------------------\n");
		printf("1: Basic, 2: Standard, 3: Premium, 4: quit: ");
		scanf("%d", &data.type);
		if (data.type == 4)
		{
			printf("Exit program\n");
			exit(1);
		}
		printf("--------------------------------------------------------------\n");
		printf("1. Download, 2: Back to Main menu: ");
		scanf("%d", &func);
		if (func == 1)
			break;
	}

	
	data.command = 0;
	write(sock, &data, sizeof(data));
	clock_gettime(CLOCK_REALTIME, &start);
	t1 = start.tv_nsec + start.tv_sec * nano;
	pthread_create(&rcv_thread, NULL, recv_msg, (void *)&sock);

	pthread_join(rcv_thread, &thread_return);
	clock_gettime(CLOCK_REALTIME, &end);
	t2 = end.tv_nsec + end.tv_sec * nano;

	if (data.command == 2)
	{
		data.command = 3;
		write(sock, &data, sizeof(data));
	}
	printf("\nFile Transmission Finished\n");
	printf("Total received bytes: %d\n", data.len);
	printf("Downlaoding time: %lld msec\n", (t2-t1)/1000000);
	printf("Client closed\n");
	close(sock);
	return 0;
}

void *recv_msg(void *arg) // read thread main
{
	int sock = *((int *)arg);
	int str_len;

	while (1)
	{
		// if (data.type == 1)
		// {
		// 	str_len = read(sock, &data, sizeof(data));
		// }
		// else if (data.type == 2)
		// {
		// 	str_len = read(sock, data.buf, STANDARD_BUF);
		// }
		// else if (data.type == 3)
		// {
		// 	str_len = read(sock, data.buf, PREMIUM_BUF);
		// }
		str_len = read(sock, &data, sizeof(data));
		// printf("%d\n", str_len);
		memset(data.buf, 0, sizeof(data.buf));

		if (data.command == 1)
			fputc('.', stdout);
		else if (data.command == 2)
			break;
	}
	return NULL;
}

void error_handling(char *msg)
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}
