#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "TcpClient.h"

#include "glog/logging.h"

namespace rpc{ 

TcpClient::TcpClient(std::string addr, int port) {
	if (addr.length() <= 0 || port <= 0) {
		LOG(ERROR) << "TcpClient structure failed";
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

	thr_->tid_ = pthread_self();
	thr_->base_ = event_base_new();
	thr_->notify_recv_fd_ = -1;
	thr_->notify_send_fd_ = -1;
	thr_->task_queue_ = NULL;
}

TcpClient::~TcpClient() {
	if (thr_->base_) {
		event_safe_free(thr_->base_, event_base_free);
	}

	port_ = -1;
	ip_ = "";
}

void TcpClient::SettingCallback(callback_t read_func, callback_t write_func, callback_t error_func) {
	if (!read_func && !write_func && !error_func) {
		LOG(WARNING) << "all the callback is nil, please check";
		return;
	}

	read_func_ = read_func;
	write_func_ = write_func;
	error_func_ = error_func;
}

void TcpClient::Start() {
	if (thr_->base_ == NULL) {
		LOG(ERROR) << "event base new failed";
		return;
	}

	struct sockaddr_in peeraddr;
	peeraddr.sin_family = AF_INET;
	peeraddr.sin_port = htons(port_);
	peeraddr.sin_addr.s_addr = inet_addr(ip_.c_str());
	
	channel = new(std::nothrow) Channel(-1);
	if (channel == NULL) {
		LOG(ERROR) << "allocate channel failed";
		return;
	}

	channel->setChannelThread(thr_);
	struct bufferevent* bev = bufferevent_socket_new(thr_->base_, -1, BEV_OPT_CLOSE_ON_FREE);
	if (bev == NULL) {
		return;
	}

	bufferevent_setcb(bev, BufferReadCallback, BufferWriteCallback, BufferErrorCallback, channel);
	
	if (bufferevent_socket_connect(bev, (struct sockaddr*)&peeraddr, sizeof(peeraddr)) < 0) {
		LOG(ERROR) << "bufferevent_socket_connect failed: " << evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR());
		return;
	}

	channel->setBufferevent(bev);

	bufferevent_enable(bev, EV_READ | EV_WRITE);
}

void TcpClient::BufferReadCallback(struct bufferevent* bev, void* arg) {
	Channel* channel = static_cast<Channel*>(arg);
	thread_t* thr = channel->getChannelThread();
	if (thr->client_->read_func_) {
		thr->client_->read_func_(channel, NULL);
	}
}

void TcpClient::BufferWriteCallback(struct bufferevent* bev, void* arg) {
	Channel* channel = static_cast<Channel*>(arg);
	thread_t* thr = channel->getChannelThread();
	if (thr->client_->write_func_) {
		thr->client_->write_func_(channel, NULL);
	}
}

void TcpClient::BufferErrorCallback(struct bufferevent* bev, short events, void* arg) {
	Channel* channel = static_cast<Channel*>(arg);
	thread_t* thr = channel->getChannelThread();
	if (thr->client_->error_func_) {
		thr->client_->error_func_(channel, NULL);
	}
}


}/*end namespace rpc*/

