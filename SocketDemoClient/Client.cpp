#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define HAVE_STRUCT_TIMESPEC
#define PTW32_STATIC_LIB
#define PTW32_STATIC

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <pthread.h>
#include <WinSock2.h>
#pragma comment(lib, "Ws2_32.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
#include <csignal>
#include <iostream>
#define bzero(b,len) (memset((b), '\0', (len)), (void) 0)

using namespace std;

// The one and only application object

int sockfd = 0;
char name[32];
#define LENGTH 2048
volatile sig_atomic_t flag = 0;


void str_overwrite_stdout() {
	printf("\r%s", "> ");
	fflush(stdout);
}

void str_trim_lf(char* arr, int length) 
{
	for (int i = 0; i < length; i++) { // trim \n
		if (arr[i] == '\n') {
			arr[i] = '\0';
			break;
		}
	}
}

void catch_ctrl_c_and_exit(int sig) {
	flag = 1;
}

void* send_msg_handler(void* arg) {
	char message[LENGTH] = {};
	char buffer[LENGTH + 32] = {};

	while (1) {
		str_overwrite_stdout();
		fgets(message, LENGTH, stdin);
		str_trim_lf(message, LENGTH);

		if (strcmp(message, "exit") == 0) {
			break;
		}
		else {
			sprintf(buffer, "%s: %s\n", name, message);
			send(sockfd, buffer, strlen(buffer), 0);
		}

		bzero(message, LENGTH);
		bzero(buffer, LENGTH + 32);
	}
	catch_ctrl_c_and_exit(2);

	return NULL;
}

void* recv_msg_handler(void * arg) {
	char message[LENGTH] = {};
	while (1) {
		int receive = recv(sockfd, message, LENGTH, 0);
		if (receive > 0) {
			printf("%s", message);
			str_overwrite_stdout();
		}
		else if (receive == 0) {
			break;
		}
		else {
			// -1
		}
		memset(message, 0, sizeof(message));
	}

	return NULL;
}

int main()
{
	char sAdd[1000];
	const char* ip = "0.0.0.0";
	unsigned int port = 8888;

	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		printf("Failed. Error Code : %d", WSAGetLastError());
		return 1;
	}

	cout << "Please enter your name: ";
	fgets(name, 32, stdin);
	str_trim_lf(name, strlen(name));

	if (strlen(name) > 32 || strlen(name) < 2) {
		printf("Name must be less than 30 and more than 2 characters.\n");
		return EXIT_FAILURE;
	}

	struct sockaddr_in server_addr;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	server_addr.sin_port = htons(port);

	int err = connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr));
	if (err == -1) {
		printf("ERROR: connect\n");
		return EXIT_FAILURE;
	}

	send(sockfd, name, 32, 0);

	cout << "=== WELCOME TO THE CHATROOM ===" << endl;

	pthread_t send_msg_thread;
	if (pthread_create(&send_msg_thread, NULL, &send_msg_handler, NULL) != 0) {
		printf("ERROR: pthread\n");
		return EXIT_FAILURE;
	}

	pthread_t recv_msg_thread;
	if (pthread_create(&recv_msg_thread, NULL, &recv_msg_handler, NULL) != 0) {
		printf("ERROR: pthread\n");
		return EXIT_FAILURE;
	}

	while (1) {
		if (flag) {
			printf("\nBye\n");
			break;
		}
	}

	closesocket(sockfd);

	return EXIT_SUCCESS;

}