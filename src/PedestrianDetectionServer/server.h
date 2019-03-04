#ifndef SERVER_H
#define SERVER_H

#include <QTcpServer>
#include <QObject>
#include "tcpclientsocket.h"
class Server : public QTcpServer
{
    Q_OBJECT
public:
    Server(QObject *parent=0,QString ip="", quint16 port=0);
    QList<TcpClientSocket*> tcpClientSocketList;//客户端列表
protected:
    void incomingConnection(qintptr socketDescriptor);//这是个虚函数，此处进行重载
public slots:
    void slotDisconnected(int);//某个客户端断开后
};

#endif // SERVER_H
