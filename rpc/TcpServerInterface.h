#ifndef __TCP_SERVER_INTERFACE__
#define __TCP_SERVER_INTERFACE__

//#include "Channel.h"

#include <functional>

namespace rpc {

class Channel;

typedef std::function<void(Channel*, void*)> callback_t;

class TcpServerInterface {
public:
	virtual ~TcpServerInterface() {}				
	virtual void SettingCallback(callback_t readcb, callback_t writecb, callback_t errorcb) = 0;
//	virtual void RegisterService() = 0;
	virtual void Start() = 0;
	virtual void Stop() = 0;
};

}/*end namespace rpc*/

#endif

