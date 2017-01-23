#include <iostream>
#include <tchar.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <time.h>

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

	TCHAR   *serverName;

	if (argc < 2)
	{
		printf("usage: %s server-name\r\n", argv[0]);
		printf("Using loopback address instead..\r\n");
		serverName = "127.0.0.1";
	}
	else
	{
		serverName = argv[1];
	}


	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0)
	{
		printf("WSAStartup failed: %d\n", iResult);
		return 1;
	}

	SOCKET ConnectSocket = INVALID_SOCKET;
	char* sendBuf = "this is a test";

	ZeroMemory(&hints, sizeof(hints));

	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	iResult = getaddrinfo(argv[2], DEFAULT_PORT, &hints, &addressList);
	for (ptr = addressList; ptr != NULL; ptr = ptr->ai_next)
	{
		ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
		iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
		break;
	}
	freeaddrinfo(addressList);

	//iResult = num of bytes sent
	iResult = send(ConnectSocket, sendBuf, (int)strlen(sendBuf)+1, 0); //strlen+1 for null char

	iResult = shutdown(ConnectSocket, SD_SEND);

	do
	{
		iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
		if (iResult > 0)
			printf("Bytes recived from server %d Msg: %s\r\n", iResult, recvbuf);
		else if (iResult == 0)
			printf("Connect closed at server\r\n");
		else
		{
			printf("Recv failed: %d\r\n", WSAGetLastError());
			break;
		}

	} while (iResult > 0);

	printf("\r\nHit key to exit");
	while (!_kbhit());

	WSACleanup();

	return 0;

}