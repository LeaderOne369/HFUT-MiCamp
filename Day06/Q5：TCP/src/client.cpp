#include "../include/socket.h"
#include "client_window.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    CliWindow window;
    window.show();

    return app.exec();
}