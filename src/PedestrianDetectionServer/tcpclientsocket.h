#ifndef TCPCLIENTSOCKET_H
#define TCPCLIENTSOCKET_H

#include <QTcpSocket>
#include <QObject>
#include <QBuffer>
#include <QDataStream>
#include <QImageReader>
#include <QDebug>
#include <opencv.hpp>
#include <opencv2/ml.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
using namespace cv;
using namespace std;
class TcpClientSocket : public QTcpSocket
{
    Q_OBJECT
public:
    TcpClientSocket(QObject *parent=0);
protected slots:
    void dataReceived();//接收自己的数据
    void slotDisconnected();//调用该槽函数将断开连接信号发射出去
private:
    quint32 blocksize;//接收数据的大小
    QImage *image;
    QImage *newImage;
    void imageProcess();//图片处理
    void dataSend();//向自己发送数据
    QImage cvMat2QImage(const Mat &inmat);//Mat转QImage
    Mat QImage2cvMat(const QImage &inImage, bool inCloneImageData = true);//QImage转Mat
signals:
        void clientDisconnected(int);//该客户端断开连接，发送该信号
};

#endif // TCPCLIENTSOCKET_H
