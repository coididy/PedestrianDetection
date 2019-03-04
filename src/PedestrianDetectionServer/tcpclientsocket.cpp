#include "tcpclientsocket.h"

TcpClientSocket::TcpClientSocket(QObject *parent)
{
    blocksize=0;
    image=new QImage;
    newImage=new QImage;
    connect(this,&TcpClientSocket::readyRead,this,&TcpClientSocket::dataReceived);
    connect(this,&TcpClientSocket::disconnected,this,&TcpClientSocket::slotDisconnected);
}

void TcpClientSocket::dataReceived()//接收自己的数据
{
    while(this->bytesAvailable()>0)
    {
        if(blocksize==0)
        {
            QDataStream in(this);
            in.setVersion(QDataStream::Qt_5_11);
            if(this->bytesAvailable()<sizeof(quint32))
                return;
            in>>blocksize;
        }

        if(this->bytesAvailable()<blocksize)
            return;

        QByteArray array = this->read(blocksize);//blockSize作read()的参数。
        QBuffer buffer(&array);
        buffer.open(QIODevice::ReadOnly);

        QImageReader reader(&buffer,"JPG");
        *image = reader.read();
        blocksize=0;
        if(!image->isNull())
        {
            blocksize=0;
            imageProcess();
            dataSend();
        }
    }
}

void TcpClientSocket::imageProcess()//对接收到的图片进行处理
{
    Mat *mat=new Mat;
    *mat=QImage2cvMat(*image);
    HOGDescriptor myHOG(Size(64,128),Size(16,16),Size(8,8),Size(8,8),9);
    vector<Rect>found;//矩形线框数组
    vector<Rect>found_filtered;//记录有效的矩形线框
    myHOG.setSVMDetector(HOGDescriptor::getDefaultPeopleDetector());
    myHOG.detectMultiScale(*mat,found,0,Size(8,8),Size(32,32),1.01,2);
    for(uint i=0; i < found.size(); i++)
    {
        Rect r = found[i];
        uint j=0;
        for(; j < found.size(); j++)
        {
            if(j != i && (r & found[j]) == r)  //按位与操作
                break;
        }
        if( j == found.size())
            found_filtered.push_back(r);
    }
    for(uint i=0; i<found_filtered.size(); i++)
    {
        Rect r = found_filtered[i];
        rectangle(*mat, r.tl(), r.br(), Scalar(255,0,0), 1);
    }
    *newImage=cvMat2QImage(*mat);//得到结果图片
}

void TcpClientSocket::dataSend()//将结果图片转发回去
{
    QByteArray image_data;
    QDataStream out(&image_data,QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_11);
    QBuffer buffer;
    newImage->save(&buffer,"JPG");//此处可能需要获取图片格式
    out<<static_cast<quint32>(buffer.data().size());
    image_data.append(buffer.data());
    if(this->isValid())
    {
        qint64 sendre=this->write(image_data);
        if(sendre==-1)
        {
            //QMessageBox::information(this,"提示","图片发送失败！");
            qDebug()<<"结果图片发送失败！";
            return;
        }
        qDebug()<<"结果图片发送成功！";
    }
    else
    {
        //QMessageBox::information(this,"提示","客户端断开！");
        qDebug()<<"客户端断开！";
        return ;
    }
}

void TcpClientSocket::slotDisconnected()//槽函数
{
    emit clientDisconnected(this->socketDescriptor());//发射断开连接信号
}

Mat TcpClientSocket::QImage2cvMat(const QImage &inImage, bool inCloneImageData)
{
    switch ( inImage.format() )
    {
    // 8-bit, 4 channel
    case QImage::Format_ARGB32:
    case QImage::Format_ARGB32_Premultiplied:
    {
        cv::Mat  mat( inImage.height(), inImage.width(),
                      CV_8UC4,
                      const_cast<uchar*>(inImage.bits()),
                      static_cast<size_t>(inImage.bytesPerLine())
                      );

        return (inCloneImageData ? mat.clone() : mat);
    }
        // 8-bit, 3 channel
    case QImage::Format_RGB32:
    {
        if ( !inCloneImageData )
        {
            qWarning() << "QImageToCvMat() - Conversion requires cloning so we don't modify the original QImage data";
        }

        cv::Mat  mat( inImage.height(), inImage.width(),
                      CV_8UC4,
                      const_cast<uchar*>(inImage.bits()),
                      static_cast<size_t>(inImage.bytesPerLine())
                      );

        cv::Mat  matNoAlpha;

        cv::cvtColor( mat, matNoAlpha, cv::COLOR_BGRA2BGR );   // drop the all-white alpha channel

        return matNoAlpha;
    }
        // 8-bit, 3 channel
    case QImage::Format_RGB888:
    {
        if ( !inCloneImageData )
        {
            qWarning() << "QImageToCvMat() - Conversion requires cloning so we don't modify the original QImage data";
        }

        QImage   swapped = inImage.rgbSwapped();

        return cv::Mat( swapped.height(), swapped.width(),
                        CV_8UC3,
                        const_cast<uchar*>(swapped.bits()),
                        static_cast<size_t>(swapped.bytesPerLine())
                        ).clone();
    }

        // 8-bit, 1 channel
    case QImage::Format_Indexed8:
    {
        cv::Mat  mat( inImage.height(), inImage.width(),
                      CV_8UC1,
                      const_cast<uchar*>(inImage.bits()),
                      static_cast<size_t>(inImage.bytesPerLine())
                      );

        return (inCloneImageData ? mat.clone() : mat);
    }
    default:
        qWarning() << "QImage2CvMat() - QImage format not handled in switch:" << inImage.format();
        break;
    }
    return cv::Mat();
}

QImage TcpClientSocket::cvMat2QImage( const cv::Mat &inMat )
{
    switch ( inMat.type() )
    {
    // 8-bit, 4 channel
    case CV_8UC4:
    {
        QImage image( inMat.data,inMat.cols, inMat.rows,static_cast<int>(inMat.step),QImage::Format_ARGB32 );
        return image;
    }
        // 8-bit, 3 channel
    case CV_8UC3:
    {
        QImage image( inMat.data,inMat.cols, inMat.rows,static_cast<int>(inMat.step),QImage::Format_RGB888 );
        return image.rgbSwapped();
    }
        // 8-bit, 1 channel
    case CV_8UC1:
    {
#if QT_VERSION >= QT_VERSION_CHECK(5, 5, 0)
        QImage image( inMat.data,inMat.cols, inMat.rows,static_cast<int>(inMat.step),QImage::Format_Grayscale8 );
#else
        static QVector<QRgb>  sColorTable;

        // only create our color table the first time
        if ( sColorTable.isEmpty() )
        {
            sColorTable.resize( 256 );

            for ( int i = 0; i < 256; ++i )
            {
                sColorTable[i] = qRgb( i, i, i );
            }
        }

        QImage image( inMat.data,
                      inMat.cols, inMat.rows,
                      static_cast<int>(inMat.step),
                      QImage::Format_Indexed8 );

        image.setColorTable( sColorTable );
#endif
        return image;
    }
    default:
        qWarning() << "cvMat2QImage() - cv::Mat image type not handled in switch:" << inMat.type();
        break;
    }
    return QImage();
}
