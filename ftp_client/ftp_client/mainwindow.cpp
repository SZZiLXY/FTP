#include <ftplib.h>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QLabel>
#include <QTime>
#include <QTimerEvent>
#include <QMessageBox>
#include <QCloseEvent>
#include <ftpconnectdlg.h>
#include <QFileDialog>

void MainWindow::init_menu()
{
    QMenu* menu = new QMenu(ui->list_file);
    auto cdupAction = menu->addAction("返回上一级目录");
    auto refAction = menu->addAction("刷新");
    auto putAction = menu->addAction("上传");
    auto getAction = menu->addAction("下载");
    auto delAction = menu->addAction("删除");
    ui->list_file->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->list_file,&QTableWidget::customContextMenuRequested,[=](const QPoint& pos){
        qDebug() << ui->list_file->currentRow();
        menu->exec(ui->list_file->viewport()->mapToGlobal(pos));
    });
    connect(cdupAction, &QAction::triggered, this,&MainWindow::cdupAction);
    connect(refAction, &QAction::triggered, this,&MainWindow::refAction);
    connect(putAction, &QAction::triggered, this,&MainWindow::putAction);
    connect(getAction, &QAction::triggered, this,&MainWindow::getAction);
    connect(delAction, &QAction::triggered, this,&MainWindow::delAction);
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    ,ui_connect_status("就绪")
    ,ui_pwd("当前路径：/")
    ,ui_cur_time(cur_time())
{
    ui->setupUi(this);
    ui->statusbar->addWidget(&ui_connect_status);
    ui->statusbar->addWidget(&ui_pwd);
    ui->statusbar->addPermanentWidget(&ui_cur_time);
    connect(ui->action_exit,&QAction::triggered,this,[=](){
        qDebug() << "action_exit";
        if(QMessageBox::question(this,this->windowTitle(),"你确定要退出吗？") != QMessageBox::Yes) return;
        exit(0);
    });
    connect(ui->action_connect,&QAction::triggered,this,&MainWindow::onConnect);
    timer1 = startTimer(10);

    ui->list_file->setColumnCount(7);
    ui->list_file->setHorizontalHeaderLabels({"权限","链接数","属主","属组","大小","日期","文件名"});

    connect(ui->list_file,&QTableWidget::cellDoubleClicked,this,&MainWindow::cellDoubleClicked);

    init_menu();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::timerEvent(QTimerEvent *e)
{
    if(e->timerId() != timer1)return;
    ui_cur_time.setText(cur_time());
}

void MainWindow::closeEvent(QCloseEvent *e)
{
    qDebug() << "closeEvent";
    if(QMessageBox::question(this,this->windowTitle(),"你确定要退出吗？") == QMessageBox::Yes)return;
    e->ignore();
}

void MainWindow::onConnect()
{
    FtpConnectDlg dlg;
    if(dlg.exec() != QDialog::Accepted) return;
    if(!ftp.login(dlg.ftp_data())) return;
    ui_connect_status.setText("连接成功：" + dlg.ftp_data().host);

    clear_ui_list();
    set_pwd();
    insert_list(ftp.dir());
}

void MainWindow::cellDoubleClicked(int row, int idx)
{
    if(row == -1)return;
    QString file_name = ui->list_file->item(row,6)->text();
    if(ui->list_file->item(row,0)->text()[0] != 'd')file_name = file_name.split(" ")[0];
    if(!ftp.cd(file_name)){
        QMessageBox::warning(this,this->windowTitle(),QString("%1切换失败，%1可能不是目录").arg(file_name));
        return;
    }
    set_pwd();
    auto list = ftp.dir();
    if(list.empty()){
        QMessageBox::warning(this,this->windowTitle(),QString("%1切换失败，%1可能不是目录").arg(file_name));
        return;
    }
    clear_ui_list();
    insert_list(list);
}

void MainWindow::cdupAction()
{
    if(!ftp.cdup()) return;
    set_pwd();
    clear_ui_list();
    insert_list(ftp.dir());
}

void MainWindow::refAction()
{
    clear_ui_list();
    insert_list(ftp.dir());
}

void MainWindow::putAction()
{
    QFileDialog file(this,"上传文件");
    file.setFileMode(QFileDialog::FileMode::ExistingFile);
    if(file.exec() != QDialog::Accepted)return;
    auto files = file.selectedFiles();
    if(files.size() > 1) {
        QMessageBox::warning(this,"选择文件过多！","最多选择一个文件");
        return;
    }
    qDebug() << files[0];
    if(!ftp.put(files[0])){
        QMessageBox::warning(this,this->windowTitle(),"上传失败\n错误：" + ftp.error());
        return;
    }
    QMessageBox::information(this,this->windowTitle(),"上传成功！");
    clear_ui_list();
    insert_list(ftp.dir());
}

void MainWindow::getAction()
{
    int row = ui->list_file->currentRow();
    if(row == -1){
        QMessageBox::warning(this,this->windowTitle(),"没有选择下载文件");
        return;
    }
    QFileDialog file(this,"下载文件到");
    file.setFileMode(QFileDialog::FileMode::AnyFile);
    file.selectFile(ui->list_file->item(row,6)->text());
    if(file.exec() != QDialog::Accepted)return;
    auto files = file.selectedFiles();
    if(files.size() > 1) {
        QMessageBox::warning(this,"选择文件过多！","最多选择一个文件");
        return;
    }
    qDebug() << files[0];
    if(!ftp.get(files[0],ui->list_file->item(row,6)->text())){
        QMessageBox::warning(this,this->windowTitle(),"下载失败\n错误：" + ftp.error());
        return;
    }
    QMessageBox::information(this,this->windowTitle(),"下载成功！");
}

void MainWindow::delAction()
{
    int row = ui->list_file->currentRow();
    if(row == -1){
        QMessageBox::warning(this,this->windowTitle(),"没有选择删除文件");
        return;
    }
    if(!ftp.del(get_file_name(row)))
    {
        QMessageBox::warning(this,this->windowTitle(),"删除失败\n错误：" + ftp.error());
        return;
    }
    QMessageBox::information(this,this->windowTitle(),"删除成功！");
    clear_ui_list();
    insert_list(ftp.dir());
}

QString MainWindow::cur_time()
{
    return QTime::currentTime().toString();
}

QString MainWindow::get_file_name(int row)
{
    auto file_name = ui->list_file->item(row,6)->text();
    if(file_name.contains(" ") && ui->list_file->item(row,0)->text()[0] == 'l')
        file_name = file_name.split(" ")[0];
    return file_name;
}

void MainWindow::set_pwd()
{
    ui_pwd.setText("当前路径：" + ftp.pwd());
}

void MainWindow::clear_ui_list()
{
    while(ui->list_file->rowCount() > 0)ui->list_file->removeRow(0);
}

void MainWindow::insert_list(const std::vector<FTP_FILE_INFO> &list)
{
    for(auto& info : list)
    {
        insert_row(ui->list_file->rowCount(),info);
    }
}

void MainWindow::insert_row(int row, const FTP_FILE_INFO &info)
{
    ui->list_file->insertRow(row);
    insert_item(row,0,info.access);
    insert_item(row,1,info.link_cnt);
    insert_item(row,2,info.ower);
    insert_item(row,3,info.group);
    insert_item(row,4,info.size);
    insert_item(row,5,info.date);
    insert_item(row,6,info.file_name);
}

void MainWindow::insert_item(int row,int idx,QString item)
{
    ui->list_file->setItem(row,idx,new QTableWidgetItem(item));
    ui->list_file->item(row,idx)->setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
}

