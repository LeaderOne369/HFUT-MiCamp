#include "common.h"
#include <QtNetwork/QUdpSocket>
#include <QApplication>
#include <QMainWindow>
#include <QTextEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QRandomGenerator>

class UDPServer : public QMainWindow
{
    Q_OBJECT
public:
    UDPServer(QWidget *parent = nullptr) : QMainWindow(parent)
    {
        setWindowTitle("UDP服务器");
        resize(500, 400);

        QWidget *central = new QWidget(this);
        setCentralWidget(central);

        QVBoxLayout *layout = new QVBoxLayout(central);

        // 状态标签
        statusLabel = new QLabel("准备启动...", this);
        layout->addWidget(statusLabel);

        // 日志显示区域
        logView = new QTextEdit(this);
        logView->setReadOnly(true);
        layout->addWidget(logView);

        // 控制按钮
        QHBoxLayout *btnLayout = new QHBoxLayout();
        startBtn = new QPushButton("启动服务器", this);
        stopBtn = new QPushButton("停止服务器", this);
        stopBtn->setEnabled(false);
        btnLayout->addWidget(startBtn);
        btnLayout->addWidget(stopBtn);
        layout->addLayout(btnLayout);

        // UDP套接字
        sock = new QUdpSocket(this);

        // 连接信号和槽
        connect(startBtn, &QPushButton::clicked, this, &UDPServer::startServer);
        connect(stopBtn, &QPushButton::clicked, this, &UDPServer::stopServer);
        connect(sock, &QUdpSocket::readyRead, this, &UDPServer::processPacket);
    }

private slots:
    void startServer()
    {
        if (sock->state() == QAbstractSocket::BoundState)
            return;

        if (!sock->bind(QHostAddress::Any, PORT))
        {
            logView->append("错误：绑定端口失败");
            return;
        }

        logView->append("服务器启动在端口" + QString::number(PORT));
        statusLabel->setText("服务器运行中");
        startBtn->setEnabled(false);
        stopBtn->setEnabled(true);
    }

    void stopServer()
    {
        sock->close();
        logView->append("服务器已停止");
        statusLabel->setText("服务器已停止");
        startBtn->setEnabled(true);
        stopBtn->setEnabled(false);
    }

    void processPacket()
    {
        QByteArray buffer;
        buffer.resize(sock->pendingDatagramSize());
        QHostAddress sender;
        quint16 senderPort;

        sock->readDatagram(buffer.data(), buffer.size(), &sender, &senderPort);

        if (buffer.size() >= static_cast<int>(sizeof(Packet)))
        {
            Packet pkt = *reinterpret_cast<Packet *>(buffer.data());
            logView->append("接收到: " + QString(pkt.data));

            quint16 recv_checksum = pkt.checksum;
            pkt.checksum = 0;
            quint16 calc_checksum = calcChecksum(pkt.data, pkt.len);

            QByteArray resp;
            if (recv_checksum == calc_checksum)
            {
                // 50%概率ACK，50%概率NACK
                int rand_val = QRandomGenerator::global()->bounded(100);
                if (rand_val < 50)
                {
                    resp = "ACK";
                }
                else
                {
                    resp = "NACK";
                }
            }
            else
            {
                resp = "NACK";
            }

            sock->writeDatagram(resp, sender, senderPort);
            logView->append("发送: " + QString(resp));
        }
    }

private:
    QTextEdit *logView;
    QPushButton *startBtn;
    QPushButton *stopBtn;
    QLabel *statusLabel;
    QUdpSocket *sock;
};

#include "server.moc"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    UDPServer server;
    server.show();
    return a.exec();
}