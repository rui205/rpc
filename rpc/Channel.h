#ifndef __CHANNEL_H__
#define __CHANNEL_H__

//#include "Util.h"
#include "TaskQueue.h"

#include "libevent/event.h"

namespace rpc {

class TcpServer;

typedef struct thread {
	pthread_t tid_;									/*thread id*/
	struct event_base* base_;		
	struct event notify_event_;		
	int notify_recv_fd_;			
	int notify_send_fd_;			
	TaskQueue* task_queue_; 						/*task queue*/
													/*dao*/
	union {
		TcpServer* server_;
	};
} thread_t;

class Channel: public noncopyable {
public:
    Channel(int fd);
    ~Channel();

public:
    thread_t* getChannelThread();
    void setChannelThread(thread_t* thr);
    int getChannelFd();
	void setBufferevent(struct bufferevent* bev);
	struct bufferevent* getBufferevent();
    int getReadBufferLength();
    int getWriteBufferLength();
    int readToBuffer(char* buffer, int length);
    int copyToBuffer(char* buffer, int length);
    int appendToBuffer(char* buffer, int length);
    int appendBufferToBuffer(struct evbuffer* buffer);
    void moveBufferReadToWrite();

private:
    int fd_;
    thread_t* thr_;
    struct bufferevent* bev_;
};

}/*end namespace rpc*/

#endif
