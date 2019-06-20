#ifndef __TASK_QUEUE_H__
#define __TASK_QUEUE_H__

#include <mutex>

#include "Util.h"

namespace rpc {

#define CACHED_TASK_ITEM_COUNT (1000)
#define CACHED_TASK_ITEM_MAX (10000)

typedef struct task_item {
	int fd_;						/*accept socket fd*/
	int event_flags_;				/*event flags(EV_XXXX)*/
	struct task_item* next_;		/*the next task item pointer*/
} task_item_t; 

class TaskQueue: public noncopyable {
public:
	TaskQueue();
	~TaskQueue();
	
public:
	void init();
	void push(task_item_t* item);
	void free(task_item_t* item);
	task_item_t* pop();
	task_item_t* getFromFreelist();
	int addToFreelist(task_item_t* item);
	int getReallocCount();
	bool empty();
	
private:
	task_item_t** freeitems_;       /*free item list*/
	task_item_t* head_;				/*the list head node*/
	task_item_t* tail_;				/*the list tail node*/
	int freeItemCurr_;				/*the current free item count*/
	int freeItemTotal_;				/*the total free item count*/
	int reallocCount_;				/*the freelist realloc count*/
	std::mutex mutex_;				/*the task queue mutex*/	
};

}/*end namespace rpc*/

#endif/*end __TASK_QUEUE_H__*/
