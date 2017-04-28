# netlib

##简介
 - netlib是一个基于Reactor Pattern的多线程C++网络库。
 - 支持平台：Linux。
 - 开发语言：C++。

##使用
 - 安装：`make install`
 - 卸载：`make uninstall`

##主要模块介绍
 - 线程库。使用RAII手法封装非递归mutex实现Scoped Locking，实现条件变量、CountDownLatch、Thread支持线程创建和等待结束、ThreadPool支持动态添加Task。
 - Reactor。使用one loop per thread模型，IO线程创建EventLoop，TimerQueue实现Add/CancelTimer接口，Epoller实现IO multiplexing，Channel分发IO events。
 - Read/Write。通过Buffer读写数据，使用readv和栈空间实现兼顾内存使用和效率的Read，使用Send和HandleWrite实现线程安全、无阻塞Write。
 - TcpServer。将TcpConnection按round robin分配到EventLoopThreadPool中，通过EventLoop::RunInLoop实现线程安全的跨线程调用。
 - TcpClient。能主动发起TCP连接，带back-off地重试至建立连接；能在连接断开后自动重新连接；能主动断开连接。

##example

###RAII技法example/raii/
 - raii.cc展示了RAII技法的运行结果。

###Echo服务器example/echo/
 - single_thread_echo_server.cc是一个单线程并发Echo服务器。

###数独服务器example/sudoku
 - sudoku.cc使用回溯法解决数独问题，有解问题用时2秒钟左右，无解问题用时5秒钟左右。
 - single_thread_sudoku.cc单线程服务器，所有连接的IO、Compute都在一个线程完成。
 - thread_pool_sudoku.cc主线程做所有连接的IO，线程池做所有连接的Compute。
 - multiloop_sudoku.cc主线程为每个客户连接分配一个IO子线程，该子线程完成该连接上的所有IO和Compute。
 - multiloop_thread_pool_sudoku.cc主线程为每个客户连接分配一个IO子线程，该子线程完成该连接上的所有IO，线程池完成所有连接上的Compute。

###传输服务器example/sendfile
 - send_file_once.cc每个连接建立后，把文件内容一次性全部读入一个字符串，调用Send发送。内存使用正比于“并发连接数*文件大小”。
 - send_file_block.cc采用流水线思路，首先发送前64KB数据，然后使用WriteCompleteCallback继续读取后面的数据，直到fread返回0，文件读取完毕。内存使用正比于“并发连接数*缓冲区大小”，与文件大小无关。

###聊天服务器example/chat
 - codec.cc实现TCP字节流打包、分包，由server和client共用。
 - single_thread_chat_server.cc单线程并发服务器。
 - multithread_chat_server.cc主线程接受连接，为每个连接分配一个EventLoop线程，使用MutexLock保护共享数据。
 - multithread_cow_chat_server.cc主线程接受连接，为每个连接分配一个EventLoop线程，使用shared_ptr实现对共享数据的Copy-On-Write。
 - chat_client.cc双线程，main函数线程读取stdin，EventLoopThread线程完成该连接上的IO。




##关于
 - Blog：http://blog.csdn.net/gaoxiangnumber1
 - E-mail：gaoxiangnumber1@126.com 欢迎您对我的项目/博客提出任何建议和批评。
 - 如果您觉得本项目不错或者对您有帮助，请赏颗星吧！谢谢您的阅读！
