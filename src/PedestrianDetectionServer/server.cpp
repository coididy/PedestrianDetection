#include "server.h"

Server::Server(QObject *parent,QString ip,quint16 port)
    :QTcpServer(parent)
{
    if(ip=="")
    {
        if(!this->listen(QHostAddress::Any,port))//服务器在本地
        {
            //QMessageBox::information(this,"QT网络通信","服务器端监听失败");
            qDebug()<<"服务器监听失败！";
        }
    }
    else
    {
        if(!this->listen(QHostAddress(ip),port))
        {
            //QMessageBox::information(this,"QT网络通信","服务器端监听失败");
            qDebug()<<"服务器监听失败！";
        }
    }
}

void Server::incomingConnection(qintptr socketDescriptor)//有新连接时会自动调用该函数
{
    //有新连接，即有新的客户端
    TcpClientSocket *tcpClientSocket=new TcpClientSocket(this);//新建一个客户端，客户端类自动接收相应的数据
    connect(tcpClientSocket,&TcpClientSocket::clientDisconnected,this,&Server::slotDisconnected);

    tcpClientSocket->setSocketDescriptor(socketDescriptor);//设置套接字描述符

    tcpClientSocketList.append(tcpClientSocket);//加入客户端列表
}

void Server::slotDisconnected(int descriptor)//descriptor表示的客户端断开了连接，将他移出
{
    for(int i=0;i<tcpClientSocketList.count();i++)
    {
        QTcpSocket *item = tcpClientSocketList.at(i);
        if(item->socketDescriptor()==descriptor)
        {
            tcpClientSocketList.removeAt(i);
            return;
        }
    }
    return;
}
