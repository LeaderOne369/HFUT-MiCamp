#ifndef SERVER_WINDOW_H
#define SERVER_WINDOW_H

#include <QMainWindow>
#include <QTextEdit>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QThread>
#include <QMutex>
#include <QDateTime>
#include <QListWidget>
#include <QStatusBar>
#include <QString>
#include <QMessageBox>
#include <atomic>

#include "../include/socket.h"

// 服务器工作线程类
class SrvThread : public QThread
{
    Q_OBJECT

public:
    SrvThread(QObject *parent = nullptr);
    ~SrvThread();

    void stop();

signals:
    void log(const QString &msg, bool isErr = false);
    void clientConn(const QString &ip);
    void timeSent(const QString &time);
    void clientDisconn();
    void started();
    void stopped();
    void error(const QString &err);

protected:
    void run() override;

private:
    std::atomic<bool> m_run;
    int m_sock;
    const int PORT = 8888;
};

// 服务器主窗口类
class SrvWindow : public QMainWindow
{
    Q_OBJECT

public:
    SrvWindow(QWidget *parent = nullptr);
    ~SrvWindow();

private slots:
    void onLog(const QString &msg, bool isErr);
    void onClientConn(const QString &ip);
    void onTimeSent(const QString &time);
    void onClientDisconn();
    void onStarted();
    void onStopped();
    void onError(const QString &err);
    void onBtnClick();

private:
    QListWidget *m_log;
    QLabel *m_status;
    QPushButton *m_btn;
    QStatusBar *m_bar;

    SrvThread *m_thread;
    bool m_running;

    void updateBtn();
};

#endif // SERVER_WINDOW_H