#include "client_window.h"

// ======== ClientThread 实现 ========

CliThread::CliThread(const QString &srvIP, int port, QObject *parent)
    : QThread(parent), m_ip(srvIP), m_port(port)
{
}

CliThread::~CliThread()
{
    wait();
}

void CliThread::run()
{
    emit stateChange(CONNECTING, "正在连接服务端。");

    int sock = createSock();
    if (sock == -1)
    {
        emit error("Socket 创建失败。");
        emit stateChange(ERROR, "Socket 创建失败。");
        return;
    }

    emit stateChange(CONNECTING, "正在连接到 " + m_ip + ":" + QString::number(m_port));

    if (!connectSrv(sock, m_ip.toStdString(), m_port))
    {
        closeSock(sock);
        emit error("连接失败。");
        emit stateChange(ERROR, "连接失败。");
        return;
    }

    emit stateChange(CONNECTED, "连接成功，正在等待数据。");

    emit stateChange(RECEIVING, "正在接收时间数据。");

    string timeStr = recvData(sock);

    closeSock(sock);

    if (!timeStr.empty())
    {
        emit timeRcv(QString::fromStdString(timeStr));
        emit stateChange(COMPLETE, "连接关闭。");
    }
    else
    {
        emit error("接收失败。");
        emit stateChange(ERROR, "接收失败。");
    }
}

// ======== ClientWindow 实现 ========

CliWindow::CliWindow(QWidget *parent)
    : QMainWindow(parent), m_state(READY), m_thread(nullptr)
{

    setWindowTitle("TCP 客户端");
    resize(400, 300);

    QWidget *central = new QWidget(this);
    setCentralWidget(central);

    QVBoxLayout *layout = new QVBoxLayout(central);

    QHBoxLayout *ipLayout = new QHBoxLayout();
    QLabel *ipLabel = new QLabel("服务端 IP:", this);
    m_ip = new QLineEdit("127.0.0.1", this);
    ipLayout->addWidget(ipLabel);
    ipLayout->addWidget(m_ip);
    layout->addLayout(ipLayout);

    m_btn = new QPushButton("连接", this);
    layout->addWidget(m_btn);

    QLabel *statusTitle = new QLabel("状态:", this);
    m_status = new QLabel("就绪", this);
    m_status->setStyleSheet("font-weight: bold;");
    QHBoxLayout *statusLayout = new QHBoxLayout();
    statusLayout->addWidget(statusTitle);
    statusLayout->addWidget(m_status);
    statusLayout->addStretch();
    layout->addLayout(statusLayout);

    QLabel *timeTitle = new QLabel("服务端时间:", this);
    m_time = new QLabel("", this);
    m_time->setStyleSheet("font-size: 16px; font-weight: bold;");
    m_time->setAlignment(Qt::AlignCenter);
    layout->addWidget(timeTitle);
    layout->addWidget(m_time);

    layout->addStretch();

    m_bar = statusBar();
    m_bar->showMessage("就绪");

    connect(m_btn, &QPushButton::clicked, this, &CliWindow::onBtnClick);

    updateUI();
}

CliWindow::~CliWindow()
{
    if (m_thread)
    {
        m_thread->wait();
        delete m_thread;
    }
}

void CliWindow::onBtnClick()
{
    if (m_state != READY && m_state != COMPLETE && m_state != ERROR)
    {
        return;
    }

    QString serverIP = m_ip->text().trimmed();
    if (serverIP.isEmpty())
    {
        serverIP = "127.0.0.1";
        m_ip->setText(serverIP);
    }

    m_recvTime = "";
    m_time->setText("");

    if (m_thread)
    {
        m_thread->wait();
        delete m_thread;
    }

    m_thread = new CliThread(serverIP, 8888, this);

    connect(m_thread, &CliThread::stateChange, this, &CliWindow::onStateChange);
    connect(m_thread, &CliThread::timeRcv, this, &CliWindow::onTimeRcv);
    connect(m_thread, &CliThread::error, this, &CliWindow::onError);

    m_thread->start();
}

void CliWindow::onStateChange(CliState state, const QString &msg)
{
    m_state = state;
    m_status->setText(msg);
    m_bar->showMessage(msg);
    updateUI();
}

void CliWindow::onTimeRcv(const QString &time)
{
    m_recvTime = time;
    m_time->setText(time);
}

void CliWindow::onError(const QString &err)
{
    QMessageBox::critical(this, "连接错误", err);
}

void CliWindow::updateUI()
{
    switch (m_state)
    {
    case READY:
        m_status->setStyleSheet("font-weight: bold; color: black;");
        m_btn->setEnabled(true);
        m_btn->setText("连接");
        m_ip->setEnabled(true);
        break;

    case CONNECTING:
    case CONNECTED:
    case RECEIVING:
        m_status->setStyleSheet("font-weight: bold; color: blue;");
        m_btn->setEnabled(false);
        m_ip->setEnabled(false);
        break;

    case COMPLETE:
        m_status->setStyleSheet("font-weight: bold; color: green;");
        m_btn->setEnabled(true);
        m_btn->setText("重新连接");
        m_ip->setEnabled(true);
        break;

    case ERROR:
        m_status->setStyleSheet("font-weight: bold; color: red;");
        m_btn->setEnabled(true);
        m_btn->setText("再试一次");
        m_ip->setEnabled(true);
        break;
    }
}