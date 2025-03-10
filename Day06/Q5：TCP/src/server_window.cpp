#include "server_window.h"

// ======== ServerThread 实现 ========

SrvThread::SrvThread(QObject *parent)
    : QThread(parent), m_run(false), m_sock(-1)
{
}

SrvThread::~SrvThread()
{
    stop();
    wait();
}

void SrvThread::stop()
{
    m_run = false;
}

void SrvThread::run()
{
    m_sock = createSock();
    if (m_sock == -1)
    {
        emit error("Socket 创建失败");
        return;
    }

    if (!bindSock(m_sock, PORT))
    {
        closeSock(m_sock);
        emit error("绑定失败");
        return;
    }

    if (!listenSock(m_sock))
    {
        closeSock(m_sock);
        emit error("监听失败");
        return;
    }

    m_run = true;
    emit started();
    emit log("服务器已启动，等待连接...");

    fd_set readfds;
    struct timeval tv;

    while (m_run)
    {
        FD_ZERO(&readfds);
        FD_SET(m_sock, &readfds);

        tv.tv_sec = 0;
        tv.tv_usec = 100000;

        int activity = select(m_sock + 1, &readfds, NULL, NULL, &tv);

        if (activity < 0)
        {
            emit log("选择错误", true);
            break;
        }

        if (activity == 0)
        {
            continue;
        }

        if (FD_ISSET(m_sock, &readfds))
        {
            string clientIP;
            int clientSock = acceptConn(m_sock, clientIP);

            if (clientSock >= 0)
            {
                emit clientConn(QString::fromStdString(clientIP));
                emit log("客户端连接成功, IP: " + QString::fromStdString(clientIP));

                string timeStr = getTime();
                emit timeSent(QString::fromStdString(timeStr));
                emit log("发送时间: " + QString::fromStdString(timeStr));

                if (!sendData(clientSock, timeStr))
                {
                    emit log("发送失败", true);
                }

                closeSock(clientSock);
                emit clientDisconn();
                emit log("客户端断开连接");
            }
        }
    }

    closeSock(m_sock);
    m_sock = -1;
    emit stopped();
    emit log("服务器已停止。");
}

// ======== ServerWindow 实现 ========

SrvWindow::SrvWindow(QWidget *parent)
    : QMainWindow(parent), m_running(false), m_thread(nullptr)
{

    setWindowTitle("TCP 时间服务器");
    resize(600, 400);

    QWidget *central = new QWidget(this);
    setCentralWidget(central);

    QVBoxLayout *layout = new QVBoxLayout(central);

    m_status = new QLabel("服务器未运行", this);
    m_status->setStyleSheet("font-weight: bold; color: red;");
    layout->addWidget(m_status);

    QLabel *logLabel = new QLabel("服务器日志:", this);
    layout->addWidget(logLabel);

    m_log = new QListWidget(this);
    layout->addWidget(m_log);

    QHBoxLayout *btnLayout = new QHBoxLayout();
    m_btn = new QPushButton("启动服务器", this);
    btnLayout->addStretch();
    btnLayout->addWidget(m_btn);
    layout->addLayout(btnLayout);

    m_bar = statusBar();
    m_bar->showMessage("准备就绪");

    connect(m_btn, &QPushButton::clicked, this, &SrvWindow::onBtnClick);

    updateBtn();
}

SrvWindow::~SrvWindow()
{
    if (m_thread)
    {
        m_thread->stop();
        m_thread->wait();
        delete m_thread;
    }
}

void SrvWindow::onLog(const QString &msg, bool isErr)
{
    QListWidgetItem *item = new QListWidgetItem(QDateTime::currentDateTime().toString("[hh:mm:ss] ") + msg);
    if (isErr)
    {
        item->setForeground(Qt::red);
    }
    m_log->addItem(item);
    m_log->scrollToBottom();
}

void SrvWindow::onClientConn(const QString &ip)
{
    m_bar->showMessage("客户端已连接: " + ip);
}

void SrvWindow::onTimeSent(const QString &time)
{
    // 未使用
}

void SrvWindow::onClientDisconn()
{
    m_bar->showMessage("客户端已断开连接");
}

void SrvWindow::onStarted()
{
    m_running = true;
    m_status->setText("服务器运行中 (端口: 8888)");
    m_status->setStyleSheet("font-weight: bold; color: green;");
    m_bar->showMessage("服务器已启动");
    updateBtn();
}

void SrvWindow::onStopped()
{
    m_running = false;
    m_status->setText("服务器未运行");
    m_status->setStyleSheet("font-weight: bold; color: red;");
    m_bar->showMessage("服务器已停止");
    updateBtn();
}

void SrvWindow::onError(const QString &err)
{
    QMessageBox::critical(this, "服务器错误", "错误: " + err);
    m_running = false;
    updateBtn();
}

void SrvWindow::onBtnClick()
{
    if (m_running)
    {
        if (m_thread)
        {
            m_thread->stop();
        }
    }
    else
    {
        if (m_thread)
        {
            m_thread->stop();
            m_thread->wait();
            delete m_thread;
        }

        m_thread = new SrvThread(this);

        connect(m_thread, &SrvThread::log, this, &SrvWindow::onLog);
        connect(m_thread, &SrvThread::clientConn, this, &SrvWindow::onClientConn);
        connect(m_thread, &SrvThread::timeSent, this, &SrvWindow::onTimeSent);
        connect(m_thread, &SrvThread::clientDisconn, this, &SrvWindow::onClientDisconn);
        connect(m_thread, &SrvThread::started, this, &SrvWindow::onStarted);
        connect(m_thread, &SrvThread::stopped, this, &SrvWindow::onStopped);
        connect(m_thread, &SrvThread::error, this, &SrvWindow::onError);

        m_thread->start();
    }
}

void SrvWindow::updateBtn()
{
    if (m_running)
    {
        m_btn->setText("停止服务器");
    }
    else
    {
        m_btn->setText("启动服务器");
    }
}