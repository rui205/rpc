#include "gflags/gflags.h"
#include "glog/logging.h"
#include "DynamicThreadPool.h"

using namespace rpc;

void SetupGoogleLog() {
    FLAGS_alsologtostderr = true;
    FLAGS_colorlogtostderr = true;
    FLAGS_logbufsecs = 0;
    FLAGS_max_log_size = 1800;

    google::InitGoogleLogging("test");
    google::SetLogDestination(google::GLOG_INFO, "log-");
}

void test_dynamic_threadpool() {
	int reserve_threads = 10;
	DynamicThreadPool* threadpool = new DynamicThreadPool(reserve_threads);
	if (threadpool == NULL) {
		LOG(ERROR) << "new threadpool failed";
		return;
	}

	int a = 10;
	const char* b = "hello, world";
	bool c = true;
	int task_count = 4000;
	int total = 0;
	
	while (task_count -- ) {
		threadpool->add([a, b, c, &total]() {
			if (c) {
				LOG(INFO) << "const: " << b;
			}

			if (a % 2 == 0) {
				LOG(INFO) << "const: " << a;
			}

			total ++;
		});
	}

	sleep(15);

	LOG(INFO) << "queue size: " << threadpool->getTaskQueueSize();

	LOG(INFO) << "total: " << total;
}

int main() {
	SetupGoogleLog();

	LOG(INFO) << "test dynamic threadpool beg";
	test_dynamic_threadpool();
	LOG(INFO) << "test dynamic threadpool end";
		
	return 0;
}

