#include "socket.h"

int createSock()
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1)
    {
        cerr << "创建套接字失败" << endl;
    }
    return sock;
}

void closeSock(int sock)
{
    close(sock);
}

bool bindSock(int sock, int port)
{
    sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(port);

    if (::bind(sock, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        cerr << "绑定失败" << endl;
        return false;
    }
    return true;
}

bool listenSock(int sock, int backlog)
{
    if (listen(sock, backlog) < 0)
    {
        cerr << "监听失败" << endl;
        return false;
    }
    return true;
}

int acceptConn(int sock, string &clientIP)
{
    sockaddr_in client;
    socklen_t clientLen = sizeof(client);

    int clientSock = accept(sock, (struct sockaddr *)&client, &clientLen);
    if (clientSock < 0)
    {
        cerr << "接受连接失败" << endl;
        return -1;
    }

    clientIP = inet_ntoa(client.sin_addr);
    return clientSock;
}

bool connectSrv(int sock, const string &serverIP, int port)
{
    sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(port);

    if (inet_pton(AF_INET, serverIP.c_str(), &server.sin_addr) <= 0)
    {
        cerr << "无效的IP地址" << endl;
        return false;
    }

    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        cerr << "连接失败" << endl;
        return false;
    }
    return true;
}

bool sendData(int sock, const string &data)
{
    if (send(sock, data.c_str(), data.length(), 0) < 0)
    {
        cerr << "发送失败" << endl;
        return false;
    }
    return true;
}

string recvData(int sock)
{
    char buffer[1024] = {0};
    int bytesRead = recv(sock, buffer, sizeof(buffer), 0);

    if (bytesRead <= 0)
    {
        if (bytesRead == 0)
        {
            return "";
        }
        cerr << "接收失败" << endl;
        return "";
    }

    return string(buffer, bytesRead);
}

string getTime()
{
    time_t now = time(NULL);
    struct tm *timeinfo = localtime(&now);
    char buffer[80];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);
    return string(buffer);
}