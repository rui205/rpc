#include <assert.h>

#include "MemoryPool.h"

#include "glog/logging.h"

namespace rpc {

#define START_SIZE 5
#define ALIGN_PTR(PTR, ALIGNMENT) (PTR + (-(ssize_t)(PTR) & (ALIGNMENT-1)))


static size_t pool_sizes[CACHING_POOL_ARRAY_SIZE] = {	
	256, 512, 1024, 2048, 
	4096, 8192,12288, 16384, 
	20480,24567, 28672, 32768, 
	40960,49125, 57344, 65535
};

size_t get_pool_capacity(pool_t* pool) {
	if (pool != NULL) {
		return pool->capacity_;
	} 

	return 0;
}

pool_t* create_pool__(pool_factory_t* factory, const char* name, size_t init_size, size_t incr_size) {
	ssize_t i = 0;
	
	if (init_size <= pool_sizes[START_SIZE]) {
		for (i = START_SIZE - 1; i >= 0 && pool_sizes[i] >= init_size; -- i) {
			;
		}
	} else {
		for (i = START_SIZE + 1; i < CACHING_POOL_ARRAY_SIZE && pool_sizes[i] < init_size; ++ i) {
			;
		}
	}

	pool_t* pool = NULL;
	pool_mgr_t* pool_mgr = (pool_mgr_t*)factory;
	

	pthread_mutex_lock(&pool_mgr->mutex_);

	LOG(INFO) << "iii: " << i;	
	if (i == CACHING_POOL_ARRAY_SIZE || TAILQ_EMPTY(&pool_mgr->free_pool_list_[i])) {
		if (i < CACHING_POOL_ARRAY_SIZE) {
			init_size = pool_sizes[i];
		}

		// reduce memory allocations 
		assert(init_size >= sizeof(pool_t) + sizeof(pool_chunk_t));
		unsigned char* buf = (unsigned char*)factory->policy_.chunk_alloc(factory, init_size);
		if (buf == NULL) {
			pthread_mutex_unlock(&pool_mgr->mutex_);
			LOG(ERROR) << "factory policy chunk_alloc failed, buf is nil";
			return NULL;
		}

		pool = (pool_t*)buf;
		TAILQ_INIT(&pool->chunk_list_);
		pool->factory_ = factory;
		pool_chunk_t* chunk = (pool_chunk_t*)(buf + sizeof(pool_t));
		chunk->buf_ = ((unsigned char*)chunk) + sizeof(pool_chunk_t);
		chunk->cur_ = ALIGN_PTR(chunk->buf_, POOL_ALIGNMENT);
		chunk->end_ = buf + init_size;
		TAILQ_INSERT_TAIL(&pool->chunk_list_, chunk, entry);
		pool->capacity_ = init_size;
		pool->incr_size_ = incr_size;
		strncpy(pool->name_, name, sizeof(pool->name_));
		//pool_mgr->capacity_ += init_size;
	} else {
		LOG(INFO) << "=========the i: " << i;
		pool = TAILQ_FIRST(&pool_mgr->free_pool_list_[i]);
		TAILQ_REMOVE(&pool_mgr->free_pool_list_[i], pool, entry);
		pool->incr_size_ = incr_size;
        strncpy(pool->name_, name, sizeof(pool->name_));

		if (pool_mgr->capacity_ > get_pool_capacity(pool)) {
			pool_mgr->capacity_ -= get_pool_capacity(pool);
		} else {
			pool_mgr->capacity_ = 0;
		}
	}

    pool->data_ = (void*)(ssize_t)i;
    TAILQ_INSERT_HEAD(&pool_mgr->used_pool_list_, pool, entry);
    ++ pool_mgr->used_count_;

	LOG(INFO) << "pool_mgr used_count_: " << pool_mgr->used_count_;
	pthread_mutex_unlock(&pool_mgr->mutex_);

	return pool;
}

bool on_chunk_alloc__(pool_factory_t* factory, size_t size) {
	pool_mgr_t* pool_mgr = (pool_mgr_t*)factory;

	pool_mgr->used_size_ += size;

	return true;
}

void on_chunk_free__(pool_factory_t* factory, size_t size) {
	pool_mgr_t* pool_mgr = (pool_mgr_t*)factory;
	pool_mgr->used_size_ -= size;
}

void release_pool__(pool_factory_t* factory, pool_t* pool) {
	if (factory == NULL || pool == NULL) {
		return;
	}

	pool_mgr_t* pool_mgr = (pool_mgr_t*)factory;

	pthread_mutex_lock(&pool_mgr->mutex_);

	int exist = 0;
	pool_t* tmp = NULL;
	TAILQ_FOREACH(tmp, &pool_mgr->used_pool_list_, entry) {
		if (tmp == pool) {
			LOG(INFO) << "pool name: " << tmp->name_;
			exist = 1;
			break;
		}
	}

	if (exist == 0) {
		pthread_mutex_unlock(&pool_mgr->mutex_);
		return;
	}

	TAILQ_REMOVE(&pool_mgr->used_pool_list_, pool, entry);
	-- pool_mgr->used_count_;

	ssize_t i = (ssize_t)(void*)pool->data_;
	if (i >= 16) {
	
	}

	LOG(INFO) << "release pool ===";

	size_t pool_capacity = get_pool_capacity(pool);
	if (pool_capacity > pool_sizes[15] || 
		(pool_capacity + pool_mgr->capacity_) > pool_mgr->max_capacity_) {

	}

	LOG(INFO) << "release i: " << i;

	TAILQ_INSERT_TAIL(&pool_mgr->free_pool_list_[i], pool, entry);
	pool_mgr->capacity_ += pool_capacity;

	pthread_mutex_unlock(&pool_mgr->mutex_);
}

pool_t* create_pool(pool_factory_t* factory, const char* pool_name, size_t init_size, size_t incr_size) {
    return (*factory->create_pool)(factory, pool_name, init_size, incr_size);
}

void release_pool(pool_factory_t* factory, pool_t* pool) {
	(*factory->release_pool)(factory, pool);
}

void pool_mgr_init(pool_mgr_t* pool_mgr, const pool_factory_policy_t* policy, size_t max_capacity) {
	memset(pool_mgr, 0, sizeof(*pool_mgr));
	pool_mgr->max_capacity_ = max_capacity;
	pthread_mutex_init(&pool_mgr->mutex_, NULL);

	TAILQ_INIT(&pool_mgr->used_pool_list_);
	for (int i = 0; i < CACHING_POOL_ARRAY_SIZE; ++ i) {
		TAILQ_INIT(&pool_mgr->free_pool_list_[i]);
	}

	if (policy == NULL) {
		policy = &default_policy;
	}

	memcpy(&pool_mgr->factory_.policy_, policy, sizeof(*policy));
		
	pool_mgr->factory_.create_pool = &create_pool__;
	pool_mgr->factory_.release_pool = &release_pool__;
//	pool_mgr->factory.dump_status = &dump_status__;
	pool_mgr->factory_.on_chunk_alloc = &on_chunk_alloc__;
	pool_mgr->factory_.on_chunk_free =  &on_chunk_free__;

}

void pool_mgr_destroy(pool_mgr_t* pool_mgr) {

}

size_t get_pool_manager_reference(pool_mgr_t* pool_mgr) {
	return pool_mgr->used_count_; 
}

size_t get_pool_manager_max_capacity(pool_mgr_t* pool_mgr) {
	return pool_mgr->max_capacity_;
}

size_t get_pool_manager_memory_used_size(pool_mgr_t* pool_mgr) {
	return pool_mgr->used_size_;
}

}/*end namespace rpc*/
