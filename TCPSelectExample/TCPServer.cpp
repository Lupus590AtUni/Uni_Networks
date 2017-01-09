// UDPexample.cpp : Defines the entry point for the console application.
//
// Add WS2_32.lib to the Additional Dependencies for the Linker
//

#include "stdafx.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>

#define DEFAULT_PORT "27015"
#define DEFAULT_BUFLEN 512

typedef struct _SOCK_DATA
{
    _SOCK_DATA  *link;
    SOCKET      socket;
    char        writebuf[DEFAULT_BUFLEN];
    char        readbuf[DEFAULT_BUFLEN];
    DWORD       dwWriteBytes;
    DWORD       dwReadBytes;
    int         close;              // 1=closing, 2=closed

} SOCK_DATA;

int _tmain(int argc, _TCHAR* argv[])
{
    WSADATA	wsaData;
    int		iResult;

    struct addrinfo *addressList = NULL;
    struct addrinfo hints;

    SOCK_DATA   SocketList[FD_SETSIZE];
    SOCK_DATA   *FreeSocketList = NULL;
    SOCK_DATA   *UsedSocketList = NULL;
    SOCK_DATA   *ptr;
    SOCK_DATA   *prev;
    FD_SET      Writer;
    FD_SET      Reader;
    int         iUpdSockets = 0;    // Number of sockets updated
    ULONG       NonBlocking = 1;
    struct timeval  zerotime;    // the zero-time timeval structure
    int         index;

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

    // Make the listen socket non-blocking and use the select()
    // method to check for incoming connections.
    if (ioctlsocket(ListenSocket, FIONBIO, &NonBlocking) == SOCKET_ERROR)
    {
        printf("ioctlsocket() failed: %d\n", WSAGetLastError());
        return 1;
    }

    // Initialize the buffer list, linking them
    // altogether.
    prev = NULL;
    for(index = FD_SETSIZE-1; index >= 0; index--)
    {
        SocketList[index].link          = prev;
        SocketList[index].socket        = INVALID_SOCKET;
        SocketList[index].dwWriteBytes  = 0;
        SocketList[index].dwReadBytes   = 0;
        SocketList[index].close         = 0;
        prev = &SocketList[index];
    }
    FreeSocketList = &SocketList[0];

    // Could pass select() NULL instead of this structure
    // if you want it to block forever until something
    // happens.
    zerotime.tv_usec    = 0;    // Return immediately
    zerotime.tv_sec     = 0;

    while(true)
    {
        // Initialize the Read and Write socket set.
        FD_ZERO(&Reader);
        FD_ZERO(&Writer);

        // Check for connection attempts.
        FD_SET(ListenSocket, &Reader);

        // Add the active sockets to the File Descriptor
        // sets for the select() function call.
        ptr = UsedSocketList;
        while(ptr != NULL)
        {
            FD_SET(ptr->socket, &Writer);
            FD_SET(ptr->socket, &Reader);
            ptr = ptr->link;
        }

        // Using select() is an alternative approach to using multithreading
        // If the function times out without finding sockets with activity, it returns 0. 
        // If there is an error, the function returns -1. 
        // If there are sockets with activity, the function returns the number of sockets 
        // that have activity.
        // select() is the least efficient way to manage non-blocking I/O, because there is 
        // a lot of overhead associated with the function. Most of this overhead is a linear
        // function of the number of connections: double the number of connections, and you 
        // double the processing time.
        // About the only time you should use select() is for compatibility reasons: it's
        // the only non-blocking I/O strategy that works on all versions of Windows 
        // (including CE) and on virtually all POSIX-based systems.
        if ((iUpdSockets = select(0, &Reader, &Writer, NULL, &zerotime)) == SOCKET_ERROR)
        {
            printf("select failed: %d\n", WSAGetLastError());
            //return 1;
        }

        // Check for an incoming connection because the ListenSocket has
        // been configured to non-blocking.
        if (FD_ISSET(ListenSocket, &Reader))
        {
            iUpdSockets--;  // We've handled the listening socket..one less to process.

            // Accept a client socket
            if ((ClientSocket = accept(ListenSocket, NULL, NULL)) != INVALID_SOCKET)
            {
                // Make socket non-blocking
                if (ioctlsocket(ClientSocket, FIONBIO, &NonBlocking) == SOCKET_ERROR)
                {
                    printf("ioctlsocket() failed: %d\n", WSAGetLastError());
                    //return 1;
                }

                // Grab a queue element and store socket
                if(FreeSocketList != NULL)
                {
                    if(UsedSocketList != NULL)
                    {
                        UsedSocketList->link = FreeSocketList;
                    }
                    else 
                    {
                        UsedSocketList = FreeSocketList;
                    }
                    UsedSocketList->socket          = ClientSocket;
                    UsedSocketList->dwWriteBytes    = 0;
                    UsedSocketList->dwReadBytes     = 0;
                    UsedSocketList->close           = 0;

                    // Took top element off free list
                    FreeSocketList          = FreeSocketList->link;
                    UsedSocketList->link    = NULL;
                }
                else
                {
                    printf("Out of buffers error\n");
                }
            }
            else if (WSAGetLastError() != WSAEWOULDBLOCK)
            {
                printf("accept() failed: %d\n", WSAGetLastError());
                //return 1;
            }
        }

        ptr = UsedSocketList;
        prev = NULL;
        while(ptr != NULL && iUpdSockets > 0)
        {
            // Check if any data has been received on this socket
            if (FD_ISSET(ptr->socket, &Reader))
            {
                iUpdSockets--;

                iResult = recv(ptr->socket, ptr->readbuf, DEFAULT_BUFLEN, 0);
                if (iResult > 0)
                {
                    printf("Bytes received: %d Msg=%s\n", iResult, ptr->readbuf);
                    ptr->dwReadBytes += iResult;
                }
                else if (iResult == 0)
                {
                    printf("Connection closing...\n");
                    ptr->close = 1; // Closing
                }
                else  
                {
                    printf("recv failed: %d\n", WSAGetLastError());
                    closesocket(ClientSocket);
                }
            }

            // Check if buffer on this socket is available for more data
            // to be sent.
            if (FD_ISSET(ptr->socket, &Writer))
            {
                iUpdSockets--;

                if(ptr->dwReadBytes > 0)
                {
                    DWORD txlen = ptr->dwReadBytes < DEFAULT_BUFLEN?ptr->dwReadBytes:DEFAULT_BUFLEN;

                    // Echo the buffer back to the sender
                    memcpy(ptr->writebuf, ptr->readbuf, txlen);
                    iResult = send(ptr->socket, ptr->writebuf, txlen, 0 );
                    if (iResult == SOCKET_ERROR) 
                    {
                        printf("send failed: %d\n", WSAGetLastError());
                        ptr->close = 1;
                    }
                    else
                    {
                        ptr->dwWriteBytes += iResult;
                        printf("Bytes Sent: %ld\n", iResult);
                        ptr->dwReadBytes = 0;
                    }
                }

                if(ptr->close == 1)
                {
                    iResult = shutdown(ptr->socket, SD_SEND);
                    if (iResult == SOCKET_ERROR) 
                    {
                        printf("shutdown failed: %d\n", WSAGetLastError());
                    } 
                    closesocket(ptr->socket);

                    ptr->close = 2;
                }
            }

            if(ptr->close == 2)
            {
                if(FreeSocketList != NULL)
                {
                    FreeSocketList->link = ptr;
                }
                else
                {
                    FreeSocketList = ptr;
                }

                if(UsedSocketList == ptr)
                {
                    UsedSocketList = ptr->link;
                }

                if(prev != NULL)
                {
                    prev->link = ptr->link;   
                }

                ptr->link   = NULL;
                ptr->socket = INVALID_SOCKET;   
            }
            else
            {
                prev = ptr;
                ptr = ptr->link;
            }
        }
    }


    // When the client connection has been accepted, close the 
    // original socket, transfer control from the temporary socket 
    // to the original socket, and stop checking for new connections.
    // No longer need server socket.
    //
    closesocket(ListenSocket);

    while(!_kbhit());

    WSACleanup();

    return 0;
}

