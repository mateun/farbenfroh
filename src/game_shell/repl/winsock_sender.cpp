//
// Created by mgrus on 13.04.2025.
//

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>

#include "tcp_client.h"


// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")


#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"

void tcp_client_send(SOCKET ConnectSocket, const std::string& message) {
    int iResult = send( ConnectSocket, message.c_str(), message.size(), 0 );
    if (iResult == SOCKET_ERROR) {
        printf("send failed with error: %d\n", WSAGetLastError());
        tcp_client_stop(ConnectSocket);
    }

}

// Can be called repeatedly
void tcp_client_receive(SOCKET sock) {
    char recvbuf[DEFAULT_BUFLEN];
    int iResult;
    int recvbuflen = DEFAULT_BUFLEN;

    // select based receive
    fd_set readSet;
    FD_ZERO(&readSet);
    FD_SET(sock, &readSet);

    TIMEVAL timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;  // Non-blocking select

    int ready = select(0, &readSet, NULL, NULL, &timeout);
    if (ready > 0 && FD_ISSET(sock, &readSet))
    {
        iResult = recv(sock, recvbuf, recvbuflen, 0);
        if (iResult > 0)
            printf("Received: %.*s\n", iResult, recvbuf);
        else if (iResult == 0)
            printf("Server closed connection\n");
        else
            printf("recv failed: %d\n", WSAGetLastError());
    }
}


int tcp_client_start(const char* server_name, const char* port, SOCKET& ConnectSocket)
{
    WSADATA wsaData;

    struct addrinfo *result = NULL,
                    *ptr = NULL,
                    hints;
    const char *sendbuf = "tcp client initial packet";
    int iResult;


    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }

    ZeroMemory( &hints, sizeof(hints) );
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // Resolve the server address and port
    iResult = getaddrinfo(server_name, port, &hints, &result);
    if ( iResult != 0 ) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    // Attempt to connect to an address until one succeeds
    for(ptr=result; ptr != NULL ;ptr=ptr->ai_next) {

        // Create a SOCKET for connecting to server
        ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
            ptr->ai_protocol);
        if (ConnectSocket == INVALID_SOCKET) {
            printf("socket failed with error: %ld\n", WSAGetLastError());
            WSACleanup();
            return 1;
        }

        u_long nonBlocking = 1;
        ioctlsocket(ConnectSocket, FIONBIO, &nonBlocking);

        // Connect to server.
        // Initiate connection
        iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            int err = WSAGetLastError();
            if (err != WSAEWOULDBLOCK) {
                // An actual error occurred.
                printf("connect failed: %d\n", err);
                closesocket(ConnectSocket);
                ConnectSocket = INVALID_SOCKET;
                continue;
            }
            // Else, connection is in progress, so use select() to wait for it.
            fd_set writeSet, exceptSet;
            FD_ZERO(&writeSet);
            FD_ZERO(&exceptSet);
            FD_SET(ConnectSocket, &writeSet);
            FD_SET(ConnectSocket, &exceptSet);

            TIMEVAL timeout;
            timeout.tv_sec = 5;  // or however many seconds you'd like to wait
            timeout.tv_usec = 0;

            int selectResult = select(0, NULL, &writeSet, &exceptSet, &timeout);
            if (selectResult <= 0) {
                printf("Connection timed out or error in select()\n");
                closesocket(ConnectSocket);
                ConnectSocket = INVALID_SOCKET;
                continue;
            } else {
                // Check for socket error completion.
                int sockError = 0;
                int optLen = sizeof(sockError);
                getsockopt(ConnectSocket, SOL_SOCKET, SO_ERROR, (char*)&sockError, &optLen);
                if (sockError != 0) {
                    printf("Nonblocking connect failed after select, error: %d\n", sockError);
                    closesocket(ConnectSocket);
                    ConnectSocket = INVALID_SOCKET;
                    continue;
                }
            }
        }
        break;
    }

    freeaddrinfo(result);

    if (ConnectSocket == INVALID_SOCKET) {
        printf("Unable to connect to server!\n");
        WSACleanup();
        return 1;
    }








//    // Receive until the peer closes the connection
//    do {
//
//        iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
//        if ( iResult > 0 )
//            printf("Bytes received: %d\n", iResult);
//        else if ( iResult == 0 )
//            printf("Connection closed\n");
//        else
//            printf("recv failed with error: %d\n", WSAGetLastError());
//
//    } while( iResult > 0 );



    return 0;
}

void tcp_client_stop(SOCKET sock) {
    // shutdown the connection since no more data will be sent
    shutdown(sock, SD_SEND);
    closesocket(sock);
    WSACleanup();

}
