
#include "stdafx.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>

#define DEFAULT_PORT "27015"
#define DEFAULT_BUFLEN 512

int _tmain(int argc, _TCHAR* argv[])
{
	WSADATA	wsaData;
	char	recvbuf[DEFAULT_BUFLEN];
	int		iResult;
	int		recvbuflen = DEFAULT_BUFLEN;

	struct addrinfo *addressList = NULL;
	struct addrinfo *ptr = NULL;
	struct addrinfo hints;

	
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0)
	{
		printf("WSAStartup failed: %d\n", iResult);
		return 1;
	}

	printf("Starting server...\r\n");

	
	SOCKET ListenSocket = INVALID_SOCKET;
	SOCKET ClientSocket = INVALID_SOCKET;

	int iRcvdBytes = 0;


	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &addressList);

	ListenSocket = socket(addressList->ai_family, addressList->ai_socktype, addressList->ai_protocol);
	iResult = bind(ListenSocket, addressList->ai_addr, (int)addressList->ai_addrlen);

	freeaddrinfo(addressList);
	
	iResult = listen(ListenSocket, SOMAXCONN);

	// Wait for incomming connection (blocking)
	ClientSocket = accept(ListenSocket, NULL, NULL);

	// VERRY SIMPLE EXAMPLE
	// Normlly loop of listen socket to get more connections from other clients
	closesocket(ListenSocket);

	do
	{
		iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
		if (iResult > 0)
			printf("Bytes recived from server %d Msg: %s\r\n", iResult, recvbuf);
		else if (iResult == 0)
			printf("Connect closed at client\r\n");
		else
			printf("Recv failed: %d\r\n", WSAGetLastError());

	} while (iResult > 0);


	iResult = send(ClientSocket,recvbuf, recvbuflen, 0);

	// Client did shutdown to send message so we can shutdown safely
	iResult = shutdown(ClientSocket, SD_SEND);

	do
	{
		iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
		if (iResult > 0)
			printf("Bytes recived from server %d Msg: %s\r\n", iResult, recvbuf);
		else if (iResult == 0)
			printf("Connect closed at client\r\n");
		else
		{
			printf("Recv failed: %d\r\n", WSAGetLastError());
			break;
		}

	} while (iResult > 0);

	closesocket(ClientSocket);

	printf("\r\nHit key to exit");
	while (!_kbhit());

	WSACleanup();

	return 0;

}