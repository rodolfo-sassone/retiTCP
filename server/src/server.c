/*
 ============================================================================
 Name        : server.c
 Author      : Rodolfo Pio Sassone
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#if defined WIN32
#include <winsock.h>
#else
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#define closesocket close
#endif

#define TRUE 1

#include <stdio.h>
#include <stdlib.h>
#include "protocol.h"


#define NO_ERROR 0
void clearwinsock() {
#if defined WIN32
	WSACleanup();
#endif
}


void add(message* msg);


void mult(message* msg);


void sub(message* msg);


void division(message* msg);


int main(int argc, char *argv[]) {
	char address[LENGHT_IP] = "127.0.0.1";
	int port = 60000;
	if (argc == 3)
	{
		strcpy(address, argv[1]);
		port = atoi(argv[2]);;
	}
	printf("Avviato\n");
#if defined WIN32
	// Initialize Winsock
	WSADATA wsa_data;
	int result =
	WSAStartup(MAKEWORD(2,2), &wsa_data);
	if (result != NO_ERROR) {
		printf("***Error at WSAStartup()***\n");
		return 0;
	}
#endif
	int my_socket;
	my_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	if(my_socket<0)
	{
		printf("***Error: creation socket***\n");
		return -1;
	}


	struct sockaddr_in sad;
	memset(&sad, 0, sizeof(sad));
	sad.sin_family=AF_INET;
	sad.sin_addr.s_addr = inet_addr(address);
	sad.sin_port = htons(port);
	int binded = bind(my_socket,(struct sockaddr*) &sad, sizeof(sad));
	if(binded<0)
	{
		printf("***Error: binding***\n");
		closesocket(my_socket);
		clearwinsock();
		return -1;
	}


	if(listen(my_socket, 5)<0)
	{
		printf("***Error: listen()***\n");
		closesocket(my_socket);
		clearwinsock();
		return -1;
	}

	int c_socket;
	struct sockaddr_in cad;
	int c_len;

	while (TRUE)
	{
		c_len = sizeof(cad);
		memset(&cad, 0, c_len);
		puts("In attesa del client...\n");
		c_socket = accept(my_socket, (struct sockaddr*) &cad, &c_len);
		if(c_socket<0)
		{
			printf("***Error: accept()***\n");
			closesocket(c_socket);
			continue;
		}


		printf("Servendo il client %s:%u...\n", inet_ntoa(cad.sin_addr), ntohs(cad.sin_port));

		short run = 1;
		while(run)
		{
			message msg;
			int byte_recv = recv(c_socket, (char*) &msg, sizeof(msg), 0);
			if(byte_recv < 0)
			{
				printf("***Errore: recv()***\n");
				closesocket(c_socket);
				run = 0;
				break;
			}

			msg.num1 = ntohl(msg.num1);
			msg.num2 = ntohl(msg.num2);
			msg.result = ntohl(msg.result);

			switch (msg.operator)
			{
			case '+':
				add(&msg);
				break;
			case '*':
			case 'x':
				mult(&msg);
				break;
			case '-':
				sub(&msg);
				break;
			case '/':
				division(&msg);
				break;
			case '=':
				run = 0;
				break;
			default:
				break;
			}

			if (run)
			{
				msg.num1 = htonl(msg.num1);
				msg.num2 = htonl(msg.num2);
				msg.result = htonl(msg.result);

				int sended = send(c_socket, (char*) &msg, sizeof(msg), 0);
				if (sended  != sizeof(msg))
				{
					printf("***Errore: invio non riuscito correttamente***\n");
					closesocket(c_socket);
					run = 0;
					break;
				}
			}
		}

		printf("Connessione con il client %s:%u conclusa.\n\n", inet_ntoa(cad.sin_addr), ntohs(cad.sin_port));
	}

	closesocket(my_socket);
	clearwinsock();
	return 0;
}


void add(message* msg)
{
	msg->result = msg->num1 + msg->num2;
}

void mult(message* msg)
{
	msg->result = msg->num1 * msg->num2;
}

void sub(message* msg)
{
	msg->result = msg->num1 - msg->num2;
}

void division(message* msg)
{
	msg->result = msg->num1 / msg->num2;
}
