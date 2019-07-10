#include <thread>
#include <list>
#include <queue>
#include <memory>
#include <mutex>
#include <condition_variable>

#include "ThreadPoolInterface.h"

namespace rpc { 

/*动态线程池在短时间内有大量的任务到来时，会创建远远高于预先设定的线程*/
/*这样做的好处是，可以及时的处理每一个任务，提高效率。当线程获取不到任务的时候，会进行自动的销毁*/
/*每次添加任务的最后，会检查需要销毁的线程，进行销毁，从而保证了添加一个任务之后，当前线程池内线程的合法性*/
class DynamicThreadPool final: public ThreadPoolInterface {
public:
    explicit DynamicThreadPool(int reserve_threads);
    ~DynamicThreadPool();
    void add(const std::function<void()>& callback) override;
	int getTaskQueueSize();

private:
    class DynamicThread {
    public:
        DynamicThread(DynamicThreadPool* pool);
        ~DynamicThread();

    private:
        DynamicThreadPool* pool_;								/*the dynamic threadpool of threads*/
        std::unique_ptr<std::thread> thd_;						/*thrad pointer*/
        void threadFunc();
    };

    void threadFunc();
    static void reapThreads(std::list<DynamicThread*>* tlist);  /*reap idel thread*/

private:
    bool shutdown_;												/*threadpool shutdown flag*/
    int nthreads_;												/*threadpool current had threads number*/
    int reserve_threads_;										/*initialize threads number*/
    int threads_waiting_;										/*current idle threads number*/
	int max_core_threads_;										/*dynamic threadpool max threads number*/
		
    std::mutex mutex_;
    std::condition_variable  cv_;								
    std::condition_variable  shutdown_cv_;						/*condition variable for destroy dynamic threadpool*/
    std::queue<std::function<void()>> callbacks_;               /*task queue*/
    std::list<DynamicThread*> dead_threads_;                    
};

}/*end namespace rpc*/ 
