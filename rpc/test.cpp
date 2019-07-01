#include <iostream>
#include <sys/queue.h>

#include "gflags/gflags.h"
#include "glog/logging.h"

//#include "libevent/event2/keyvalq_struct.h"
//#include "libevent/event.h"
#include "Dao.h"
#include "TcpServer.h"
#include "MemoryPool.h"
#include "DynamicThreadPool.h"

void setGoogleLogging() {
    FLAGS_alsologtostderr = true;
    FLAGS_colorlogtostderr = true;
    FLAGS_logbufsecs = 0;
    FLAGS_max_log_size = 1800;

    google::InitGoogleLogging("test");
    google::SetLogDestination(google::GLOG_INFO,"log-");
}

    // struct event_base* base = event_base_new();
    // if (base == NULL) {
    //     LOG(ERROR) << "base is nil";
    //     return -1;
    // }

    // LOG(ERROR) << "cc111";
    // rpc::MySQL* client = new rpc::MySQL(base, "localhost", "root", ""/*none*/, "mysql", 3306);
    // if (client == NULL) {
    //     LOG(ERROR) << "mysql client new failed";
    //     return -1;
    // }

    // rpc::MySQLStatement* stmt = client->prepare("SELECT * FROM `sample` WHERE `id` != ?");
    // if (stmt == NULL) {
    //     LOG(ERROR) << "mysql statment is nil";
    //     return -1;
    // }

    // int cond = 4;
    // stmt->params(0, &cond, MYSQL_TYPE_LONG);

    // int id = -1;
    // int value = -1;
    // char name[64];
    // memset(name, 0, sizeof(name));

    // stmt->bind(0, &id,    sizeof(id),    MYSQL_TYPE_LONG);
    // stmt->bind(2, &value, sizeof(value), MYSQL_TYPE_LONG);
    // stmt->bind(1, &name,  sizeof(name),  MYSQL_TYPE_STRING);

    // if (stmt->execute() == 0) {
    //     while (stmt->fetch() == 0) {
    //         LOG(INFO) << id << ": " << id << "  name: " << name << " value: " << value;
    //     }
    // }

class test_echo_server final: public rpc::TcpServerImpl {
public:
	test_echo_server(): rpc::TcpServerImpl(std::string("0.0.0.0:9900"), 10) {}
	~test_echo_server() {}
	void Read(rpc::Channel* chan, void* arg) {
		if (chan == NULL) {
			return;
		}

		LOG(INFO) << "user read callback function";

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
		LOG(INFO) << "pkg total length: " << pkgTotal;
		LOG(INFO) << "read buffer: " << chan->getReadBufferLength();
		
		if (chan->getReadBufferLength() < 8 + pkgTotal) {
			return;
		}

		char* buffer = (char*)calloc(1, pkgTotal);
		if (buffer == NULL) {
			LOG(ERROR) << "user read callback calloc met error: " << strerror(errno);
			return;
		}
		
		int readTotal = 0;
		chan->readToBuffer((char*)&pkgHeader, 8);
		LOG(INFO) << "the first read buffer: " << pkgHeader;
			
		memset(buffer, 0, pkgTotal);
		readTotal = chan->readToBuffer(buffer, pkgTotal);
		if (readTotal != pkgTotal) {
			LOG(ERROR) << "the readTotal: " << readTotal << "  pkgTotal: " << pkgTotal;
		}
		
		LOG(INFO) << "buffer: " << buffer;

		// doing...
		//

		free(buffer);
		buffer = NULL;
	}

	void Write(rpc::Channel* chan, void* arg) {
		LOG(INFO) << "echo_server write";
	}

	void Error(rpc::Channel* chan, void* arg) {
		LOG(INFO) << "echo_server error";
	}
};

struct node {
	int a;
	int b;
	TAILQ_ENTRY(node) entry;
};

void test_list() {
	TAILQ_HEAD(, node) tq;
	TAILQ_INIT(&tq);

	for (int i = 0; i < 10; ++ i) {
		struct node* n = (struct node*)calloc(1, sizeof(struct node));
		n->a = i;
		n->b = i + 1;
		TAILQ_INSERT_TAIL(&tq, n, entry);
	}

	struct node* tmp = TAILQ_FIRST(&tq);
	LOG(INFO) << "tmp a: " << tmp->a << "  b: " << tmp->b;
	TAILQ_REMOVE(&tq, tmp, entry);

	tmp = TAILQ_FIRST(&tq);
	LOG(INFO) << "new head: " << tmp->a;
	//tmp = TAILQ_NEXT(tmp, entry);
	//LOG(INFO) << "tmp a: " << tmp->a << "  b: " << tmp->b;
	
	struct node* n;
	TAILQ_FOREACH(n, &tq, entry) {
		LOG(INFO) << "node a: " << n->a << "  b: " << n->b;
	}
}
	

void test_task(void* arg) {
	LOG(INFO) << "doing task";
}

void test_dynamic_thread_pool() {
	rpc::DynamicThreadPool* thr_pool = new(std::nothrow) rpc::DynamicThreadPool(5);
	if (thr_pool == NULL) {
		LOG(ERROR) << "thread pool is nil";
		return;
	}
	LOG(INFO) << "thread pool success";	

	thr_pool->add(test_task);

	sleep(3);
}

void test_echo() {
	test_echo_server* server = new test_echo_server();
	server->Start();
}

int main() {
	setGoogleLogging();
	//event_enable_debug_logging(EVENT_DBG_ALL);

	test_echo();
	return 0;
//	test_dynamic_thread_pool();
	//test_list();
	//
	rpc::pool_mgr_t pool_mgr;
	//pool_mgr.capacity_ = 1002;
	rpc::pool_mgr_init(&pool_mgr, NULL, 8192);
	rpc::pool_t* pool = rpc::create_pool(&pool_mgr.factory_, "test01", 8192, 4096);
	if (pool == NULL) {
		LOG(ERROR) << "1111111 is nil";
	}

	rpc::pool_t* pool2 = rpc::create_pool(&pool_mgr.factory_, "test02", 8192, 4096);
	if (pool2 == NULL) {
		LOG(ERROR) << "2222222 is nil";
	}

	rpc::pool_t* pool3 = rpc::create_pool(&pool_mgr.factory_, "test03", 8192, 2048);
	if (pool3 == NULL)  {
		LOG(ERROR) << "3333333 is nil";
	}

	LOG(INFO) << "pool_mgr count: " << pool_mgr.used_count_;

	rpc::release_pool(&pool_mgr.factory_, pool3);

	//
	
/*
	rpc::TcpServer* server = new(std::nothrow) rpc::TcpServer("47.104.153.114:6298", 4);
	if (server != NULL) {
		server->settingCallback(test01_read_callback, NULL, NULL);
		LOG(INFO) << "server new success";
		server->start();
	} else {
		LOG(ERROR) << "server new failed";
	}
*/
	return 0;
}
