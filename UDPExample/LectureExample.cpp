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
	SOCKET	udpSocket = INVALID_SOCKET;

	struct addrinfo *addressList = NULL;
	struct addrinfo hints;

	


	if (argc < 2)
	{
		printf("usage: %s -client server-name\r\n", argv[0]);
		printf("usage: %s -server\r\n", argv[0]);
		return 1;
	}
	// Start using WinSock
	iResult = WSAStartup(MAKEWORD(2, 2, ), &wsaData);

	if (strcmp(argv[1], "-client") == 0)
	{
		// Client code
		printf("Client running\r\n");
		if (argc != 3)
		{
			printf("usage: %s -client server-name\n", argv[0]);
			return 1;
		}
		char *sendbuf = "this is a message";

		ZeroMemory(&hints, sizeof(hints));
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_DGRAM;
		hints.ai_protocol = IPPROTO_UDP;

		iResult = getaddrinfo(argv[2], DEFAULT_PORT, &hints, &addressList);

		udpSocket = socket(addressList->ai_family, addressList->ai_socktype, addressList->ai_protocol);

		// acctually send something
		const int NULL_CHAR_LENGTH = 1; // strlen drops this from its char counting
		iResult = sendto(udpSocket, sendbuf, (int)strlen(sendbuf)+NULL_CHAR_LENGTH, 0, addressList->ai_addr, (int)addressList->ai_addrlen);

		freeaddrinfo(addressList); // done with address list, free the momory 

		// recive reply from server
		iResult = recvfrom(udpSocket, recvbuf, recvbuflen, 0, NULL, NULL); // don't care about sender address which is the last two args (with length)
		// iResult is number of bytes recived

		printf("Bytes received: %d Msg=%s\n", iResult, recvbuf);

		closesocket(udpSocket); // done with socket
	}
	else
	{
		if (strcmp(argv[1], "-server") == 0)
		{
			// Server code

			printf("Server running\r\n");
			int			iRcvdBytes = 0;
			sockaddr_in	rx_addr;
			sockaddr sender_addr;
			int			iSenderAddrSize = sizeof(sender_addr);

			udpSocket = socket(AF_INET,SOCK_DGRAM, IPPROTO_UDP);

			rx_addr.sin_family = AF_INET; // TCPIP
			rx_addr.sin_port = htons((u_short)atol(DEFAULT_PORT));
			rx_addr.sin_addr.s_addr = htonl(INADDR_ANY);
			iResult = bind(udpSocket, (SOCKADDR*)&rx_addr, sizeof(rx_addr));

			iResult = recvfrom(udpSocket, recvbuf, recvbuflen, 0, &sender_addr, &iSenderAddrSize); // now we want to know who sent it because we want to send stuff back
			printf("Bytes received: %d Msg=%s\n", iResult, recvbuf);

			const int NULL_CHAR_LENGTH = 1; // strlen drops this from its char counting
			iResult = sendto(udpSocket, recvbuf, (int)strlen(recvbuf) + NULL_CHAR_LENGTH, 0, &sender_addr, iSenderAddrSize);

			closesocket(udpSocket);

		}
		else
		{
			printf("usage: %s -client server-name\r\n", argv[0]);
			printf("usage: %s -server\r\n", argv[0]);
			return 1;
		}
	}

	printf("press any key to exit\r\n");
	while (!_kbhit());
	WSACleanup();  // Thanks WinSock
}