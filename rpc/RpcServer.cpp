#include "RpcServer.h"

namespace rpc {

RpcServer::RpcServer(std::string addr, int nthreads): TcpServer(addr, nthreads) {
	
}

RpcServer::~RpcServer() {
	/*delete name/sevicre mapping*/
}

void RpcServer::Read(Channel* chan, void* arg) {

}

void RpcServer::Write(Channel* chan, void* arg) {

}

void RpcServer::Error(Channel* chan, void* arg) {

}

void RpcServer::RegisterService(RpcService* service) {
	if (service == NULL) {
		return;
	}

	services_[service->name_] = service;
}


}/*end namespace rpc*/
