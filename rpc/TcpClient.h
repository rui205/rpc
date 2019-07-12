#include <list>
#include <string>
#include <functional>

#include "Channel.h"

namespace rpc {

typedef std::function<void(Channel* channel, void* arg)> callback_t;

class TcpClient {
public:
	TcpClient(std::string addr, int port);
	~TcpClient();

public:
	void SettingCallback(callback_t read_func, callback_t write_func, callback_t error_func);
	void Start();

	static void BufferReadCallback(struct bufferevent* bev, void* arg);
	static void BufferWriteCallback(struct bufferevent* bev, void* arg);
	static void BufferErrorCallback(struct bufferevent* bev, short events, void* arg);

public:
	callback_t read_func_;
	callback_t write_func_;
	callback_t error_func_;
	
private:
	std::string ip_;
	int port_;
	thread_t* thr_;
	Channel* channel;
};

}/*end namespace rpc*/