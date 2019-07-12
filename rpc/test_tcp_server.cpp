#include <arpa/inet.h>
#include <stdlib.h>

#include "gflags/gflags.h"
#include "glog/logging.h"

#include "TcpServer.h"

/*example for tcp server read and write*/
/*the channel timeout 10s(closed)*/
class TcpServerImpl final: public rpc::TcpServer {
public:
	TcpServerImpl(): rpc::TcpServer(std::string("0.0.0.0:9900"), 4) {

	}
	
	~TcpServerImpl() {

	}

	void Read(rpc::Channel* chan, void* arg) {
		if (chan == NULL) {
			LOG(ERROR) << "channel is nil";
			return;
		}

		LOG(INFO) << "On Read";

		int len = chan->getReadBufferLength();
		if (len < 8) {
			return;
		}

		uint64_t pkgHeader = 0;
		int rc = chan->copyToBuffer((char*)&pkgHeader, 8);
		if (rc < 8) {
			return;
		}

		int pkgTotal = ntohl(pkgHeader);
		
		if (chan->getReadBufferLength() < pkgTotal + 8) {
			return;
		}

		char* buffer = (char*)calloc(1, pkgTotal);
		if (buffer == NULL) {
			LOG(ERROR) << "buffer allocate failed: " << strerror(errno);
			return;
		}

		pkgHeader = 0;
		rc = chan->readToBuffer((char*)&pkgHeader, 8);
		if (rc < 8 || pkgHeader <= 0) {
			return;
		}

		LOG(INFO) << "package total length: " << pkgTotal;
		
		rc = chan->readToBuffer(buffer, pkgTotal);
		if (rc != pkgTotal) {
			LOG(ERROR) << "channel read failed";
			return;
		}	

		LOG(INFO) << "buffer:\n" << buffer;

		rc = chan->appendToBuffer(buffer, pkgTotal);

		// doing
		//
		//
		
		free(buffer);
		buffer = NULL;
	}

	void Write(rpc::Channel* chan, void* arg) {
		if (chan == NULL) {
			return;
		}

		LOG(INFO) << "On Write";
	}

	void Error(rpc::Channel* chan, void* arg) {
		if (chan == NULL) {
			return;
		}

		LOG(INFO) << "On Error";
	}
};

void SetupGoogleLog() {
    FLAGS_alsologtostderr = true;
    FLAGS_colorlogtostderr = true;
    FLAGS_logbufsecs = 0;
    FLAGS_max_log_size = 1800;

    google::InitGoogleLogging("test");
    google::SetLogDestination(google::GLOG_INFO,"log-");
}

int main() {
	SetupGoogleLog();
	TcpServerImpl* server = new(std::nothrow) TcpServerImpl;
	if (server) {
		server->Start();
	}

	return 0;
}
