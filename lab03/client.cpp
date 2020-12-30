#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <string.h>
#include <iostream>
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <string>

#define PORT 5001
#define SERVER_IP "127.0.0.1"
#define MSG_LEN 1024

using namespace std;

static string userName;

void perror_and_exit(std::string err_msg, size_t exit_code)
{
	perror(err_msg.c_str());
	exit(exit_code);
}

string generateGetMessage(char *s)
{
	string res_str = "";
	string str = s;
	res_str.append("GET ");
	res_str.append(str + " ");
	res_str.append("HTTP/1.1\r\n");
	res_str.append("Username: ");
	res_str.append(userName);
	res_str.append("\r\n");
	res_str.append("Host: " + str + "\r\n");
	res_str.append("User-Agent: Console\n");
	res_str.append("Accept: text/html\n");
	res_str.append("\0");
	cout << "request_message: " << res_str << endl;
	return res_str;
}

int main(void)
{
	int clientSock = socket(AF_INET,  SOCK_STREAM, IPPROTO_TCP);
	if (clientSock < 0)
		perror_and_exit("socket error", 1);

	struct sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(PORT);
	serverAddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

	if (connect(clientSock, (struct sockaddr*) &serverAddr, sizeof(serverAddr)) < 0)
		perror_and_exit("connect error", 1);

	printf("enter your Name: \n");
	cin >> userName;

	char message[MSG_LEN];
	printf("enter url: \n");
	scanf("%s", message);

	string mes = generateGetMessage(message);
	const char * msg = mes.c_str();

	printf("Sending message...\n\n");

	cout << "msg " << msg << "\n";
	sendto(clientSock, msg, strlen(msg), 0, (struct sockaddr*) &serverAddr, sizeof(serverAddr));

	unsigned int sAddrlen = sizeof(serverAddr);
	if (recvfrom(clientSock, message, MSG_LEN, 0, (struct sockaddr*) &serverAddr, &sAddrlen) == -1)
		perror_and_exit("recvfrom", 1);
	std::cout << message;

	close(clientSock);
	return 0;
}
