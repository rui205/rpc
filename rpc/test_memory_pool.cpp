#include "MemoryPool.h"

#include "gflags/gflags.h"
#include "glog/logging.h"

using namespace rpc;

void SetupGoogleLog() {
    FLAGS_alsologtostderr = true;
    FLAGS_colorlogtostderr = true;
    FLAGS_logbufsecs = 0;
    FLAGS_max_log_size = 1800;

    google::InitGoogleLogging("test");
    google::SetLogDestination(google::GLOG_INFO,"log-");
}

void test() {
	pool_mgr_t* pool_mgr = new(std::nothrow) pool_mgr_t;
	if (pool_mgr == NULL) {
		LOG(INFO) << "pool manager is nil";
	}

	pool_manager_init(pool_mgr, NULL, 999999);
	pool_t* pool = create_pool(get_pool_manager_factory(pool_mgr), "test_pool_01", 8192, 4096);
	if (pool == NULL) {
		LOG(INFO) << "pool create failed";
		return;
	}

	char* buf = (char*)pool_alloc(pool, 2045);
	if (buf == NULL) {
		LOG(ERROR) << "allocate memory failed, buf is nil";
		return;
	}	

		
}

int main() {
	SetupGoogleLog();

	LOG(INFO) << "memory thread pool test beg ...";

	test();

	LOG(INFO) << "memory thread pool test end ...";
	return 0;
}
