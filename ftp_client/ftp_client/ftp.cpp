#include "ftp.h"
#include <QFile>
#include <QDebug>
#include <QFileInfo>

Ftp::Ftp():
    ftp(new ftplib())
{
    ftp->SetConnmode(ftplib::port);
}

Ftp::~Ftp()
{
    delete ftp;
}

bool Ftp::login(const FTP_DATA &data)
{
    if(!ftp->Connect(data.host.toStdString().c_str())) return false;
    return ftp->Login(data.user.toStdString().c_str(),data.pass.toStdString().c_str());
}


QString Ftp::pwd()
{
    QByteArray arr(0x100,'\0');
    ftp->Pwd(arr.data(),arr.size());
    return arr;
}

std::vector<FTP_FILE_INFO> Ftp::dir()
{
    //ftp->Dir("dir.txt",cur_path.c_str());
    if(!ftp->Dir("dir.txt",pwd().toStdString().c_str())) return {};
    QFile file(R"(dir.txt)");
    file.open(QFile::ReadOnly);
    QString list = file.readAll();
    file.close();
    std::vector<FTP_FILE_INFO> _list;
    QStringList lines = list.split("\r\n");
    for(auto& line : lines)
    {
        auto args = line.split(" ");
        args.removeAll("");
        if(args.size() < 9)continue;
        QString date = args[5] + " " + args[6] +  " " + args[7];
        QString file_name = args[8];
        for(int i{9};i < args.size();i++) file_name +=  " " + args[i];
        _list.push_back({
            .access = args[0]
            ,.link_cnt = args[1]
            ,.ower = args[2]
            ,.group = args[3]
            ,.size = args[4]
            ,.date = date
            ,.file_name = file_name
        });
    }
    return _list;
}

bool Ftp::cd(const QString &path)
{
    return ftp->Chdir(path.toStdString().c_str());
}

bool Ftp::cdup()
{
    return ftp->Cdup();
}

bool Ftp::put(const QString &put_file)
{
//    return ftp->Put(put_file.toStdString().c_str(),"test.txt",ftplib::image);
    return ftp->Put(put_file.toStdString().c_str(),QFileInfo(put_file).fileName().toStdString().c_str(),ftplib::image);
}

bool Ftp::get(const QString &get_file,const QString& remote_file)
{
    return ftp->Get(get_file.toStdString().c_str(),remote_file.toStdString().c_str(),ftplib::image);
}

bool Ftp::del(const QString &file)
{
    bool ret = ftp->Delete(file.toStdString().c_str());
    if(ret) return true;
    return ftp->Rmdir(file.toStdString().c_str());
}

QString Ftp::error()
{
    return ftp->LastResponse();
}
