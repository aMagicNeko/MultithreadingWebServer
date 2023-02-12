## 模型
采用Reactor模式，由主线程负责连接的建立和任务的分发，子线程来完成具体的任务，采用one loop per thread设计，采用线程池限制线程数量和减少频繁创建销毁开销，采用epoll的ET模式，更高效。

## 线程模块
1. 通过EventLoopThreadPool限制线程数量和减少频繁创建销毁开销。
2. EventLoopThreadPool不是抢任务式的线程池，而是由主线程主动去给每个线程轮流放任务，因此可能会负载不均匀。
3. 主线程通过调用EventLoopThreadPool的start()接口创建并运行EventLoopThread，一个细节是，为了保存在子线程内创建的EventLoop指针在循环启动每个线程时会调用Condition的wait接口等待对应线程真正跑起来。

## HTTP模块
1. HttpData对象封装了输入和输出缓冲区、连接的状态、处理的状态、是否错误、Http方法、以及其他属性如keep_alive
2. 在连接到来时由主线程创建HttpData，然后交由其他线程处理，如果为keep_alive，则会添加对应定时器
## 定时器模块
1. 采用最小堆，直接使用stl中的priority_queue实现
2. 每个线程只有一个TimerManager，保存在其EventLoop对象中
3. 在EventLoop的loop()中，当从poll()中唤醒，会去从定器中不断弹出过期事件然后处理。
## EventLoop模块
1. Channel封装了描述符、监听事件、返回事件和其四种回调函数(connect, read, write, error)以及其HTTP对象的指针、EventLoop的指针
2. Epoll封装了Epoll表、添加的fd对应的Chanel，调用poll()并得到返回后会将返回事件返回给其Chanel.Epoll还包含一个TimeManager对象管理定时器.
3. EventLoop封装了事件循环，包含了Epoll对象指针、用来wakeup的channel(其fd调用eventfd创建)，当其他线程需要向该线程中添加函数执行时，调用runInLoop()接口，这个接口将向wakeupfd中写使得循环被唤醒，loop中被唤醒后先处理事件，再来执行保存在待执行函数数组中的函数。

## Log模块
采用多缓冲的形式，不必每一次其他线程写日志就唤醒日志线程。多生产者单消费者模型，消费者占用较小资源且是异步日志。
相关的类有LogStream, FixedBuffer, Logger, AsyncLogging, LogFile
1. LogFile封装了底层的文件操作
2. FixedBuffer封装了固定大小的缓存区
3. LogStream用来格式化字符串并输入到其缓冲区，每个对象配备一个唯一的小的FixedBuffer
每一个Logger对象配备了LogStream，程序通过调用Logger对象来进行日志记录，在Logger对象析构时才会把其LogStream流中的缓存交给日志线程
4. AsyncLogging封装了日志线程，在第一个Logger对象析构时，通过pthread_once确保启动。AsyncLogging中包含了一个（大的FixedBuffer）的数组，这个数组存储等待往硬盘中输出的内容。
除此之外，AsyncLogging维护另外两个大的FixedBuffer，其中一个用来接收从Logger对象处来的数据，另外一个是一个备用的，当第一个满了，就用第二个，第二个若没有，就new一个，当等待数组中的某个buffer已经输出完毕，可以用来补充为备用，或者直接删除。
5. 在线程数量较少且高并发的情况，由于每个Logger析构时要获取AsyncLogging中的锁，竞争会很激烈，可以采用给每个线程一个大的FixedBuffer，然后在其满时被添加到AsyncLogging的输出数组，也即每个线程双缓存。

## Server启动
1. 设置忽略SIGPIPE，防止进程在向客户端已调用close()的连接写数据时退出。
2. 建立监听Channel对象
3. 启动线程池
4. 启动主Loop

## 同步
1. 线程池中创建线程时为了保证子线程创建的loop_在主线程中使用时已经创建，使用Condition
2. 主线程向子线程中添加待执行函数或者添加Channel对象时获取锁
3. 由于每一个HttpData对象或者Channel对象在交付给子线程后完全由子线程处理，其生命周期也由shared_ptr管理，所以不需要同步操作
4. 日志模块中Logger对象析构时获取AsyncLogging中的锁来输入到其缓冲区中，AsyncLogging更换空缓冲区时也获取锁
