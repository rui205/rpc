#include <pthread.h>
#include <string>

//#include "Util.h"
#include "Channel.h"
#include "libevent/event2/listener.h"

namespace rpc {

typedef std::function<void(Channel*, void*)> callback_t;

class TcpServer: public noncopyable {
public:
	TcpServer(std::string addr, int thread_num);												/*thread number > 1*/
	virtual ~TcpServer();
	static void notify(int fd, short events, void* arg);											/*wakeup other thread*/
	static void* routine(void* arg);																/*worker thread routine*/
	static void monitor(evconnlistener*, int, struct sockaddr*, int, void*);						/*accept callback*/
	static void dispatch(int fd, int events, void* arg);										/*dispatch events*/
	void threadInitAutoIncrement();														/*auto add, when thread start*/
	void settingCallback(callback_t readcb, callback_t writecb, callback_t errorcb);    		/*set your owner callback function*/
	pthread_cond_t* getThreadCondition();													/*get master thread condition*/
	pthread_mutex_t* getThreadMutex();														/*get master thread mutex*/
	
	void start();																					/*start the tcp server*/

private:
	void threadInit();																			/*init thread info*/
	void threadStart(void* (*func)(void*), void* arg);											/*create the worker thread*/
	void setupThread(thread_t* thr);
	void waitingForThreadRegistration();
	thread_t* getThreadFromPools();
	Channel* createChannel(int fd, int events, struct event_base* base);						/*create the channel for events read or write*/
    static void bufferReadCallback(struct bufferevent* bev, void* arg);
    static void bufferWriteCallback(struct bufferevent* bev, void* arg);
    static void bufferErrorCallback(struct bufferevent* bev, short events, void* arg);


private:
	std::string ip_;																				/*server ip address*/	
	int port_;																						/*listen port*/
	int counts_;																					/*thread init counts*/
	int threads_;																					/*the thread number*/
	int lastIndex_; 																				/*thread last index*/
	int initThreadCount_;																			/*the number of thread registration*/
	struct evconnlistener* listener_;																/*server monitor*/
	thread_t* master_;																				/*master thread info*/
	thread_t* threadPool_;																			/*server thread pool*/
	pthread_cond_t condition_;
	pthread_mutex_t thrMutex_;

	
public:
	/*void (*func)(Channel* void*)*/
	callback_t readCallback_;
	callback_t writeCallback_;
	callback_t errorCallback_;
	
};/*end TcpServer*/

} /*end namespace rpc*/
