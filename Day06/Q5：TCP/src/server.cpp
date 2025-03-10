#include "../include/socket.h"
#include "server_window.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    SrvWindow window;
    window.show();

    return app.exec();
}