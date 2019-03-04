#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    image=new QImage;
    newImage=new QImage;
    nowImage=new QImage;
    tcpsocket=new QTcpSocket;
    is_open=false;
    is_recevied=false;
    blocksize=0;
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton1_clicked()//打开一张图片
{
    QString filename=QFileDialog::getOpenFileName(this,tr("打开图片"),".","image Files(*.jpg *.png *.jpeg *.bmp)");
    if(filename.isEmpty())
        return;
    else
    {
        if(!(image->load(filename))) //加载图像
        {
            QMessageBox::information(this, tr("打开图像失败"),tr("打开图像失败!"));
            return;
        }
        QImageReader imgReader(filename);
        imgReader.read(image);
        *image=image->convertToFormat(QImage::Format_ARGB32);
        nowImage=image;
        ui->label->resize(image->width(),image->height());
        ui->label->setPixmap(QPixmap::fromImage(*image));
        is_open=true;
        is_recevied=false;
    }
}

void MainWindow::on_pushButton2_clicked()//检测
{
    if(!is_open)
    {
        QMessageBox::information(this,"提示","请先选择一张图片！");
        return;
    }
    //连接到服务器
    if(!connectToServer())//未连接
    {
        return;
    }
    //发送图片
    if(!clientSendData())//上传失败
    {
        return;
    }
    else
    {
        QMessageBox::information(this,"提示","图片上传成功！");
    }
}

bool MainWindow::clientSendData()//发送图片
{
    QByteArray image_data;
    QDataStream out(&image_data,QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_11);
    QBuffer buffer;
    image->save(&buffer,"JPG");//此处可能需要获取图片格式
    out<<static_cast<quint32>(buffer.data().size());
    image_data.append(buffer.data());
    if(tcpsocket->isValid())
    {
        qint64 sendre=tcpsocket->write(image_data);
        if(sendre==-1)
        {
            QMessageBox::information(this,"提示","图片上传失败！");
            return false;
        }
    }
    else
    {
        QMessageBox::information(this,"提示","服务器断开！");
        return false;
    }
    return true;
}

bool MainWindow::connectToServer()//连接服务器
{
    QString ip=ui->lineEdit1->text().trimmed();
    if(ip.isEmpty())
    {
        ip="127.0.0.1";
    }
    //QString ip="47.93.241.205";//公网
    //QString ip="172.17.226.246";//私网
    //QString ip="127.0.0.1";
    quint16 port=5550;
    tcpsocket->connectToHost(ip,port);
    if(!tcpsocket->waitForConnected(30000))
    {
        QMessageBox::information(this, "提示", "连接服务器失败！");
        return false;
    }
    connect(tcpsocket,&QTcpSocket::readyRead, this, &MainWindow::clientRecvData);
    return true;
}

void MainWindow::clientRecvData()//接收数据
{
    while(tcpsocket->bytesAvailable()>0)
    {
        if(blocksize==0)
        {
            QDataStream in(tcpsocket);
            in.setVersion(QDataStream::Qt_5_11);
            if(tcpsocket->bytesAvailable()<sizeof(quint32))
                return;
            in>>blocksize;
        }

        if(tcpsocket->bytesAvailable()<blocksize)
            return;

        QByteArray array = tcpsocket->read(blocksize);//blockSize作read()的参数。
        QBuffer buffer(&array);
        buffer.open(QIODevice::ReadOnly);

        QImageReader reader(&buffer,"JPG");//图片格式待定
        *newImage=reader.read();
        blocksize=0;
        if(!newImage->isNull())
        {
            qDebug()<<"结果图片接收成功！";
            is_recevied=true;
            blocksize=0;
        }
    }
}

void MainWindow::on_pushButton3_clicked()//显示结果
{
    if(is_recevied)
    {
        ui->label->resize(newImage->width(),newImage->height());
        ui->label->setPixmap(QPixmap::fromImage(*newImage));
        nowImage=newImage;
    }
}

void MainWindow::on_pushButton4_clicked()//另存为
{
    if(!is_open)
    {
        QMessageBox::information(this,tr("提示！"),tr("请先打开一张图片！再进行该项操作！"));
        return;
    }
    QString filename1 = QFileDialog::getSaveFileName(this,tr("Save Image"),".",tr("所有文件 (*)")); //选择路径
    if(nowImage->save(filename1))//第二个参数表示若没有指明保存的格式，会根据扩展名来自动辨别，第三个参数表示图片质量，默认为-1
    {
        QMessageBox::information(this,tr("提示！"),tr("保存成功！"));
    }
    else
    {
        QMessageBox::information(this,tr("提示！"),tr("保存失败！"));
    }
}
