# netlib

##简介

 - netlib是一个基于Reactor Pattern的多线程C++网络库。
 - 支持平台：Linux。
 - 开发语言：C++。

##使用

 - 安装：`make install`
 - 卸载：`make uninstall`

##主要模块介绍

 - Reactor。使用non-blocking IO配合IO multiplexing配合应用层Buffer。用户注册接受连接、接收数据的Event Callback，IO线程运行Event Loop，以Event Driven的方式实现业务逻辑。
 - Multithread。支持4种IO计算模型，默认使用one loop per thread配合thread pool。主IO线程接受连接并分配至子IO线程，连接的IO任务由所属的IO线程完成，计算任务由thread pool完成。
 - EventLoopEpoller。使用epoll level-trigger模式实现IO复用，Channel负责注册和分发事件回调，TimerQueue使用timerfd实现Add/CancelTimer。EventLoop使用eventfd实现异步唤醒IO线程执行任务回调，使用RunInLoop实现无锁、线程安全的跨线程调用。
 - Thread Lib。使用RAII手法封装非递归mutex做到Scoped Locking。Condition解决虚假唤醒，区分signal/broadcast语义。Thread支持创建/销毁线程，使用tid标识线程。使用任务队列实现适合多消费者的ThreadPool。
 - Buffer。使用动态char数组保存数据，支持前方高效地添加数据、内部移动数据，大小自动适应。
 - Read。使用TcpConnection的input buffer、IO线程的stack buffer配合readv()实现节约内存、效率高且公平的数据读取。
 - Write。使用TcpConnection的output buffer配合EventLoop::RunInLoop实现非阻塞、线程安全且保证消息完整的数据发送。
 - Connection。使用std::shared_ptr管理TcpConnection的生命期；使用shutdown(2)关闭连接，保证收发数据的完整性。
 - TcpClient。能主动发起TCP连接，带back-off地重试直至建立连接；能在连接断开后自动重新连接；能主动断开连接。

##关于

 - Blog：http://blog.csdn.net/gaoxiangnumber1
 - E-mail：gaoxiangnumber1@126.com 欢迎您对我的项目/博客提出任何建议和批评。
 - 如果您觉得本项目不错或者对您有帮助，请赏颗星吧！谢谢您的阅读！
