ftp服务端
基于libevent的FTP服务端

libevent是一个开源的事件驱动网络库，旨在提供高效、可扩展的网络通信。它允许开发人员编写基于事件回调的网络应用程序，而无需关心底层网络细节。

以下是libevent的一些主要特点和用途：

事件驱动架构： libevent基于事件驱动的编程模型，应用程序通过注册回调函数来处理特定类型的事件，如套接字可读或可写，定时器到期等。这使得应用程序能够在事件发生时作出响应，而不需要轮询。
跨平台支持： libevent被设计为跨平台的库，可以在不同的操作系统上运行，如Linux、Windows、macOS等。
多种I/O多路复用支持： libevent提供了对多种I/O多路复用机制的封装，如select、poll、epoll（Linux）、kqueue（BSD）、IOCP（Windows）等。这使得应用程序可以选择适合其需求的最佳I/O多路复用方法。
定时器支持： 应用程序可以使用libevent创建定时器，以在指定的时间间隔内执行特定操作，或在未来的某个时间点触发事件。
高性能： libevent的设计目标之一是提供高性能的网络通信，它使用了各种技术来优化事件处理和I/O操作，以确保在高负载情况下仍能保持良好的性能。
支持多种协议： libevent不仅适用于TCP和UDP通信，还支持UNIX域套接字、SSL加密通信等。
可扩展性： libevent的设计允许开发人员根据需要添加自定义事件处理器和特定协议的支持。
广泛应用： libevent被广泛用于各种类型的网络应用程序，如Web服务器、代理服务器、网络游戏服务器等，以及需要高性能网络通信的应用领域。
代码结构
主程序

main.cpp 

测试输出

testUtil.h

命令工厂

XFtpFactory.class 单例

XFtpUSER.class ： XFtpTask

XFtpPORT.class ： XFtpTask

XFtpLIST.class ： XFtpTask

XFtpRETR.class ： XFtpTask

XFtpSTOR.class ： XFtpTask

命令任务

XFtpTask.class : XTask

XFtpServerCMD.class : XFtpTask

线程池(基于libevent)

XTask.h

XThread.class

XThreadPool.class 单例

主要的技术栈：线程池、libevent 工厂设计模式 单例设计模式 多线程 网络编程

文件服务器 FTP协议

代码流程
1.线程池初始化为10个线程

2.使用evconnlistener_new_bind监听21端口

3.有用户连接通过XFtpFactory::CreateTask创建一个任务，这个任务注册了USER,PORT,PWD,LIST,CWD,CDUP,RETR,STOR 命令，然后把这个任务丢给线程池处理

XTask *XFtpFactory::CreateTask() {

  testout("At XFtpFactory::CreateTask");

  XFtpServerCMD *x = new XFtpServerCMD();

  x->Reg("USER", new XFtpUSER());

  x->Reg("PORT", new XFtpPORT());

  XFtpTask *list = new XFtpLIST();

  x->Reg("PWD", list);

  x->Reg("LIST", list);

  x->Reg("CWD", list);

  x->Reg("CDUP", list);

  x->Reg("RETR", new XFtpRETR());

  x->Reg("STOR", new XFtpSTOR());

  

  return x;

}
ftp客户端
基于qt和ftplib

由于服务端只实现了主动传输(PORT)，所以客户端使用主动传输模式

文件列表使用QTableWidget显示

状态栏显示当前连接状态，当前路径，当前时间(使用定时器更新时间)

操作使用右键文件列表来操作

有刷新(LIST)，返回上一级(CDUP)，上传(STOR)，下载(RETR)，删除(DELE)功能

菜单栏有连接(USER,PASS)，退出功能
