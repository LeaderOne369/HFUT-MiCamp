#ifndef COMMON_H
#define COMMON_H

#include <iostream>
#include <cstring>
#include <QtCore/QtGlobal>
#include <QtCore/QTime>
#include <QtCore/QDebug>
#include <QtNetwork/QHostAddress>

#define PORT 8080
#define MAX_DATA_SIZE 1024

struct Packet
{
    quint16 len;
    char data[MAX_DATA_SIZE];
    quint16 checksum;
};

quint16 calcChecksum(const char *data, size_t len)
{
    quint16 checksum = 0;
    for (size_t i = 0; i < len; ++i)
    {
        checksum ^= data[i];
    }
    return checksum;
}

#endif