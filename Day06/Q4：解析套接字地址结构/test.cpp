#include <iostream>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <cstring>
#include <iomanip>

using namespace std;

int main()
{
    struct sockaddr_in addr; // 题目要求的地址结构体
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080); // 字节序转换
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    memset(addr.sin_zero, 0, 8);

    struct sockaddr *addr_ptr = (struct sockaddr *)&addr;

    cout << "套接字地址结构：" << endl;
    cout << "  协议族：" << hex << setw(4) << setfill('0') << addr.sin_family << endl;
    cout << "  端口号：" << hex << setw(4) << setfill('0') << addr.sin_port << endl;
    cout << "  IP地址：" << hex << setw(8) << setfill('0') << addr.sin_addr.s_addr << endl;
    cout << "  填充字段：";
    for (int i = 0; i < 8; i++)
    {
        cout << hex << setw(2) << setfill('0') << static_cast<int>(addr.sin_zero[i]) << " ";
    }
    cout << endl;

    cout << "通用地址数据：";
    for (int i = 0; i < 14; i++)
    {
        cout << hex << setw(2) << setfill('0') << static_cast<int>(addr_ptr->sa_data[i]) << " ";
    }
    cout << endl;

    cout << "主机字节序端口：" << dec << ntohs(addr.sin_port) << endl;
    cout << "可读IP地址：" << inet_ntoa(addr.sin_addr) << endl;

    return 0;
}