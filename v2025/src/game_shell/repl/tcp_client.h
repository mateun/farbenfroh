//
// Created by mgrus on 13.04.2025.
//

#ifndef TCP_CLIENT_H
#define TCP_CLIENT_H
#include <windows.h>


int tcp_client_start(const char* server_name, const char* port, SOCKET& connectSocket);
void tcp_client_send(SOCKET sock, const std::string& message);
void tcp_client_receive(SOCKET sock);
void tcp_client_stop(SOCKET sock);

#endif //TCP_CLIENT_H
