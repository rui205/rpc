#include <assert.h>
#include <stdlib.h>

#include "TaskQueue.h"

namespace rpc {

TaskQueue::TaskQueue() {
	/*TaskQueue()*/
}

TaskQueue::~TaskQueue() {
	if (freeitems_ != NULL) {
		::free(freeitems_);
		freeitems_ = NULL;
	}

	freeItemCurr_ = 0;
	freeItemTotal_ = CACHED_TASK_ITEM_COUNT;
}

void TaskQueue::init() {
	freeItemTotal_ = CACHED_TASK_ITEM_COUNT;
	freeItemCurr_ = 0;

	head_ = NULL;
	tail_ = NULL;

	freeitems_ = (task_item_t**)calloc(1, sizeof(task_item_t*) * freeItemTotal_);
	assert(freeitems_);
}

void TaskQueue::push(task_item_t* item) {
	if (item == NULL) {
		return;
	}

	item->next_ = NULL;

	/*thread safe*/
	mutex_.lock();

	if (tail_ == NULL) {
		head_ = item;
	} else {
		tail_->next_ = item;
	}

	tail_ = item;

	mutex_.unlock();
}

void TaskQueue::free(task_item_t* item) {
	if (item == NULL) {
		return;
	}

	mutex_.lock();

	if (addToFreelist(item) == -1) {
		::free(item);
		item = NULL;
	}

	mutex_.unlock();
}

task_item_t* TaskQueue::pop() {
	task_item_t* item = NULL;

	mutex_.lock();

	item = head_;
	if (item != NULL) {
		head_ = head_->next_;
		if (head_ == NULL) {
			tail_ = NULL;
		}
	}

	mutex_.unlock();

	return item;
}

task_item_t* TaskQueue::getFromFreelist() {
	task_item_t* item = NULL;

//	mutex_.lock();

	if (freeItemCurr_ > 0) {
		item = freeitems_[-- freeItemCurr_];
	}

	if (item == NULL) {
		item = (task_item_t*)calloc(1, sizeof(task_item_t));
	}

//	mutex_.unlock();

	return item;
}

/*if return -1, cached task item > cached max count*/
int TaskQueue::addToFreelist(task_item_t* item) {
	int rc = -1;

	if (freeItemCurr_ < freeItemTotal_) {
		freeitems_[freeItemCurr_ ++] = item;
		rc = 0;
	} else {
		int newSize = freeItemTotal_ * 2;
		if (newSize > CACHED_TASK_ITEM_MAX) {
			return rc;
		} else {
			task_item_t** newFreeItemlist = (task_item_t**)realloc(freeitems_, newSize * sizeof(task_item_t*));
			if (newFreeItemlist != NULL) {
				freeItemTotal_ = newSize;
				freeitems_ = newFreeItemlist;
				freeitems_[freeItemCurr_ ++] = item;
				reallocCount_ ++;
				rc = 0;
			}		
		}
	}

	return rc;
}

int TaskQueue::getReallocCount() {
	return reallocCount_;
}

bool TaskQueue::empty() {
	if (head_ == NULL && tail_ == NULL) {
		return true;
	} else {
		return false;
	}
}

}/*end namespace rpc*/

