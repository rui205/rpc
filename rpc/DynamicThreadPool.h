#include <thread>
#include <list>
#include <queue>
#include <memory>
#include <mutex>
#include <condition_variable>

#include "ThreadPoolInterface.h"

namespace rpc { 

class DynamicThreadPool final: public ThreadPoolInterface {
public:
    explicit DynamicThreadPool(int reserve_threads);
    ~DynamicThreadPool();
    void add(const std::function<void()>& callback) override;

private:
    class DynamicThread {
    public:
        DynamicThread(DynamicThreadPool* pool);
        ~DynamicThread();

    private:
        DynamicThreadPool* pool_;
        std::unique_ptr<std::thread> thd_;
        void threadFunc();
    };

    void threadFunc();
    static void reapThreads(std::list<DynamicThread*>* tlist);

private:
    bool shutdown_;
    int nthreads_;
    int reserve_threads_;
    int threads_waiting_;

    std::mutex mutex_;
    std::condition_variable  cv_;
    std::condition_variable  shutdown_cv_;
    std::queue<std::function<void()>> callbacks_;
    std::list<DynamicThread*> dead_threads_;
};

}/*end namespace rpc*/ 
