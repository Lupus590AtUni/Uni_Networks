// UDPexample.cpp : Defines the entry point for the console application.
//
// Add WS2_32.lib to the Additional Dependencies for the Linker
//

#include <iostream>
#include <tchar.h>
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

	if(argc<2)
	{
		printf("usage: %s -client server-name\r\n", argv[0]);
		printf("usage: %s -server\r\n", argv[0]);
		return 1;
	}

	// Initialize Winsock
	// The WSAStartup function is called to initiate use of WS2_32.lib.
	// The WSADATA structure contains information about the Windows Sockets 
	// implementation. The MAKEWORD(2,2) parameter of WSAStartup makes a 
	// request for the version of Winsock on the system, and sets the passed 
	// version as the highest version of Windows Sockets support that the 
	// caller can use.
	//
	iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
	if (iResult != 0) 
	{
		printf("WSAStartup failed: %d\n", iResult);
		return 1;
	}

	if(strcmp(argv[1], "-client")==0)
	{
		// For a client to communicate on a network, it must connect to a server.
		// Start by connecting to a socket.
		//
		SOCKET ConnectSocket = INVALID_SOCKET;

		char *sendbuf = "this is a test";
		    
		// Validate the parameters
		if (argc != 3) 
		{
			printf("usage: %s -client server-name\n", argv[0]);
			return 1;
		}

		// ****************************************
		// After initialization, a SOCKET object must be instantiated
		// ****************************************

		// Call the getaddrinfo function requesting the IP address for the 
		// server name passed on the command line. For this application, 
		// the Internet address family is unspecified(AF_UNSPEC) so that either 
		// an IPv6 or IPv4 address can be returned. The application does request 
		// the socket type to be a stream socket for the TCP protocol.
		//
		// The getaddrinfo function provides protocol-independent translation 
		// from an ANSI host name to an address.
		//
		// int WSAAPI getaddrinfo(	const char FAR* nodename, 
		//							const char FAR* servname,
		//							const struct addrinfo FAR* hints, 
		//							struct addrinfo FAR** res);
		// nodename = A pointer to a NULL-terminated ANSI string that contains 
		// a host (node) name or a numeric host address string.
		// For the Internet protocol, the numeric host address string is a 
		// dotted-decimal IPv4 address or an IPv6 hex address.
		// servname = A pointer to a NULL-terminated ANSI string that contains 
		// either a service name or port number represented as a string
		// hints = A pointer to an addrinfo structure that provides hints 
		// about the type of socket the caller supports.
		// res = Output. A pointer to a linked list of one or more addrinfo structures 
		// that contains response information about the host. 
		//
		// The getaddrinfo function is used to determine the values in the 
		// sockaddr structure:
		//
		// - AF_UNSPEC is used so either IPV6 or IPv4 addresses are possible 
		// for Internet address family. 
		// - the IP address of the server that the client will connect to. 
		// - 27015 is the port number associated with the server that the client will connect to. 
		//
		// Returns a linked list of addresses, the memory for which must be
		// freed by calling freeaddrinfo() when finished with.
		//
		// You may directly set an IP address by creating a sockaddr_in 
		// object clientService and setting its values e.g.
		// sockaddr_in clientService;
		// hostent* localHost;
		// char* localIP;
		// localHost = gethostbyname("");
		// localIP = inet_ntoa (*(struct in_addr *)*localHost->h_addr_list);
		// clientService.sin_family = AF_INET;
		// clientService.sin_addr.s_addr = inet_addr(localIP);
		// OR just this clientService.sin_addr.s_addr = inet_addr("127.0.0.1");		
		// clientService.sin_port = htons(5150);
		// bind( ListenSocket,(SOCKADDR*) &clientService, sizeof(clientService) );
		//
		ZeroMemory(&hints, sizeof(hints));
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;

		// Using the hostname(or IP address) typed in by the user, use DNS to
		// Resolve the server address. We pass a portnumber so that it can
		// be included in the returned addresses.
		//
		iResult = getaddrinfo(argv[2], DEFAULT_PORT, &hints, &addressList);
		if ( iResult != 0 ) 
		{
			printf("getaddrinfo failed: %d\n", iResult);
			WSACleanup();
			return 1;
		}

		// Attempt to connect to an address until one succeeds
		for(ptr=addressList; ptr != NULL ;ptr=ptr->ai_next) 
		{

			// Call the socket function and return its value to the ConnectSocket 
			// variable. For this application, use the Internet address family, 
			// streaming sockets, and the TCP/IP protocol returned by the call to 
			// getaddrinfo.
			//

			// Create a SOCKET for connecting to server
			ConnectSocket = socket(ptr->ai_family, 
									ptr->ai_socktype, 
									ptr->ai_protocol);

			// Check for errors to ensure that the socket is a valid socket.
			// Error detection is a key part of successful networking code. 
			// If the socket call fails, it returns INVALID_SOCKET. The if 
			// statement in the previous code is used to catch any errors that may 
			// have occurred while creating the socket. 
			// WSAGetLastError returns an error number associated with the last error 
			// that occurred.
			// WSACleanup is used to terminate the use of the WS2_32 DLL.
			//
			if (ConnectSocket == INVALID_SOCKET) 
			{
				printf("Error at socket(): %ld\n", WSAGetLastError());
				freeaddrinfo(addressList);
				WSACleanup();
				return 1;
			}

			// Call the connect function, passing the created socket and the 
			// sockaddr structure as parameters. Check for general errors.

			// Connect to server.
			iResult = connect( ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
			if (iResult == SOCKET_ERROR) 
			{
				closesocket(ConnectSocket);
				ConnectSocket = INVALID_SOCKET;
				continue;
			}
			break;
		}

		freeaddrinfo(addressList);

		if (ConnectSocket == INVALID_SOCKET) 
		{
			printf("Unable to connect to server!\n");
			WSACleanup();
			return 1;
		}

		// The send and recv functions both return an integer value of the 
		// number of bytes sent or received, respectively, or an error. 
		// Each function also takes the same parameters: the active socket, 
		// a char buffer, the number of bytes to send or receive, and any 
		// flags to use.


		// Send an initial buffer
		//
		iResult = send( ConnectSocket, sendbuf, (int)strlen(sendbuf)+1, 0 );
		if (iResult == SOCKET_ERROR) 
		{
			printf("send failed: %d\n", WSAGetLastError());
			closesocket(ConnectSocket);
			WSACleanup();
			return 1;
		}

		printf("Bytes Sent: %ld\n", iResult);

		// shutdown the connection since no more data will be sent
		// shutdown() acts like a close() but waits for the receiver
		// to ACK the last data sent by the client.
		// SD_SEND = no more sends allowed
		//
		iResult = shutdown(ConnectSocket, SD_SEND);
		if (iResult == SOCKET_ERROR) 
		{
			printf("shutdown failed: %d\n", WSAGetLastError());
			closesocket(ConnectSocket);
			WSACleanup();
			return 1;
		}

		// Receive until the peer closes the connection
		do 
		{
            // Note: This bit of code is "broken".
            // This code overwrites the contents of the buffer
            // each time around the loop.
            // Depending on your application, you should either
            // keep a pointer into the buffer which is updated
            // to the next position at which data should be
            // stored in the buffer or create a new buffer
            // each time you receive more data.
            //
			iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
			if ( iResult > 0 )
				printf("Bytes received: %d Msg=%s\n", iResult, recvbuf);
			else if ( iResult == 0 )
				printf("Connection closed\n");
			else
				printf("recv failed: %d\n", WSAGetLastError());

		} while( iResult > 0 );

		// cleanup
		closesocket(ConnectSocket);
	}
	else
	{
		// For a server to accept client connections, it must be bound to a 
		// network address within the system. The following code demonstrates 
		// how to bind a socket that has already been created to an 
		// IP address and port. Client applications use the IP address and 
		// port to connect to the host network.
		// The sockaddr structure holds information regarding the address family,
		// IP address, and port number.
		//
		SOCKET ListenSocket = INVALID_SOCKET;
		SOCKET ClientSocket = INVALID_SOCKET;

		int iRcvdBytes = 0;

		// ****************************************
		// After initialization, a SOCKET object must be instantiated
		// ****************************************

		// Call the getaddrinfo function requesting the IP address for this
		// server. The application requests the socket type to be a 
		// stream socket for the TCP protocol.
		//
		// The getaddrinfo function is used to determine the values in the 
		// sockaddr structure:
		// - AF_INET is used to specify the IPv4 address family. 
		// - SOCK_STREAM is used to specify a stream socket. 
		// - IPPROTO_TCP is used to specify the TCP protocol . 
		// - AI_PASSIVE flag indicates the caller intends to use the returned 
		// socket address structure in a call to the bind function. 
		// When the AI_PASSIVE flag is set and nodename parameter to the 
		// getaddrinfo function is a NULL pointer, the IP address portion 
		// of the socket address structure is set to INADDR_ANY for 
		// IPv4 addresses or IN6ADDR_ANY_INIT for IPv6 addresses. 
		// - 27015 is the port number associated with the server that the client will connect to. 
		//
		// Returns a linked list of addresses, the memory for which must be
		// freed by calling freeaddrinfo() when finished with.
		//
		ZeroMemory(&hints, sizeof(hints));
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;
		hints.ai_flags = AI_PASSIVE;

		// Resolve the server address and port
		iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &addressList);
		if ( iResult != 0 ) 
		{
			printf("getaddrinfo failed: %d\n", iResult);
			WSACleanup();
			return 1;
		}

		// Create a SOCKET for connecting to server
		ListenSocket = socket(addressList->ai_family, addressList->ai_socktype, addressList->ai_protocol);
		if (ListenSocket == INVALID_SOCKET) 
		{
			printf("socket failed: %ld\n", WSAGetLastError());
			freeaddrinfo(addressList);
			WSACleanup();
			return 1;
		}

		// Call the bind function (associate socket with local address), 
		// passing the created socket and sockaddr structure 
		// returned from the getaddrinfo function as parameters. 
		// Check for general errors.
		//
		// Setup the TCP listening socket
		//
		iResult = bind( ListenSocket, addressList->ai_addr, (int)addressList->ai_addrlen);
		if (iResult == SOCKET_ERROR) 
		{
			printf("bind failed: %d\n", WSAGetLastError());
			freeaddrinfo(addressList);
			closesocket(ListenSocket);
			WSACleanup();
			return 1;
		}

		freeaddrinfo(addressList);

		// After the socket is bound to an IP address and port on the system, 
		// the server must then listen on that IP address and port for 
		// incoming connection requests.
		// To listen on a socket call the listen function, passing the created 
		// socket and the maximum number of allowed connections to accept as 
		// parameters. Check for general errors.
		//
		iResult = listen(ListenSocket, SOMAXCONN);
		if (iResult == SOCKET_ERROR) 
		{
			printf("listen failed: %d\n", WSAGetLastError());
			closesocket(ListenSocket);
			WSACleanup();
			return 1;
		}

		// Once the socket is listening for a connection, the program must 
		// handle connection requests on that socket.
		// To accept a connection on a socket, create a temporary SOCKET object 
		// called AcceptSocket for accepting connections.
		//

		// Create a continuous loop that checks for connections requests. 
		// If a connection request occurs, call the accept function to 
		// handle the request.
		//
		// Accept a client socket
		ClientSocket = accept(ListenSocket, NULL, NULL);
		if (ClientSocket == INVALID_SOCKET) 
		{
			printf("accept failed: %d\n", WSAGetLastError());
			closesocket(ListenSocket);
			WSACleanup();
			return 1;
		}

		// When the client connection has been accepted, close the 
		// original socket, transfer control from the temporary socket 
		// to the original socket, and stop checking for new connections.
		// No longer need server socket.
		//
		closesocket(ListenSocket);

		// The send and recv functions both return an integer value of the 
		// number of bytes sent or received, respectively, or an error. 
		// Each function also takes the same parameters: the active socket, 
		// a char buffer, the number of bytes to send or receive, and any 
		// flags to use.

		// Receive until the peer shuts down the connection
		do 
		{

            // Note: This bit of code is "broken".
            // This code overwrites the contents of the buffer
            // each time around the loop.
            // Depending on your application, you should either
            // keep a pointer into the buffer which is updated
            // to the next position at which data should be
            // stored in the buffer or create a new buffer
            // each time you receive more data.
            //
			iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
			if (iResult > 0)
			{
				printf("Bytes received: %d Msg=%s\n", iResult, recvbuf);
				iRcvdBytes = iResult;
			}
			else if (iResult == 0)
				printf("Connection closing...\n");
			else  
			{
				printf("recv failed: %d\n", WSAGetLastError());
				closesocket(ClientSocket);
				WSACleanup();
				return 1;
			}

		} while (iResult > 0);

		// Echo the buffer back to the sender
		iResult = send( ClientSocket, recvbuf, iRcvdBytes, 0 );
		if (iResult == SOCKET_ERROR) 
		{
			printf("send failed: %d\n", WSAGetLastError());
			closesocket(ClientSocket);
			WSACleanup();
			return 1;
		}

		printf("Bytes Sent: %ld\n", iResult);

		// shutdown the connection since we're done
		// shutdown() acts like a close() but waits for the receiver
		// to ACK the last data sent by the client.
		// SD_SEND = no more sends allowed
		// For TCP sockets, a FIN will be sent after all data is sent 
		// and acknowledged by the receiver.
		// Other options are SD_RECEIVE and SD_BOTH.
		//
		iResult = shutdown(ClientSocket, SD_SEND);
		if (iResult == SOCKET_ERROR) 
		{
			printf("shutdown failed: %d\n", WSAGetLastError());
			closesocket(ClientSocket);
			WSACleanup();
			return 1;
		}

		// cleanup
		closesocket(ClientSocket);
	}

	while(!_kbhit());

	WSACleanup();

	return 0;
}

