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
    void add(const std::function<void(void*)>& callback) override;

private:
    class DynamicThread {
    public:
        DynamicThread(DynamicThreadPool* pool);
        ~DynamicThread();

    private:
        DynamicThreadPool* pool_;								/*the dynamic threadpool of threads*/
        std::unique_ptr<std::thread> thd_;
        void threadFunc();
    };

    void threadFunc();
    static void reapThreads(std::list<DynamicThread*>* tlist);

private:
    bool shutdown_;												/*threadpool shutdown flag*/
    int nthreads_;												/*threadpool current had threads number*/
    int reserve_threads_;										/*initialize threads number*/
    int threads_waiting_;										/*current idle threads number*/

    std::mutex mutex_;
    std::condition_variable  cv_;
    std::condition_variable  shutdown_cv_;
    std::queue<std::function<void(void*)>> callbacks_;          /*task queue*/ 
    std::list<DynamicThread*> dead_threads_;                    
};

}/*end namespace rpc*/ 
