source := AsyncLogging.o
source += Channel.o
source += CountDownLatch.o
source += Epoll.o
source += EventLoop.o
source += EventLoopThread.o
source += EventLoopThreadPool.o
source += FileUtil.o
source += HttpData.o
source += LogFile.o
source += Logging.o
source += LogStream.o
source += Server.o
source += Thread.o
source += Timer.o
source += Util.o
CC      := g++
LIBS    :=   -l server  -L . -l pthread
INCLUDE := -I./usr/local/lib
CFLAGS  := -std=c++17 -g -Wall -O3 -D_PTHREADS
CXXFLAGS := $(CFLAGS)

libserver.a : $(source)
	ar rcs $@ $^
%.o : %.cc
	g++ $< -c

clean:
	rm AsyncLogging.o
	rm Channel.o
	rm CountDownLatch.o
	rm Epoll.o
	rm EventLoop.o
	rm EventLoopThread.o
	rm EventLoopThreadPool.o
	rm FileUtil.o
	rm HttpData.o
	rm LogFile.o
	rm Logging.o
	rm LogStream.o
	rm Thread.o
	rm Server.o
	rm Timer.o
	rm Util.o
LoggingTest:
	$(CC) test/LoggingTest.cc -o $@ $(LIBS) $(CFLAGS)

Main:
	$(CC) Main.cc -o $@ $(LIBS) $(CFLAGS)