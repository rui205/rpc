#include <list>
#include <string>
#include <functional>

#include "Util.h"
#include "Channel.h"

namespace rpc {

typedef std::function<Channel* channel, void* arg> client_func_ptr;
	
class TcpClient {
public:
	TcpClient(std::string addr, int port);
	~TcpClient();

public:
	void SettingCallback(client_func_ptr read_func, client_func_ptr write_func, client_func_ptrr error_func);
	void Start();

	static void BufferReadCallback(struct bufferevent* bev, void* arg);
	static void BufferWriteCallback(struct bufferevent* bev, void* arg);
	static void BufferErrorCallback(struct bufferevent* bev, short events, void* arg);

public:
	client_func_ptr read_func_;
	client_func_ptr write_func_;
	client_func_ptr error_func_;
	
private:
	std::string ip_;
	int port_;
	thread_t* thr_;
	Channel channel;
};

}/*end namespace rpc*/