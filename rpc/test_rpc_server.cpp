#include "gflags/gflags.h"
#include "glog/logging.h"

#include "RpcServer.h"

using namespace rpc;

int main() {

	RpcServer* rpc_server = new(std::nothrow) RpcServer("0.0.0.0:8844", 4);
	if (rpc_server == NULL) {
		LOG(INFO) << "new rpc server failed";
		exit(-1);
	}

//	rpc_server->Start();
	return 0;
}
