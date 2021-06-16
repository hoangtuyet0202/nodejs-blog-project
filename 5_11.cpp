#include "stdafx.h"
#pragma warning(disable:4996)
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define __STDC_WANT_LIB_EXT1__ 1
#include "stdio.h"
#include <ctime>
#include <stdlib.h>
#include "winsock2.h"
#include "ws2tcpip.h"
#include <iostream>
#include <fstream>
#include <string>
#include <process.h>
#include <map>
#pragma comment (lib,"ws2_32.lib")
using namespace std;
#define SERVER_ADDR "127.0.0.1"
#define BUFF_SIZE 2048
#define CLIENT_DISCONNECT 0
WSADATA wsaData;
WORD wVersion = MAKEWORD(2, 2);
int SERVER_PORT;
map<string, int> userstatus;
FILE *fptr;
struct clientStruct
{
	SOCKET clientSocket; // client socket
	sockaddr_in clientAddr; //address socket
	char* username = ""; //username login socket
	int status = 0; // status socket
};
int Receive(SOCKET, char *, int, int);
int Send(SOCKET, char *, int, int);

//@funtion: Initiate WinSock
void InitiateWinSock() {
	if (WSAStartup(wVersion, &wsaData)) {
		printf("Winsock 2.2 is not supported\n");
	}
}
//@funtion: Input name and port
void InputCommandLineArgument(int argc, char *argv[]) {
	if (argc < 2)
		printf("ERROR INPUT");
	SERVER_PORT = atoi(argv[1]);
}
//@function Remove Char
char* RemoveChar(char *str, int index, int count)
{
	int n;
	n = strlen(str);
	if (index + count - 1>n) str[0] = NULL;
	else
	{
		while (str[index + count] != NULL)
		{
			str[index] = str[index + count];
			index++;
		}
		str[index] = NULL;
	}
	return str;
}
//@function Remove Prefix
char* RemovePrefix(char *buff, int ret) {
	buff[ret] = 0;
	char *rs;
	rs = RemoveChar(buff, 0, 5);
	return rs;
}

//@function Get list account, read File account.txt
void ReadFile() {
	string myText;
	ifstream MyReadFile("account.txt");
	while (getline(MyReadFile, myText)) {
		string user;
		int status;
		status = (int)myText[myText.length() - 1] - 48;
		user = myText.erase(myText.length() - 2, myText.length() - 1);
		userstatus.insert(pair<string, int>(user, status));
	}
	MyReadFile.close();
}
//@function: Check user in map
//@param: Data user
//@return: message send client
char* CheckUser(char *user) {
	string userr;
	userr = user;
	for (map<string, int>::iterator i = userstatus.begin(); i != userstatus.end(); i++) {
		if (i->first == userr) {
			if (i->second == 0)
				return "10";
			else
				return "11";
		}
	}
	return "12";
}
//@function: get prefix
//@return: prefix USER||POST||LOUT
string getPreFix(char *buff) {
	string prefix;
	prefix = buff;
	return prefix.substr(0, 4);
}

int main(int argc, char* argv[])
{
	InputCommandLineArgument(argc, argv);
	InitiateWinSock();

	//Construct socket	
	ReadFile();
	SOCKET listenSock;
	listenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	//Bind address to socket
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(SERVER_PORT);
	serverAddr.sin_addr.s_addr = inet_addr(SERVER_ADDR);

	if (bind(listenSock, (sockaddr *)&serverAddr, sizeof(serverAddr)))
	{
		printf("Error! Cannot bind this address.");
		return 0;
	}

	//Listen request from client
	if (listen(listenSock, 10)) {
		printf("Error! Cannot listen.");
		return 0;
	}

	printf("Server started!\n");
	clientStruct client[FD_SETSIZE];
	SOCKET connSock;
	fd_set readfds, initfds; //use initfds to initiate readfds at the begining of every loop step
	sockaddr_in clientAddr;
	int clientAddrLen = sizeof(clientAddr), clientPort;
	int ret, nEvents;
	char buff[BUFF_SIZE];
	char clientIP[INET_ADDRSTRLEN];
	for (int i = 0; i < FD_SETSIZE; i++)
		client[i].clientSocket = 0;	// 0 indicates available entry
	FD_ZERO(&initfds);
	FD_SET(listenSock, &initfds);

	//Communicate with clients
	while (1) {
		readfds = initfds;		/* structure assignment */
		nEvents = select(0, &readfds, 0, 0, 0);
		if (nEvents < 0) {
			printf("\nError! Cannot poll sockets: %d", WSAGetLastError());
			break;
		}

		//new client connection
		if (FD_ISSET(listenSock, &readfds)) {
			if ((connSock = accept(listenSock, (sockaddr *)&clientAddr, &clientAddrLen)) < 0) {
				printf("\nError! Cannot accept new connection: %d", WSAGetLastError());
				break;
			}
			else {
				printf("You got a connection from %s\n", inet_ntoa(clientAddr.sin_addr)); /* prints client's IP */
				inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, sizeof(clientIP));
				clientPort = ntohs(clientAddr.sin_port);
				int i;
				for (i = 0; i < FD_SETSIZE; i++)
					if (client[i].clientSocket == 0) {
						client[i].clientSocket = connSock;
						client[i].clientAddr = clientAddr;
						FD_SET(client[i].clientSocket, &initfds);
						break;
					}

				if (i == FD_SETSIZE) {
					printf("\nToo many clients.");
					closesocket(connSock);
				}

				if (--nEvents == 0)
					continue; //no more event
			}
		}

		//receive data from clients
		for (int i = 0; i < FD_SETSIZE; i++) {
			if (client[i].clientSocket == 0)
				continue;

			if (FD_ISSET(client[i].clientSocket, &readfds)) {
				try {
					buff[0] = 0;
					ret = Receive(client[i].clientSocket, buff, BUFF_SIZE, 0);
					if (ret <= 0) {
						FD_CLR(client[i].clientSocket, &initfds);
						closesocket(client[i].clientSocket);
						client[i].clientSocket = 0;
						client[i].username = "";
						client[i].status = 0;
					}
					else if (ret > 0) {
						buff[ret] = 0;
						char *dataclient = RemoveChar(buff, ret - 4, 4);
						printf("Receive from client[%s:%d] %s\n", inet_ntoa(client[i].clientAddr.sin_addr), client[i].clientAddr.sin_port, dataclient);
						//Echo to client
						char* datetime = new char[80];
						time_t rawtime;
						struct tm timeinfo;
						time(&rawtime);
						localtime_s(&timeinfo, &rawtime);
						strftime(datetime, 80, "%d/%m/%Y %H:%M:%S", &timeinfo);
						fptr = fopen("log_20183980.txt", "a");
						fprintf(fptr, "%s:%d [%s] $ %s $ ", inet_ntoa(client[i].clientAddr.sin_addr), client[i].clientAddr.sin_port, datetime, dataclient);
						string prefix = getPreFix(dataclient);
						//check USER
						if (prefix == "USER") {
							//if the client is already in use
							if (client[i].status == 1) {
								char *messagesend = "13";
								fprintf(fptr, "%s\n", messagesend);
								Send(client[i].clientSocket, messagesend, strlen(messagesend), 0);
							}
							else {
								//remove "USER "
								char *user = RemovePrefix(dataclient, ret);
								char *messagesend;//message send to client (10 || 11 || 12|| 14)
								messagesend = CheckUser(user);
								// 11 is account locked
								if (messagesend == "11") {
									fprintf(fptr, "%s\n", messagesend);
								}
								// 10 successful
								else if (messagesend == "10") {
									client[i].username = user;
									client[i].status = 1;
									fprintf(fptr, "%s\n", messagesend);
								}
								else if (messagesend == "12") {
									fprintf(fptr, "%s\n", messagesend);
								}
								Send(client[i].clientSocket, messagesend, strlen(messagesend), 0);
							}
						}

						//check POST
						// 20 is success and 21 is not login
						else if (prefix == "POST") {
							//check client login
							if (client[i].username != "" && client[i].status == 1) {
								char *messagesend = "20";
								fprintf(fptr, "%s\n", messagesend);
								Send(client[i].clientSocket, messagesend, strlen(messagesend), 0);
							}
							else {
								char *messagesend = "21";
								fprintf(fptr, "%s\n", messagesend);
								Send(client[i].clientSocket, messagesend, strlen(messagesend), 0);
							}

						}
						//check LOUT
						else if (prefix == "LOUT") {
							if (client[i].status == 1) {
								char *messagesend = "30";
								client[i].username = "";
								client[i].status = 0;
								fprintf(fptr, "%s\n", messagesend);
								Send(client[i].clientSocket, messagesend, strlen(messagesend), 0);
							}
							else {
								char *messagesend = "31";
								fprintf(fptr, "%s\n", messagesend);
								Send(client[i].clientSocket, messagesend, strlen(messagesend), 0);
							}
						}
						//QUIT client
						else {
							char *messagesend = "40";
							fprintf(fptr, "%s\n", messagesend);
							Send(client[i].clientSocket, messagesend, strlen(messagesend), 0);
						}

					}
				}
				catch (int err){
					if(err == SOCKET_ERROR) {
						// logout client
					}
				}

			}

			if (--nEvents <= 0)
				continue; //no more event
		}

	}

	closesocket(listenSock);
	WSACleanup();
	return 0;
}




/* The recv() wrapper function */
int Receive(SOCKET s, char *buff, int size, int flags) {
	int ret,count=0;
	while (true) {
		char receivebuff[2048];

		ret = recv(s, receivebuff, 2048, 0);
		if (ret == SOCKET_ERROR) {
			throw SOCKET_ERROR;
			printf("Error: %d", WSAGetLastError());
		}
		count += ret;
		receivebuff[ret] = 0;
		strcat(buff,receivebuff);
		cout << buff;
		if (receivebuff[ret - 4] == '/' && receivebuff[ret - 3] == 'r' && receivebuff[ret - 2] == '/' && receivebuff[ret - 1] == 'n')
			break;
	}

	return count;
}

/* The send() wrapper function*/
int Send(SOCKET s, char *buff, int size, int flags) {
	int n;

	n = send(s, buff, size, flags);
	if (n == SOCKET_ERROR) {
		throw SOCKET_ERROR;
		printf("Error: %d", WSAGetLastError());
	}
	return n;
}