#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "TcpServer.h"

#include "glog/logging.h"

namespace rpc {

#if 0
TcpServer::TcpServer(std::string addr, int thread_num) {
	if (addr.length() <= 0 || thread_num <= 1) {
		exit(-1);
	}

	/*split the addr host:port*/
	std::string c = ":";
	std::list<std::string> res;	
	split(addr, res, c);
	ip_ = res.front();
	res.pop_front();
	port_ = std::stoi(res.front());
	res.pop_front();

	LOG(INFO) << "ip_: " << ip_ << "  port_: " << port_;
	
	/*init listener*/
	listener_ = NULL;

	/*init thread number*/
	threads_ = thread_num;

	/*init thread pool*/
	threadPool_ = new thread_t[threads_];
	
	/*init master thread*/
	master_ = new(std::nothrow) thread_t;
	master_->tid_ = pthread_self();
    struct event_config* config = event_config_new();
    event_config_set_flag(config, EVENT_BASE_FLAG_NOLOCK);
    event_config_set_flag(config, EVENT_BASE_FLAG_EPOLL_USE_CHANGELIST);
	master_->base_ = event_base_new_with_config(config);
	event_safe_free(config, event_config_free);

	/**/
	counts_ = 0;
	lastIndex_ = -1;
	initThreadCount_ = 0;
}

TcpServer::~TcpServer() {
	if (listener_ != NULL) {
    	event_safe_free(listener_, evconnlistener_free);
	}

	if (master_->base_ != NULL) {
		event_safe_free(master_->base_, event_base_free);
	}

	for (int i = 0; i < threads_; ++ i) {
		if (threadPool_[i].base_ != NULL) {
			event_safe_free(threadPool_[i].base_, event_base_free);
		}
	}

	threads_ = 0;

	delete master_;
	delete threadPool_;

	master_ = threadPool_ = NULL;
}

void TcpServer::waitingForThreadRegistration() {
	while (initThreadCount_ < threads_) {
		pthread_cond_wait(&condition_, &thrMutex_);
	}
}

void TcpServer::threadInitAutoIncrement() {
	++ initThreadCount_;
}

pthread_cond_t* TcpServer::getThreadCondition() {
	return &condition_;
}

pthread_mutex_t* TcpServer::getThreadMutex() {
	return &thrMutex_;
}

void TcpServer::threadInit() {
	pthread_cond_init(&condition_, NULL);
	pthread_mutex_init(&thrMutex_, NULL);

	for (int i = 0; i < threads_; ++ i) {
		int fds[2] = { -1, -1 };
		if (pipe(fds) < 0) {
			LOG(ERROR) << "pipe init failed: " << strerror(errno);
			exit(-1);	
		}

		threadPool_[i].notify_recv_fd_ = fds[0];
		threadPool_[i].notify_send_fd_ = fds[1];

		setupThread(&threadPool_[i]);
	}

	LOG(INFO) << "setupThread all";
	
	for (int i = 0; i < threads_; ++ i) {
		threadStart(routine, &threadPool_[i]);
	}

	LOG(INFO) << "threadStart all";

	pthread_mutex_lock(&thrMutex_);
	waitingForThreadRegistration();
	pthread_mutex_unlock(&thrMutex_);
}

/*
void TcpServer::setupThread(thread_t* thr) {
	if (thr == NULL) {
		LOG(ERROR) << "setupThread met error: thread is nil";
		return;
	}

	struct event_config* config = event_config_new();
	if (config == NULL) {
		LOG(ERROR) << "event_config_new failed";
		return;
	}

//	set the event_base no lock and used changelist
	event_config_set_flag(config, EVENT_BASE_FLAG_NOLOCK);
	event_config_set_flag(config, EVENT_BASE_FLAG_EPOLL_USE_CHANGELIST);

	thr->base_ = event_base_new_with_config(config);
	thr->server_ = this;

	event_safe_free(config, event_config_free);
	
	if (thr->base_ == NULL) {
		LOG(ERROR) << "setupThread new base failed";
		exit(-1);
	}

//	setting persist events
	int flags = EV_READ | EV_PERSIST;
	event_set(&thr->notify_event_, thr->notify_recv_fd_, flags, notify, thr);
	event_base_set(thr->base_, &thr->notify_event_);

	if (event_add(&thr->notify_event_, NULL) == -1) {
		LOG(ERROR) << "threadInit event_add failed";
		exit(-1);
	}

	thr->task_queue_ = new(std::nothrow) TaskQueue();
	thr->task_queue_->init();

}
*/

/*worker thread routine*/
void* TcpServer::routine(void* arg) {
	thread_t* thr = static_cast<thread_t*>(arg);
	if (thr == NULL) {
		pthread_exit(NULL);
	}
	
	pthread_mutex_lock(thr->server_->getThreadMutex());
	LOG(INFO) << "start thread id: " << thr->tid_;
	thr->server_->threadInitAutoIncrement();
	pthread_cond_signal(thr->server_->getThreadCondition());
	pthread_mutex_unlock(thr->server_->getThreadMutex());

	event_base_dispatch(thr->base_);
	event_safe_free(thr->base_, event_base_free);

	pthread_exit(NULL);
}

void TcpServer::threadStart(void* (*func)(void*), void* arg) {
	thread_t* thr = static_cast<thread_t*>(arg);
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	/*set your expectations thread attr*/
	// ...
	// ...
	if (pthread_create(&thr->tid_, &attr, func, arg) != 0) {
		LOG(ERROR) << "threadStart met error: " << strerror(errno);
		exit(-1);
	}
}

void TcpServer::bufferReadCallback(struct bufferevent* bev, void* arg) {
	Channel* chan = static_cast<Channel*>(arg);
	if (chan == NULL) {
		return;
	}

	LOG(INFO) << "internal bufferevent read callback";
	
	thread_t* thr = chan->getChannelThread();
	if (thr->server_->readCallback_) {
		thr->server_->readCallback_(chan, NULL);
	}
}

void TcpServer::bufferWriteCallback(struct bufferevent* bev, void* arg) {
	Channel* chan = static_cast<Channel*>(arg);
	if (chan == NULL) {
		return;
	}

	LOG(INFO) << "internal bufferevent write callback";

	thread_t* thr = chan->getChannelThread();
	if (thr->server_->writeCallback_) {
		thr->server_->writeCallback_(chan, NULL);
	}
}

void TcpServer::bufferErrorCallback(struct bufferevent* bev, short events, void* arg) {
	Channel* chan = static_cast<Channel*>(arg);
	if (chan == NULL) {
		return;
	}

	LOG(INFO) << "internal bufferevent error callback";

	
    if (events & BEV_EVENT_EOF) {
        LOG(INFO) << "peer closed";
    } else if (events & BEV_EVENT_TIMEOUT) {
        LOG(INFO) << "channel timeout";
    } else if (events & BEV_EVENT_ERROR) {
        LOG(INFO) << "unrecoverable error";
    } else {
        LOG(INFO) << "unknow error";
    }

	thread_t* thr = chan->getChannelThread();
	if (thr->server_->errorCallback_) {
		thr->server_->errorCallback_(chan, NULL);
	}

	delete chan;
	chan = NULL;
/*	
	struct bufferevent* tmp = chan->getBufferevent();
	if (tmp != NULL) {
		event_safe_free(tmp, bufferevent_free);
	}
*/	
}

void TcpServer::settingCallback(callback_t readcb, callback_t writecb, callback_t errorcb) {
	if (readcb == NULL && writecb == NULL && errorcb == NULL) {
		LOG(INFO) << "please setting server callback function";
		return;
	}

	readCallback_ = readcb;
	writeCallback_ = writecb;
	errorCallback_ = errorcb;
}

Channel* TcpServer::createChannel(int fd, int events, struct event_base* base) {
	if (fd < 0 || base == NULL) {
		return NULL;
	}

	Channel* chan = new(std::nothrow) Channel(fd);
	if (chan == NULL) {
		LOG(ERROR) << "new channel met error: " << strerror(errno);
		return NULL;
	}

	struct bufferevent* tmp = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
	if (tmp == NULL) {
		delete chan;
		chan = NULL;
		return NULL;
	}	

	bufferevent_set_max_single_read(tmp, 128 * 1024);
	bufferevent_setcb(tmp, bufferReadCallback, bufferWriteCallback, bufferErrorCallback, (void*)chan);

	/*the channel timeout*/
	struct timeval tv = { 4*3600, 0 };
	bufferevent_set_timeouts(tmp, &tv, NULL);

    chan->setBufferevent(tmp);

	bufferevent_enable(tmp, EV_READ);
	
	return chan;
}

void TcpServer::notify(int fd, short events, void* arg) {
	if (arg == NULL) {
		LOG(ERROR) << "notify arg is nil";
		return;
	}

	thread_t* thr = static_cast<thread_t*>(arg);
	char buf[1];

	int rc = read(fd, buf, sizeof(buf));
	if (rc != 1) {
		/*can not close fd*/
		LOG(ERROR) << "notify read met error: " << strerror(errno);
		return;
	}

	LOG(INFO) << "recv notify rc: " << rc;
	
	task_item_t* item = NULL;
	Channel* chan = NULL;
	
	switch (buf[0]) {
		case 'c':
			item = thr->task_queue_->pop();
			if (item == NULL) {
				LOG(ERROR) << "notify task_queue_ pop nil";
				break;
			}
		
			chan = thr->server_->createChannel(item->fd_, item->event_flags_, thr->base_);
			if (chan == NULL) {
				LOG(ERROR) << "create channel failed";
				break;
			}	
	}

	/*if create channel failed, free item and close fd*/
	if (chan == NULL) {
		close(item->fd_);
	}

	thr->task_queue_->free(item);

	/*set the channel owner thread*/
	if (chan != NULL) {
		chan->setChannelThread(thr);
	}
}

thread_t* TcpServer::getThreadFromPools() {
	++ lastIndex_;
	lastIndex_ = lastIndex_ % threads_;
	return &threadPool_[lastIndex_];
}

void TcpServer::dispatch(int fd, int events, void* arg) {
	TcpServer* server = static_cast<TcpServer*>(arg);

	/*get thread from thread pools*/
	thread_t* thr = server->getThreadFromPools();
	if (thr == NULL) {
		close(fd);
		return;
	}

	/*set socket buffer length*/
	int bufferLength = 128 * 1024; 
	setsockopt(fd, SOL_SOCKET, SO_RCVBUF, (const char*)&bufferLength, sizeof(int)); 

	/*get a free item from task queue freelist*/
	task_item_t* item = thr->task_queue_->getFromFreelist(); 
	if (item == NULL) {
		close(fd);
		return;
	}

	item->fd_ = fd;
	item->event_flags_ = events;
	item->next_ = NULL;

	/*push*/
	thr->task_queue_->push(item);

	/*notify the corresponding thread*/
	char buf[1] = { 'c' };
	int rc = write(thr->notify_send_fd_, buf, sizeof(buf));
	if (rc != sizeof(buf)) {
		/*do nothing and wait the next success write*/
		LOG(ERROR) << "dispatch write met error: " << strerror(errno);
		return;
	}	

	LOG(INFO) << "dispatch succ OK";
}

void TcpServer::monitor(evconnlistener* listener, 
		int fd, struct sockaddr* addr, int len, void* arg) {
	if (fd < 0 || arg == NULL) {
		return;
	}	

	dispatch(fd, EV_READ | EV_PERSIST, arg);
}
/*
void TcpServer::start() {
	
	threadInit();

	struct sockaddr_in baddr;
	memset(&baddr, 0, sizeof(baddr));
	baddr.sin_family = AF_INET;
	baddr.sin_port = htons(9900);
	baddr.sin_addr.s_addr = inet_addr("0.0.0.0");

	if (master_->base_ == NULL) {
		LOG(ERROR) << "the master event_base is nil";
		return;
	}

	listener_ = evconnlistener_new_bind(master_->base_, monitor, (void*)this, 
						LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE_PORT, 
							65535, (struct sockaddr*)&baddr, sizeof(baddr));
	
	if (listener_ == NULL) {
		LOG(ERROR) << "start listener_ is nil";
		exit(-1);
	}
	
    event_base_dispatch(master_->base_);
}
*/

#endif

TcpServerImpl::TcpServerImpl(std::string addr, int thread_num) {
	if (addr.length() <= 0 || thread_num <= 1) {
		exit(-1);
	}

	/*split the addr host:port*/
	std::string c = ":";
	std::list<std::string> res;	
	split(addr, res, c);
	ip_ = res.front();
	res.pop_front();
	port_ = std::stoi(res.front());
	res.pop_front();

	LOG(INFO) << "ip_: " << ip_ << "  port_: " << port_;
	
	/*init listener*/
	listener_ = NULL;

	/*init thread number*/
	threads_ = thread_num;

	/*init thread pool*/
	thread_pool_ = new thread_t[threads_];
	
	/*init master thread*/
	master_ = new(std::nothrow) thread_t;
	master_->tid_ = pthread_self();
    struct event_config* config = event_config_new();
    event_config_set_flag(config, EVENT_BASE_FLAG_NOLOCK);
    event_config_set_flag(config, EVENT_BASE_FLAG_EPOLL_USE_CHANGELIST);
	master_->base_ = event_base_new_with_config(config);
	event_safe_free(config, event_config_free);

	/**/
	counts_ = 0;
	last_index_ = -1;
	init_thread_count_ = 0;
}

TcpServerImpl::~TcpServerImpl() {
    if (listener_ != NULL) {
        event_safe_free(listener_, evconnlistener_free);
    }

    if (master_->base_ != NULL) {
        event_safe_free(master_->base_, event_base_free);
    }

    for (int i = 0; i < threads_; ++ i) {
        if (thread_pool_[i].base_ != NULL) {
            event_safe_free(thread_pool_[i].base_, event_base_free);
        }
    }

    threads_ = 0;

    delete master_;
    delete thread_pool_;

    master_ = thread_pool_ = NULL;
}

void TcpServerImpl::Start() {
	/*init the thead info*/
	ThreadInit();
	
	/*create listener and bind addr*/
	struct sockaddr_in baddr;
	memset(&baddr, 0, sizeof(baddr));
	baddr.sin_family = AF_INET;
	baddr.sin_port = htons(9900);
	baddr.sin_addr.s_addr = inet_addr("0.0.0.0");

	if (master_->base_ == NULL) {
		LOG(ERROR) << "the master event_base is nil";
		return;
	}
	listener_ = evconnlistener_new_bind(master_->base_, Monitor, (void*)this, 
						LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE_PORT, 
							65535, (struct sockaddr*)&baddr, sizeof(baddr));
	
	if (listener_ == NULL) {
		LOG(ERROR) << "start listener_ is nil";
		exit(-1);
	}
	
    event_base_dispatch(master_->base_);
}

void TcpServerImpl::Stop() {

}

void TcpServerImpl::Read(Channel* chan, void* arg) {
	// NOTHING TODO
}

void TcpServerImpl::Write(Channel* chan, void* arg) {
	// NOTHING TODO
}

void TcpServerImpl::Error(Channel* chan, void* arg) {
	// NOTHING TODO
}

void TcpServerImpl::SettingCallback(callback_t readcb, callback_t writecb, callback_t errorcb) {
	if (readcb == NULL && writecb == NULL && errorcb == NULL) {
		return;
	}

	read_callback_ = readcb;
	write_callback_ = writecb;
	error_callback_ = errorcb;
}

void TcpServerImpl::AutoIncrement() {
	++ init_thread_count_;
}

void TcpServerImpl::Notify(int fd, short events, void* arg) {
    thread_t* thr = static_cast<thread_t*>(arg);
    char buf[1];

    int rc = ::read(fd, buf, sizeof(buf));
    if (rc != 1) {
        /*can not close fd*/
        LOG(ERROR) << "notify read met error: " << strerror(errno);
        return;
    }

    LOG(INFO) << "recv notify rc: " << rc;
    
    task_item_t* item = NULL;
    Channel* chan = NULL;
    
    switch (buf[0]) {
        case 'c':
            item = thr->task_queue_->pop();
            if (item == NULL) {
                LOG(ERROR) << "notify task_queue_ pop nil";
                break;
            }
        
            chan = thr->server_->CreateChannel(item->fd_, item->event_flags_, thr->base_);
            if (chan == NULL) {
                LOG(ERROR) << "create channel failed";
                break;
            }   
    }

    /*if create channel failed, free item and close fd*/
    if (chan == NULL) {
        close(item->fd_);
    }

    thr->task_queue_->free(item);

    /*set the channel owner thread*/
    if (chan != NULL) {
        chan->setChannelThread(thr);
    }
}

void TcpServerImpl::Monitor(evconnlistener* listener, int fd, struct sockaddr* remote_addr, int  events, void* arg) {
   if (fd < 0 || arg == NULL) {
		LOG(ERROR) << "Monitor recv fd or arg is nil";
        return;
    }

    Dispatch(fd, EV_READ | EV_PERSIST, arg);
}

void TcpServerImpl::Dispatch(int fd, int events, void* arg) {
    TcpServerImpl* server = static_cast<TcpServerImpl*>(arg);

    /*get thread from thread pools*/
    thread_t* thr = server->GetThreadFromPools();
    if (thr == NULL) {
        close(fd);
        return;
    }

    /*set socket buffer length*/
	int buffer_length = 128 * 1024;
    setsockopt(fd, SOL_SOCKET, SO_RCVBUF, (const char*)&buffer_length, sizeof(int));

    /*get a free item from task queue freelist*/
    task_item_t* item = thr->task_queue_->getFromFreelist();
    if (item == NULL) {
        close(fd);
        return;
    }

    item->fd_ = fd;
    item->event_flags_ = events;
    item->next_ = NULL;

    /*push*/
    thr->task_queue_->push(item);

    /*notify the corresponding thread*/
    char buf[1] = { 'c' };
    int rc = ::write(thr->notify_send_fd_, buf, sizeof(buf));
    if (rc != sizeof(buf)) {
        /*do nothing and wait the next success write*/
        LOG(ERROR) << "dispatch write met error: " << strerror(errno);
        return;
    }

    LOG(INFO) << "dispatch succ OK";
}

void* TcpServerImpl::Routine(void* arg) {
    thread_t* thr = static_cast<thread_t*>(arg);
    if (thr == NULL) {
        pthread_exit(NULL);
    }

    pthread_mutex_lock(thr->server_->GetThreadMutex());
    LOG(INFO) << "start thread id: " << thr->tid_;
    thr->server_->AutoIncrement();
    pthread_cond_signal(thr->server_->GetThreadCondVar());
    pthread_mutex_unlock(thr->server_->GetThreadMutex());

    event_base_dispatch(thr->base_);
    event_safe_free(thr->base_, event_base_free);

    pthread_exit(NULL);
}

pthread_cond_t* TcpServerImpl::GetThreadCondVar() {
	return &cond_var_;
}

pthread_mutex_t* TcpServerImpl::GetThreadMutex() {
	return &mutex_;
}

void TcpServerImpl::ThreadInit() {
	pthread_cond_init(&cond_var_, NULL);
	pthread_mutex_init(&mutex_, NULL);

	for (int i = 0; i < threads_; ++ i) {
		int fds[2] = { -1, -1 };
		if (pipe(fds) < 0) {
			LOG(ERROR) << "pipe init failed: " << strerror(errno);
			exit(-1);	
		}

		thread_pool_[i].notify_recv_fd_ = fds[0];
		thread_pool_[i].notify_send_fd_ = fds[1];

		SetupThread(&thread_pool_[i]);
	}

	LOG(INFO) << "setupThread all";
	
	for (int i = 0; i < threads_; ++ i) {
		ThreadStart(Routine, &thread_pool_[i]);
	}

	LOG(INFO) << "threadStart all";

	pthread_mutex_lock(&mutex_);
	WaitingForThreadRegistration();
	pthread_mutex_unlock(&mutex_);
}

void TcpServerImpl::ThreadStart(void* (*func)(void*), void* arg) {
    thread_t* thr = static_cast<thread_t*>(arg);
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    /*set your expectations thread attr*/
    // ...
    // ...
    if (pthread_create(&thr->tid_, &attr, func, arg) != 0) {
        LOG(ERROR) << "threadStart met error: " << strerror(errno);
        exit(-1);
    }
}

void TcpServerImpl::SetupThread(thread_t* thr) {
    if (thr == NULL) {
        LOG(ERROR) << "SetupThread met error: thread is nil";
        return;
    }

    struct event_config* config = event_config_new();
    if (config == NULL) {
        LOG(ERROR) << "event_config_new failed";
        return;
    }

    /*set the event_base no lock and used changelist*/
    event_config_set_flag(config, EVENT_BASE_FLAG_NOLOCK);
    event_config_set_flag(config, EVENT_BASE_FLAG_EPOLL_USE_CHANGELIST);

    thr->base_ = event_base_new_with_config(config);
    thr->server_ = this;

    event_safe_free(config, event_config_free);

    if (thr->base_ == NULL) {
        LOG(ERROR) << "SetupThread new base failed";
        exit(-1);
    }

    /*setting persist events*/
    int flags = EV_READ | EV_PERSIST;
    event_set(&thr->notify_event_, thr->notify_recv_fd_, flags, Notify, thr);
    event_base_set(thr->base_, &thr->notify_event_);

    if (event_add(&thr->notify_event_, NULL) == -1) {
        LOG(ERROR) << "threadInit event_add failed";
        exit(-1);
    }

    thr->task_queue_ = new(std::nothrow) TaskQueue();
    thr->task_queue_->init();

    /*dao*/
}

void TcpServerImpl::WaitingForThreadRegistration() {
    while (init_thread_count_ < threads_) {
        pthread_cond_wait(&cond_var_, &mutex_);
    }
}

thread_t* TcpServerImpl::GetThreadFromPools() {
    ++ last_index_;
    last_index_ = last_index_ % threads_;
    return &thread_pool_[last_index_];
}

Channel* TcpServerImpl::CreateChannel(int fd, short events, struct event_base* base) {
    if (fd < 0 || base == NULL) {
        return NULL;
    }

    Channel* chan = new(std::nothrow) Channel(fd);
    if (chan == NULL) {
        LOG(ERROR) << "new channel met error: " << strerror(errno);
        return NULL;
    }

    struct bufferevent* tmp = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
    if (tmp == NULL) {
        delete chan;
        chan = NULL;
        return NULL;
    }   

    bufferevent_set_max_single_read(tmp, 128 * 1024);
    bufferevent_setcb(tmp, BufferReadCallback, BufferWriteCallback, BufferErrorCallback, (void*)chan);

    /*the channel timeout*/
    struct timeval tv = { 10, 0 };
    bufferevent_set_timeouts(tmp, &tv, NULL);

    chan->setBufferevent(tmp);

    bufferevent_enable(tmp, EV_READ);
    
    return chan;

}

void TcpServerImpl::BufferReadCallback(struct bufferevent* bev, void* arg) {
    Channel* chan = static_cast<Channel*>(arg);
    if (chan == NULL) {
        return;
    }

    LOG(INFO) << "internal bufferevent read callback";

	//read(chan, NULL);
	
    thread_t* thr = chan->getChannelThread();
	thr->server_->Read(chan, NULL);
/*
	if (thr->server_->read_callback_) {
        thr->server_->read_callback_(chan, NULL);
    }
    */
}

void TcpServerImpl::BufferWriteCallback(struct bufferevent* bev, void* arg) {
	Channel* chan = static_cast<Channel*>(arg);
	if (chan == NULL) {
		return;
	}

	LOG(INFO) << "internal bufferevent write callback";

	thread_t* thr = chan->getChannelThread();
	thr->server_->Write(chan, NULL);
/*	if (thr->server_->write_callback_) {
		thr->server_->write_callback_(chan, NULL);
	}
*/
}

void TcpServerImpl::BufferErrorCallback(struct bufferevent* bev, short events, void* arg) {
	Channel* chan = static_cast<Channel*>(arg);
	if (chan == NULL) {
		return;
	}

	LOG(INFO) << "internal bufferevent error callback";


	if (events & BEV_EVENT_EOF) {
		LOG(INFO) << "peer closed";
	} else if (events & BEV_EVENT_TIMEOUT) {
		LOG(INFO) << "channel timeout";
	} else if (events & BEV_EVENT_ERROR) {
		LOG(INFO) << "unrecoverable error";
	} else {
		LOG(INFO) << "unknow error";
	}

	thread_t* thr = chan->getChannelThread();
	thr->server_->Error(chan, NULL);
/*	
	if (thr->server_->error_callback_) {
		thr->server_->error_callback_(chan, NULL);
	}
*/
	delete chan;
	chan = NULL;
	/*	
	struct bufferevent* tmp = chan->getBufferevent();
	if (tmp != NULL) {
		event_safe_free(tmp, bufferevent_free);
	}
	*/	
}

}/*end namespace rpc*/
