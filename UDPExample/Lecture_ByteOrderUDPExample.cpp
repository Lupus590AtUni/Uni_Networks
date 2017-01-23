#include "stdafx.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include "Lecture_PackUnPack.h"

#define DEFAULT_PORT "27015"
#define DEFAULT_BUFLEN 512

typedef struct
{
	unsigned short code; // Type of message
	unsigned short length; // Length of message in bytes
} MESSAGE_PREAMBLE;

#define PREAMBLE_LENGTH 4 // in bytes

// UNIQUE CODE IDS
#define MESSAGE_CODE_TEST 0x0001
#define MESSAGE_CODE_BULLET_FIRED 0x0002

// Test Message
typedef struct
{
	unsigned char	username[20];
	unsigned long	longValue;
	unsigned short	shortValue;
	unsigned char	byteValue;
	int				intValue; // 32-bit signed

	// Send variable lenght data
	unsigned short dataLength;
	unsigned char data[100]; //Max data is 100 bytes
} MESSAGE_TEST;

// Bullet Fired Message
typedef struct
{
	int startX;
	int startY;
	int vectorX;
	int vectorY;
} MESSAGE_BULLET_FIRED;


// PACK MESSAGES
// Pack the preamble
unsigned short packPreamble(MESSAGE_PREAMBLE *preamb, unsigned char *buf)
{
	packShort(buf, preamb->code);
	packShort(buf, preamb->length);

	return PREAMBLE_LENGTH;
}

// Pack Test Message
unsigned short packTestMessage(MESSAGE_TEST *msg, unsigned char *buf)
{
	//unsigned short msglen;
	unsigned char *startPtr = buf; // clever trick to calc msglen

	packBytes(buf, msg->username, 20);
	packLong(buf, msg->longValue);
	packShort(buf, msg->shortValue);
	packByte(buf, msg->byteValue);
	packLong(buf, msg->intValue);

	packShort(buf, msg->dataLength);
	packBytes(buf, msg->data, msg->dataLength);

	/*msglen = */ return (unsigned short)(buf - startPtr);
	//return msglen;

}

// UNPACK MESSAGES
// Unpack the preamble
unsigned short unpackPreamble(MESSAGE_PREAMBLE *preamble, unsigned char *buf)
{
	unpackShort(buf, preamble->code);
	unpackShort(buf, preamble->length + PREAMBLE_LENGTH);
	return PREAMBLE_LENGTH;
}

// Unpack the test message
unsigned short unpackTestMessage(MESSAGE_TEST *msg, unsigned char *buf)
{
	//unsigned short msglen;
	unsigned char *startPtr = buf; // clever trick to calc msglen

	unpackBytes(buf, msg->username, 20);
	unpackLong(buf, msg->longValue);
	unpackShort(buf, msg->shortValue);
	unpackByte(buf, msg->byteValue);
	unpackLong(buf, msg->intValue);

	unpackShort(buf, msg->dataLength);
	unpackBytes(buf, msg->data, msg->dataLength);

	/*msglen = */ return (unsigned short)(buf - startPtr);
	//return msglen;

}


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
		MESSAGE_PREAMBLE preamb;
		MESSAGE_TEST testMsg;
		int i;
		unsigned char sendbuf[1000];
		unsigned short buflen;

		// Create message
		preamb.code = MESSAGE_CODE_TEST;

		// Pack message
		buflen = packTestMessage(*testMsg, (sendbuf + PREAMBLE_LENGTH));


		ZeroMemory(&hints, sizeof(hints));
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_DGRAM;
		hints.ai_protocol = IPPROTO_UDP;

		iResult = getaddrinfo(argv[2], DEFAULT_PORT, &hints, &addressList);

		udpSocket = socket(addressList->ai_family, addressList->ai_socktype, addressList->ai_protocol);

		// acctually send something
		const int NULL_CHAR_LENGTH = 1; // strlen drops this from its char counting
		iResult = sendto(udpSocket, (char*) sendbuf, (int)strlen((char*) sendbuf) + NULL_CHAR_LENGTH, 0, addressList->ai_addr, (int)addressList->ai_addrlen);

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

			udpSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

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