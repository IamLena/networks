#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define PORT 8080
#define SRV_IP "127.0.0.1"
#define BUFSIZE 4
#define ALPHABET "0123456789abcdefghijklmnopqrstuvwxyz"

void print_convert(int number, int base)
{
	if (number / base != 0)
		print_convert(number / base, base);
	printf("%c", ALPHABET[number % base]);
}

void output(int number)
{
	printf("decimal:\t");
	print_convert(number, 10);
	printf("\nbinary:\t");
	print_convert(number, 2);
	printf("\nhexidecimal:\t");
	print_convert(number, 16);
	printf("\noctal:\t\t");
	print_convert(number, 8);
	printf("\n17th base:\t");
	print_convert(number, 17);
	printf("\n");
}

int main(void)
{
	int socketfd;
	struct sockaddr_in servaddr, cliaddr;
	char buffer[4];

	socketfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (socketfd == -1)
	{
		printf("socket() failed\n");
		perror("socket");
		exit(1);
	}

	memset(&servaddr, 0, sizeof(servaddr));
	memset(&cliaddr, 0, sizeof(cliaddr));

	servaddr.sin_family    = AF_INET;
	servaddr.sin_addr.s_addr = INADDR_ANY;
	servaddr.sin_port = htons(PORT);

	if ( bind(socketfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0 )
	{
		printf("bind() failed\n");
		close(socketfd);
		perror("bind");
		exit(1);
	}

	printf("server is ready\n");

	socklen_t len = sizeof(cliaddr);

	// (recvfrom(sock, buf, MSG_LEN, 0, &client_addr, &slen)
	if (recvfrom(socketfd, (char *)buffer, BUFSIZE, 0, ( struct sockaddr *) &cliaddr, &len) == -1)
	{
		printf("recvfrom() failed\n");
		close(socketfd);
		perror("recvfrom");
		exit(1);
	}

	close(socketfd);
	output(*(int *)buffer);
}
