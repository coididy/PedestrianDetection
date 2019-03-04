#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    port=5550;

    //创建服务器
    //
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked()//点击运行服务器
{
    QString ip=ui->lineEdit->text().trimmed();
    server=new Server(this,ip,port);//开始监听
    QMessageBox::information(this,"提示","服务器运行成功！");
}
