#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
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
#define ROOT "/mnt/c/sem07/networks/lab03"

using namespace std;

typedef struct s_resstatus {
	std::string code;
	std::string msg;
} t_resstatus;

s_resstatus OK = {"200", "OK"};
s_resstatus NOT_FOUND = {"404", "NOT FOUND"};
s_resstatus FORBIDDEN = {"403", "FORBIDDEN"};


vector<string> split(const string& str, const string& delim)
{
	vector<string> tokens;
	size_t prev = 0, pos = 0;
	do
	{
		pos = str.find(delim, prev);
		if (pos == string::npos) pos = str.length();
		string token = str.substr(prev, pos-prev);
		if (!token.empty()) tokens.push_back(token);
		prev = pos + delim.length();
	}
	while (pos < str.length() && prev < str.length());
	return tokens;
}

string getFileContent(string url)
{
	string filecontent = "";
	int fd = open(url.c_str(), O_RDONLY);
	if (fd != -1)
	{
		char buf[20];
		int res;
		while((res = read(fd, &buf, 19)) > 0)
		{
			buf[res] = '\0';
			filecontent.append(buf);
		}
		close(fd);
	}
	return filecontent;
}

std::string formRespond(string url)
{
	string root = ROOT;
	t_resstatus status = OK;
	struct stat s;
	if (stat(url.c_str(), &s) != 0 )
	{
		url = root + url;
		if (stat(url.c_str(), &s) != 0 )
		{
			status = NOT_FOUND;
		}
	}
	if (!(s.st_mode & S_IFREG))
		status = FORBIDDEN;

	std::string resultMsg = "";
	resultMsg.append("HTTP/1.1 ");
	resultMsg.append(status.code);
	resultMsg.append(" ");
	resultMsg.append(status.msg);
	resultMsg.append("\r\n");
	resultMsg.append("Connection: closed\r\n");
	resultMsg.append("Content-Type: text/html; charset=UTF-8\r\n");
	resultMsg.append("\r\n");

	if (strcmp(status.code.c_str(), "200") == 0)
		resultMsg.append(getFileContent(url));
	return resultMsg;
}

std::string handleRequestMessage(char * innerMessage)
{
	// cout << "TOKENS\n";
	// vector<string> tokens = split(innerMessage, "\r\n");
	// for(int i = 0; i < tokens.size(); i++)
	// 	std::cout << tokens[i] << std::endl;

	string method = strtok(innerMessage, " ");
	string url = strtok(NULL, " ");
	string httpVersion = strtok(NULL, "\r\n");
	string userName = strtok(NULL, "\r\n");
	string HostName = strtok(NULL, "\r\n");
	string Name = strstr(userName.c_str(), ": ") + 2;

	int history = open("history.txt", O_WRONLY | O_APPEND | O_CREAT, S_IRWXU);
	string historyrecord = Name + " looked for " + url + "\n";
	write(history, historyrecord.c_str(), strlen(historyrecord.c_str()));
	close(history);

	if (strcmp(method.c_str(), "GET") == 0)
	{
		return formRespond(url);
	}
	return "NOT GET METHOD";
}

std::string clientHandler(char *message)
{
	printf("====================================\n");
	printf("CLIENT'S MESSAGE:\n%s\n", message);
	std::string res = handleRequestMessage(message);
	const char *msg = res.c_str();
	printf("====================================\n");
	printf("SERVER'S MESSAGE:\n%s", msg);
	return msg;
}

// This class manages a thread pool that will process requests
class ThreadPool {
	public:
		ThreadPool() : done(false) {
			auto numberOfThreads = std::thread::hardware_concurrency();
			if (numberOfThreads == 0)
				numberOfThreads = 1;

			for (unsigned i = 0; i < numberOfThreads; ++i)
				threads.push_back(std::thread(&ThreadPool::doWork, this));
		}

		~ThreadPool() {
			done = true;

			workQueueConditionVariable.notify_all();
			for (auto& thread : threads)
				if (thread.joinable())
					thread.join();
		}

		void queueWork(int fd, char* request) {
			std::lock_guard<std::mutex> g(workQueueMutex);
			workQueue.push(std::pair<int, char*>(fd, request));
			workQueueConditionVariable.notify_one();
		}

	private:
		std::condition_variable_any workQueueConditionVariable;
		std::vector<std::thread> threads;
		std::mutex workQueueMutex;
		std::queue<std::pair<int, char*>> workQueue;

		bool done;

		void doWork() {
			while (!done) {
				std::pair<int, char*> request;
				{ // Create a scope, so we don't lock the queue for longer than necessary
					std::unique_lock<std::mutex> g(workQueueMutex);
					workQueueConditionVariable.wait(g, [&]{return !workQueue.empty() || done;});
					request = workQueue.front();
					workQueue.pop();
				}
				processRequest(request);
			}
		}
		void processRequest(const std::pair<int, char*> item) {
			std::string s = clientHandler(item.second);
			send(item.first, s.c_str(), s.size(), 0);
			close(item.first);
		}
};

void perror_and_exit(std::string err_msg, size_t exit_code)
{
	perror(err_msg.c_str());
	exit(exit_code);
}

int main()
{
	struct sockaddr_in addr, client_addr;

	int listener = socket(AF_INET, SOCK_STREAM, 0);
	if(listener < 0)
		perror_and_exit("socket()", 1);

	addr.sin_family = AF_INET;
	addr.sin_port = htons(PORT);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);

	if(bind(listener, (struct sockaddr *)&addr, sizeof(addr)) < 0)
		perror_and_exit("bind()", 2);

	listen(listener, 10);

		printf("Server is listening on %s:%d...\n", SERVER_IP, PORT);
	ThreadPool tp;
	while(1)
	{

		int sock;
		int bytes_read;
		char *buf = (char*)malloc(MSG_LEN);
		socklen_t cli_addr_size = sizeof(client_addr);

		sock = accept(listener, (struct sockaddr*) &client_addr, &cli_addr_size);
		if(sock < 0)
			perror_and_exit("accept()", 3);

		bytes_read = recv(sock, buf, MSG_LEN, 0);
		if (bytes_read < 0)
		{
			printf("Recv failed");
			close(sock);
			continue ;
		}
		if (bytes_read == 0)
		{
			puts("Client disconnected upexpectedly.");
			close(sock);
			continue ;
		}
		buf[bytes_read] = '\0';
		cout << "buf" << buf << "\n";
		char tst[MSG_LEN];
		strcpy(tst, buf);

		tp.queueWork(sock, tst);
		free(buf);
	}
	close(listener);
	return 0;
}
