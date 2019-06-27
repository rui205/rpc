#include <pthread.h>
#include <string>

//#include "Util.h"
#include "TcpServerInterface.h"
#include "Channel.h"
#include "libevent/event2/listener.h"

namespace rpc {

typedef std::function<void(Channel*, void*)> callback_t;

class TcpServerImpl final: public TcpServerInterface {
public:
	TcpServerImpl(std::string addr, int thread_num);
	~TcpServerImpl();

public:
	void Start() override;
	void Stop() override;
	void SettingCallback(callback_t readcb, callback_t writecb, callback_t errorcb) override;																/*set your owner callback function*/
	void AutoIncrement();
	pthread_cond_t* GetThreadCondVar();
	pthread_mutex_t* GetThreadMutex();
	static void Notify(int fd, short events, void* arg);
	static void Monitor(evconnlistener*, int , struct sockaddr*, int , void*);
	static void Dispatch(int fd, int events, void* arg);
	static void* Routine(void* arg);

private:
	void ThreadInit();
	void ThreadStart(void* (*func)(void*), void* arg);
	void SetupThread(thread_t* thr);
	void WaitingForThreadRegistration();
	thread_t* GetThreadFromPools();
	Channel* CreateChannel(int fd, short events, struct event_base* base);
	static void BufferReadCallback(struct bufferevent* bev, void* arg);
	static void BufferWriteCallback(struct bufferevent* bev, void* arg);
	static void BufferErrorCallback(struct bufferevent* bev, short error_events, void* arg);

public:
	callback_t read_callback_;
	callback_t write_callback_;
	callback_t error_callback_;
	
private:
	std::string ip_;
	int port_;
	int counts_;
	int threads_;
	int last_index_;
	int init_thread_count_;
	struct evconnlistener* listener_;
	thread_t* master_;
	thread_t* thread_pool_;
	pthread_cond_t cond_var_;
	pthread_mutex_t mutex_;
};/*end TcpServerImpl*/

#if 0
class TcpServer: public noncopyable {
public:
	TcpServer(std::string addr, int thread_num);												/*thread number > 1*/
	virtual ~TcpServer();
	static void notify(int fd, short events, void* arg);											/*wakeup other thread*/
	static void* routine(void* arg);																/*worker thread routine*/
	static void monitor(evconnlistener*, int, struct sockaddr*, int, void*);						/*accept callback*/
	static void dispatch(int fd, int events, void* arg);										/*dispatch events*/
	void threadInitAutoIncrement();														/*auto add, when thread start*/
	void settingCallback(callback_t readcb, callback_t writecb, callback_t errorcb);    		
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

#endif
} /*end namespace rpc*/
