#include <string>
#include <unordered_map>
#include <functional>

#include "TcpServer.h"

namespace rpc {

typedef struct {
	std::string name_;
	std::function<void(std::string&, void*)> function_;
	void* args_;
} RpcService;

class RpcServer final: public TcpServer {
public:
	RpcServer(std::string addr, int nthreads);
	~RpcServer();

public:
	void Read(Channel* chan, void* arg) override;
	void Write(Channel* chan, void* arg) override;
	void Error(Channel* chan, void* arg) override;
	void RegisterService(RpcService* service);						/*register rpc method*/

private:
	TcpServer* server_;
	std::unordered_map<std::string, RpcService*> services_;			/*mapping of rpc method*/	
};

}/*end namespace rpc*/
