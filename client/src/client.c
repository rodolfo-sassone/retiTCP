/*
 ============================================================================
 Name        : client.c
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "protocol.h"


#define MAX_INPUT 512

#define NO_ERROR 0
void clearwinsock() {
#if defined WIN32
	WSACleanup();
#endif
}

void parser(char* input, short* run, short* s, char** token);


int main(int argc, char *argv[])
{
	char address[LENGHT_IP] = "127.0.0.1";
	int port = 60000;
	if (argc == 3)
	{
		strcpy(address, argv[1]);
		port = atoi(argv[2]);;
	}

#if defined WIN32
	// Initialize Winsock
	WSADATA wsa_data;
	int result =
	WSAStartup(MAKEWORD(2,2), &wsa_data);
	if (result != NO_ERROR)
	{
		printf("***Error at WSAStartup()***\n");
		return 0;
	}
#endif
	int my_socket;
	my_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	if(my_socket<0)
	{
		printf("***Error: creation socket***");
		return -1;
	}

	//Server Connection
	struct sockaddr_in sad;
	memset(&sad, 0, sizeof(sad));
	sad.sin_family = AF_INET;
	sad.sin_addr.s_addr = inet_addr(address);
	sad.sin_port = htons(port);
	int connected = connect(my_socket, (struct sockaddr*) &sad, sizeof(sad));
	if (connected<0)
	{
		printf("***Error: connect()***");
		closesocket(my_socket);
		clearwinsock();
		return -1;
	}

	puts("Connessione Riuscita\n");
	puts("Questa calcolatrice lavora solo con i numeri interi.\n"
			"In caso di divisione considerera' solo la parte intera del risultato.\n"
			"Scrivi le operazioni in notazione prefissa (es. x 4 2 per 4x2).\n"
			"Per uscire digita =\n");

	short run = 1;
	message msg;
	while(run)
	{
		short status = 1;		//flag: 1 if all right, 0 otherwise
		char input[MAX_INPUT]={""};
		char* token[3];
		puts("Scrivi operazione: ");
		gets(input);

		parser(input, &run, &status, token);

		msg.operator = *token[0];
		msg.num1 = 0;
		msg.num2 = 0;
		msg.result = 0;

		if(status)
		{
			msg.num1 = htonl(atoi(token[1]));
			msg.num2 = htonl(atoi(token[2]));
			msg.result = htonl(msg.result);

			int sended = send(my_socket,(char*) &msg, sizeof(msg), 0);
			if(sended != sizeof(msg))
			{
				printf("***Errore: send()***");
				closesocket(my_socket);
				clearwinsock();
				return -1;
			}

			puts("Risultato: ");

			int byte_rcvd = recv(my_socket,(char*) &msg, sizeof(msg), 0);
			if(byte_rcvd <= 0)
			{
				printf("***Errore: recv()***");
				closesocket(my_socket);
				clearwinsock();
				return -1;
			}

			msg.num1 = ntohl(msg.num1);
			msg.num2 = ntohl(msg.num2);
			msg.result = ntohl(msg.result);

			printf("%d\n\n", msg.result);
		}

	}

	int sended = send(my_socket,(char*) &msg, sizeof(msg), 0);
	if(sended != sizeof(msg))
	{
		printf("***Errore: send()***");
		closesocket(my_socket);
		clearwinsock();
		return -1;
	}

	closesocket(my_socket);
	clearwinsock();
	return 0;
}


void parser(char* input, short* run, short* s, char** token)
{
	char separator = ' ';
	char* t;
	int i=0;

	//Tokenization
	t = strtok(input, &separator);

	while(t != NULL && i < 3)
	{
		token[i]=t;
		i++;
		t=strtok(NULL, &separator);
	}

	//Parsing
	if(i != 3)
	{
		if(*token[0] == '=')
		{
			*run = 0;
			*s = 0;
		}
		else
		{
			puts("***Operazione non valida o sintassi errata***\n");
			*s = 0;
		}
	}
	else
	{
		if(strlen(token[0])!=1)
		{
			puts("***Operazione non valida: operatore non valido***");
			*s=0;
		}
		else
		{	//Control operator
			if(*token[0] == '+' || *token[0] == '-' || *token[0] == 'x' || *token[0] == '/' || *token[0] == '*')
			{
				int j=0;
				short isANum1 = 1;
				short isANum2 = 1;
				//Control if token1 are digit
				for(j=0;j<strlen(token[1]);j++)
				{
					if(!isdigit(token[1][j]))
						isANum1 = 0;
				}
				if(isANum1)
				{	//Control if token2 are digit
					for(j=0;j<strlen(token[2]);j++)
					{
						if(!isdigit(token[2][j]))
							isANum2 = 0;
					}
					if(isANum2)
					{	//Control division by 0
						if(*token[0] == '/' && atoi(token[2]) == 0)
						{
							puts("***Operazione non valida: impossibile dividere per 0***\n");
							*s = 0;
						}
					}
					else
					{
						puts("***Operazione non valida: operando 2 non valido***\n");
						*s = 0;
					}
				}
				else
				{
					puts("***Operazione non valida: operando 1 non valido***\n");
					*s = 0;
				}

			}
			else
			{
				puts("***Operazione non valida: operatore non valido***\n");
				*s = 0;
			}
		}
	}
}

