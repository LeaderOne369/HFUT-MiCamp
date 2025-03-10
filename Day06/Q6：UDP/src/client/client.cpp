#include "common.h"
#include <QtNetwork/QUdpSocket>
#include <QApplication>
#include <QMainWindow>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QTextEdit>
#include <QLabel>
#include <QTimer>

class UDPClient : public QMainWindow
{
    Q_OBJECT
public:
    UDPClient(QWidget *parent = nullptr) : QMainWindow(parent)
    {
        setWindowTitle("UDP客户端");
        resize(500, 400);

        QWidget *central = new QWidget(this);
        setCentralWidget(central);

        QVBoxLayout *layout = new QVBoxLayout(central);

        // 消息历史显示区域
        msgHistory = new QTextEdit(this);
        msgHistory->setReadOnly(true);
        layout->addWidget(msgHistory);

        // 输入区域
        QHBoxLayout *inputLayout = new QHBoxLayout();
        msgInput = new QLineEdit(this);
        sendBtn = new QPushButton("发送", this);
        inputLayout->addWidget(msgInput);
        inputLayout->addWidget(sendBtn);
        layout->addLayout(inputLayout);

        // 状态标签
        statusLabel = new QLabel("就绪", this);
        layout->addWidget(statusLabel);

        // 网络设置
        sock = new QUdpSocket(this);
        serverAddr = QHostAddress::LocalHost;
        serverPort = PORT;

        // 设置超时重传
        timer = new QTimer(this);
        timer->setSingleShot(true);

        // 连接信号和槽
        connect(sendBtn, &QPushButton::clicked, this, &UDPClient::sendMessage);
        connect(msgInput, &QLineEdit::returnPressed, this, &UDPClient::sendMessage);
        connect(sock, &QUdpSocket::readyRead, this, &UDPClient::readResponse);
        connect(timer, &QTimer::timeout, this, &UDPClient::retransmit);
    }

private slots:
    void sendMessage()
    {
        if (waiting_for_response)
        {
            msgHistory->append("等待服务器响应，请稍后...");
            return;
        }

        QString msg = msgInput->text().trimmed();
        if (msg.isEmpty())
            return;

        msgInput->clear();

        strncpy(current_pkt.data, msg.toUtf8().constData(), MAX_DATA_SIZE);
        current_pkt.len = strlen(current_pkt.data);
        current_pkt.checksum = calcChecksum(current_pkt.data, current_pkt.len);

        sock->writeDatagram(reinterpret_cast<char *>(&current_pkt), sizeof(current_pkt), serverAddr, serverPort);
        msgHistory->append("发送: " + QString(current_pkt.data));

        timer->start(3000); // 3秒超时
        waiting_for_response = true;
        statusLabel->setText("等待响应...");
    }

    void readResponse()
    {
        if (!waiting_for_response)
            return;

        QByteArray reply;
        reply.resize(sock->pendingDatagramSize());
        QHostAddress sender;
        quint16 senderPort;

        sock->readDatagram(reply.data(), reply.size(), &sender, &senderPort);

        if (!reply.isEmpty())
        {
            timer->stop(); // 收到响应，停止计时器

            if (reply == "ACK")
            {
                msgHistory->append("收到确认消息");
                statusLabel->setText("消息已确认");
            }
            else if (reply == "NACK")
            {
                msgHistory->append("收到否认消息，正在重新发送");
                sock->writeDatagram(reinterpret_cast<char *>(&current_pkt), sizeof(current_pkt), serverAddr, serverPort);
                msgHistory->append("发送: " + QString(current_pkt.data));
                timer->start(3000); // 3秒超时
                return;
            }
            else
            {
                msgHistory->append("未知回复: " + QString(reply));
            }

            waiting_for_response = false;
            statusLabel->setText("就绪");
        }
    }

    void retransmit()
    {
        if (waiting_for_response)
        {
            msgHistory->append("响应超时，重新发送");
            sock->writeDatagram(reinterpret_cast<char *>(&current_pkt), sizeof(current_pkt), serverAddr, serverPort);
            msgHistory->append("发送: " + QString(current_pkt.data));
            timer->start(3000); // 3秒超时
        }
    }

private:
    QTextEdit *msgHistory;
    QLineEdit *msgInput;
    QPushButton *sendBtn;
    QLabel *statusLabel;

    QUdpSocket *sock;
    QHostAddress serverAddr;
    quint16 serverPort;
    QTimer *timer;

    bool waiting_for_response = false;
    Packet current_pkt;
};

#include "client.moc"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    UDPClient client;
    client.show();
    return a.exec();
}