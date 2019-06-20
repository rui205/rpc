.SUFFIXES:.cpp .o

.PHONY:clean
.PHONY:install

ROOTDIR = ../..
#CFLAGS += -I $(ROOTDIR)/Framework/3rd/
#CFLAGS += -I ../../Framework/3rd/jsoncpp/
#CFLGAS += -I ../jsoncpp/
#CFLAGS += -I $(ROOTDIR)/Framework/3rd/jsoncpp/
#CFLAGS += -I $(ROOTDIR)/inc/hiredis/
#CFLAGS += -I $(ROOTDIR)/Framework/3rd/gflags/
#CFLAGS += -I $(ROOTDIR)/Framework/3rd/easylogging/
#CFLAGS += -I $(ROOTDIR)/Framework/3rd/libevent/
#CFLAGS += -I $(ROOTDIR)/Framework/3rd/jsoncpp/
CFLAGS += -Wall -g -Wno-unused-function -O3
CFLAGS += -std=c++11 -rdynamic
#-DELPP_THREAD_SAFE 
#-rdynamic

LDFLAGS += -Wl,-rpath,$(ROOTDIR)/Target/ 

#LDFLAGS += -L $(ROOTDIR)/Target/jsoncpp/

#LDFLAGS += -L $(ROOTDIR)/lib/hiredis

#LDFLAGS += -L $(ROOTDIR)/Target/gflags/

#LDFLAGS += -L $(ROOTDIR)/Target/libevent/

#LDFLAGS += -L $(ROOTDIR)/Target/glog/

#LDFLAGS += -lssl -lcrypto -lrt -lpthread -lgflags 

#LIBS += $(ROOTDIR)/Target/jsoncpp/libjsoncpp.a

#LIBS += $(ROOTDIR)/Target/hiredis/libhiredis.a

#LIBS += $(ROOTDIR)/Target/libevent/libevent.a

#LIBS += $(ROOTDIR)/Target/libevent/libevent_pthreads.a

#LIBS += $(ROOTDIR)/Target/gflags/libgflags.a

#LIBS += $(ROOTDIR)/Target/glog/libglog.a

CC = g++

AR = ar

SRCS = ../TaskQueue.cpp TaskQueueTest.cpp

OBJS = $(SRCS:.cpp=.o) 

#CLIB = server.so

EXEC = task_queue_test

$(EXEC):$(OBJS)

	$(CC) -o $@ $^ $(LIBS) $(LDFLAGS)
	
%.o:%.cpp

	$(CC) $(CFLAGS) -o $@ -c $< 
	
clean:

	rm -rf *.o core.* log-* $(EXEC)
