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

	if(argc<2)
	{
		printf("usage: %s server-name\r\n", argv[0]);
        printf("Using loopback address instead..\r\n");
		serverName = "127.0.0.1";
	}
    else
    {
		serverName = argv[1];
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

	while(!_kbhit())        // Loop until key pressed
	{
		// For a client to communicate on a network, it must connect to a server.
		// Start by connecting to a socket.
		//
		SOCKET ConnectSocket = INVALID_SOCKET;

		char sendbuf[1000];
        
        sprintf(sendbuf, "%s %i", "this is a test", clock());

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
        iResult = getaddrinfo(serverName, DEFAULT_PORT, &hints, &addressList);

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
			printf("Unable to connect to server!: %ld\n", WSAGetLastError());
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
        /* WARNING: DO NOT OVER USE TCP CONNECTIONS BY OPENING
        AND CLOSING THE SOCKETS TOO QUICKLY.
        Use UDP instead if you want to send large amounts of sporadic
        data.
        If you want to send a lot of data using TCP, keep the connection 
        open and use multiple sends. This is also more efficient because
        the overhead of the connection process is quite significant.
        However, you should need keep the socket open for long periods
        so this is not always possible.
        Instead, use socket pooling to avoid closing the sockets.

        Alternatively, you could modify "MaxUserPort" and
        "TcpTimedWaitDelay".
        Increase the default MaxUserPort setting and reduce the 
        TcpTimedWaitDelay setting so that you do not run out of 
        client anonymous ports. For example, you can set the 
        MaxUserPort setting to 20000 and set the TcpTimedWaitDelay 
        setting to 30. A lower TcpTimedWaitDelay setting means that 
        the sockets wait in the TIME_WAIT state for less time. 
        A higher MaxUserPort setting means that you can have more 
        sockets in the TIME_WAIT state.
        Note that if you adjust the MaxUserPort or TcpTimedWaitDelay 
        setting, you must restart Microsoft Windows for the new setting 
        to take effect.
        http://blogs.technet.com/tristank/archive/2008/03/11/maxuserport-what-it-is-what-it-does-when-it-s-important.aspx
        Explanation of Problem:
        =======================
        The Winsock control will get an error after a Connect method
        is called if the same local port has been used within the 
        past four minutes. 
        The Windows socket control does not have the ability to set the 
        socket option SO_REUSEADDR. This option allows the socket to be 
        bound to an address that is already in use. 
        When you close the handle to a socket, some additional 
        negotiation goes on between the client and the server. The socket 
        will wait for up to two times the maximum time that windows would 
        wait to receive an acknowledgement from the other end of the 
        socket that closed the port. By default, this option is set 
        to two minutes. Therefore, Windows may wait up to four minutes 
        before the port is actually released. 
        This makes that specific port unavailable until it is actually 
        released.
        If you are not using socket pooling, and/or your request model 
        is something like one request = one outbound TCP connection 
        (or more!).
        In this type of scenario, you can run out of ephemeral ports 
        (between 1024 and MaxUserPort) very quickly, and the problem 
        will scale with the load applied to the system, particularly 
        if a socket is acquired and abandoned with every request. 
        */
		closesocket(ConnectSocket);
	}

    printf("\r\nHit key to exit");
	while(!_kbhit());

	WSACleanup();

	return 0;
}

