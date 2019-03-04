#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include<QMainWindow>
#include<QMessageBox>
#include<QFileDialog>
#include<string>
#include<QImageReader>
#include<QDebug>
#include<QTcpSocket>
#include<QBuffer>
using namespace std;
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pushButton1_clicked();

    void on_pushButton2_clicked();

    void on_pushButton3_clicked();

private:
    Ui::MainWindow *ui;
    bool is_open;//是否打开了图片
    bool is_recevied;//是否接收到了结果图片
    quint32 blocksize;//接收数据的大小
    QTcpSocket *tcpsocket;
    QImage *image;
    QImage *newImage;
    QImage *nowImage;//当前显示的图片
    bool clientSendData();//向服务器发送数据
    bool connectToServer();//连接服务器
private slots:
    void clientRecvData();//接收数据
    void on_pushButton4_clicked();
};
#endif // MAINWINDOW_H
