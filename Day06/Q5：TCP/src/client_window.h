#ifndef CLIENT_WINDOW_H
#define CLIENT_WINDOW_H

#include <QMainWindow>
#include <QTextEdit>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QtCore/QThread>
#include <QMutex>
#include <QString>
#include <QMessageBox>
#include <QStatusBar>
#include <atomic>

#include "../include/socket.h"

// 客户端状态枚举
enum CliState
{
    READY,
    CONNECTING,
    CONNECTED,
    RECEIVING,
    COMPLETE,
    ERROR
};

// 客户端工作线程类
class CliThread : public QThread
{
    Q_OBJECT

public:
    CliThread(const QString &srvIP, int port, QObject *parent = nullptr);
    ~CliThread();

signals:
    void stateChange(CliState state, const QString &msg);
    void timeRcv(const QString &time);
    void error(const QString &err);

protected:
    void run() override;

private:
    QString m_ip;
    int m_port;
};

// 客户端主窗口类
class CliWindow : public QMainWindow
{
    Q_OBJECT

public:
    CliWindow(QWidget *parent = nullptr);
    ~CliWindow();

private slots:
    void onBtnClick();
    void onStateChange(CliState state, const QString &msg);
    void onTimeRcv(const QString &time);
    void onError(const QString &err);

private:
    QLabel *m_status;
    QLabel *m_time;
    QLineEdit *m_ip;
    QPushButton *m_btn;
    QStatusBar *m_bar;

    CliState m_state;
    CliThread *m_thread;
    QString m_recvTime;

    void updateUI();
};

#endif // CLIENT_WINDOW_H