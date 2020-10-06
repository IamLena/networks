// Client side implementation of UDP client-server model
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

void input_number(int *num_adr)
{
	printf("Input number: ");
	if (scanf("%d", num_adr) != 1)
	{
		printf("Error while reading number\n");
		exit(1);
	}
}

int main(void)
{
	int number;
	input_number(&number);

	int socketfd;
	struct sockaddr_in servaddr;

	socketfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (socketfd == -1)
	{
		printf("socket() failed\n");
		perror("socket");
		exit(1);
	}

	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(PORT);
	servaddr.sin_addr.s_addr = INADDR_ANY;

	if (inet_aton(SRV_IP, &servaddr.sin_addr) == 0)
    {
		printf("inet_aton() failed\n");
		close(socketfd);
		perror("inet_aton");
		exit(1);
	}

	// sendto(sock, buf, MSG_LEN, 0, &server_addr, slen)
	sendto(socketfd, &number, sizeof(number), 0, (const struct sockaddr *) &servaddr, sizeof(servaddr));
	printf("number sent\n");
}
