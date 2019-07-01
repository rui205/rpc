#include <pthread.h>
#include <string>

//#include "Util.h"
#include "TcpServerInterface.h"
#include "Channel.h"
#include "libevent/event2/listener.h"

namespace rpc {

typedef std::function<void(Channel*, void*)> callback_t;

class TcpServerImpl: public TcpServerInterface {
public:
	explicit TcpServerImpl(std::string addr, int thread_num);
	~TcpServerImpl();

public:
	void Start() override;
	void Stop() override;
	virtual void Read(Channel* chan, void* arg);
	virtual void Write(Channel* chan, void* arg);
	virtual void Error(Channel* chan, void* arg);
	void AutoIncrement();
	pthread_cond_t* GetThreadCondVar();
	pthread_mutex_t* GetThreadMutex();

private:
	void SettingCallback(callback_t readcb, callback_t writecb, callback_t errorcb) override;																/*set your owner callback function*/
	void ThreadInit();
	void ThreadStart(void* (*func)(void*), void* arg);
	void SetupThread(thread_t* thr);
	void WaitingForThreadRegistration();
	thread_t* GetThreadFromPools();
	Channel* CreateChannel(int fd, short events, struct event_base* base);
	static void Notify(int fd, short events, void* arg);
	static void Monitor(evconnlistener*, int , struct sockaddr*, int , void*);
	static void Dispatch(int fd, int events, void* arg);
	static void* Routine(void* arg);
	static void BufferReadCallback(struct bufferevent* bev, void* arg);
	static void BufferWriteCallback(struct bufferevent* bev, void* arg);
	static void BufferErrorCallback(struct bufferevent* bev, short events, void* arg);

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

} /*end namespace rpc*/
