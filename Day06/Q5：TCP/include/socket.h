#ifndef SOCKET_H
#define SOCKET_H

#include <iostream>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>
#include <ctime>

using namespace std;

int createSock();
void closeSock(int sock);
bool bindSock(int sock, int port);
bool listenSock(int sock, int backlog = 5);
int acceptConn(int sock, string &clientIP);
bool connectSrv(int sock, const string &serverIP, int port);
bool sendData(int sock, const string &data);
string recvData(int sock);
string getTime();

#endif // SOCKET_H