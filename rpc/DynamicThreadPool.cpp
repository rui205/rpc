#include "DynamicThreadPool.h"
#include "glog/logging.h"

namespace rpc {

DynamicThreadPool::DynamicThreadPool(int reserve_threads) {
    shutdown_ = false;
    nthreads_ = 0;
    threads_waiting_ = 0;
    reserve_threads_ = reserve_threads;
 
    for (int i = 0; i < reserve_threads_; ++ i) {
        std::lock_guard<std::mutex> lock(mutex_);
        nthreads_ ++;
        new DynamicThread(this);
    }
}

DynamicThreadPool::~DynamicThreadPool() {
    std::unique_lock<std::mutex> lock(mutex_);
    shutdown_ = true;
    cv_.notify_all();

    while (nthreads_ != 0) {
        shutdown_cv_.wait(lock);
    }

    reapThreads(&dead_threads_);
}

void DynamicThreadPool::add(const std::function<void()>& callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    callbacks_.push(callback);
    if (threads_waiting_ == 0) {
        nthreads_ ++;
		LOG(INFO) << "the current threads: " << nthreads_;
        new DynamicThread(this);
    } else {
		LOG(INFO) << "the current threads: " << nthreads_;
        cv_.notify_one();
    }

    if (!dead_threads_.empty()) {
        reapThreads(&dead_threads_);
    }
}

/*waitting for condition variable, unitl recv signal*/
void DynamicThreadPool::threadFunc() {
    for (; ;) {
        std::unique_lock<std::mutex> lock(mutex_);
        if (!shutdown_ && callbacks_.empty()) {
            if (threads_waiting_ >= reserve_threads_) {
                return;
            }

            threads_waiting_ ++;
            cv_.wait(lock);
			/*it may be spurious wakeups, so we must check callbacks_ whether or not empty*/
            threads_waiting_ --;
        }

		/*never carry lock on exec task*/
        if (!callbacks_.empty()) {
            auto cb = callbacks_.front();
            callbacks_.pop();
            lock.unlock();
            cb();
        } else if (shutdown_) {
            break;
        }
    }
}

/*traverse tlist and delete all thread in tlist*/
void DynamicThreadPool::reapThreads(std::list<DynamicThread*>* tlist) {
    for (auto t = tlist->begin(); t != tlist->end(); t = tlist->erase(t)) {
        delete *t;
    }
}

void DynamicThreadPool::DynamicThread::threadFunc() {
    LOG(INFO) << "dynamic thread func";
    pool_->threadFunc();

	/*thread exit, we should --nthreads_ and push it to dead_threads_*/
    std::unique_lock<std::mutex> lock(pool_->mutex_);
 //   std::lock_guard<std::mutex> lock(pool_->mutex_);
    pool_->nthreads_ --;

	LOG(INFO) << "the current threads: " << pool_->nthreads_;
    pool_->dead_threads_.push_back(this);

    if (pool_->shutdown_ && (pool_->nthreads_ == 0)) {
		LOG(INFO) << "111111111111111";
        pool_->shutdown_cv_.notify_one();
    }
}

//thd_ 只能使用初始化列表进行赋值
DynamicThreadPool::DynamicThread::DynamicThread(DynamicThreadPool* pool)
    : pool_(pool)
    , thd_(new std::thread(&DynamicThreadPool::DynamicThread::threadFunc, this)) {
    LOG(INFO) << "dynamic thread ...";
}

DynamicThreadPool::DynamicThread::~DynamicThread() {
    thd_->join();
    thd_.reset();
}

}/*end namespace rpc*/
