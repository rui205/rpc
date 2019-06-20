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

size_t pool_get_capacity(pool_t* pool) {
	return pool->capacity_;
}

pool_t* create_pool__(pool_factory_t* factory, const char* name, size_t init_size, size_t incr_size) {
	int i = 0;
	
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

	LOG(INFO) << "iii: " << i;	
	if (i == CACHING_POOL_ARRAY_SIZE || TAILQ_EMPTY(&pool_mgr->free_pool_list_[i])) {
		if (i < CACHING_POOL_ARRAY_SIZE) {
			init_size = pool_sizes[i];
		}

		// reduce memory allocations 
		assert(init_size >= sizeof(pool_t) + sizeof(pool_chunk_t));
		unsigned char* buf = (unsigned char*)factory->policy_.chunk_alloc(factory, init_size);
		if (buf == NULL) {
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
	} else {
		LOG(INFO) << "=========the i: " << i;
		pool = TAILQ_FIRST(&pool_mgr->free_pool_list_[i]);
		TAILQ_REMOVE(&pool_mgr->free_pool_list_[i], pool, entry);
		pool->incr_size_ = incr_size;
        strncpy(pool->name_, name, sizeof(pool->name_));

		if (pool_mgr->capacity_ > pool_get_capacity(pool)) {
			pool_mgr->capacity_ -= pool_get_capacity(pool);
		} else {
			pool_mgr->capacity_ = 0;
		}
	}

    pool->data_ = (void*)(ssize_t)i;
    TAILQ_INSERT_HEAD(&pool_mgr->used_pool_list_, pool, entry);
    ++ pool_mgr->used_count_;

	return pool;
}

pool_t* create_pool(pool_factory_t* factory, const char* pool_name, size_t init_size, size_t incr_size) {
	LOG(INFO) << "mid: " << factory;
    pool_t* pool = (*factory->create_pool)(factory, pool_name, init_size, incr_size);
    return pool;
}

void pool_mgr_init(pool_mgr_t* pool_mgr, const pool_factory_policy_t* policy, size_t max_capacity) {
//	memset(pool_mgr, 0, sizeof(*pool_mgr));
	pool_mgr->max_capacity_ = max_capacity;

	TAILQ_INIT(&pool_mgr->used_pool_list_);
	for (int i = 0; i < CACHING_POOL_ARRAY_SIZE; ++ i) {
		TAILQ_INIT(&pool_mgr->free_pool_list_[i]);
	}

	if (policy == NULL) {
		policy = &default_policy;
	}

	memcpy(&pool_mgr->factory_.policy_, policy, sizeof(*policy));
		
	pool_mgr->factory_.create_pool = &create_pool__;
//	pool_mgr->factory.release_pool = &release_pool_;
//	pool_mgr->factory.dump_status = &dump_status__;
//	pool_mgr->factory.on_chunk_alloc = &on_chunk_alloc__;
//	pool_mgr->factory.on_chunk_free =  &on_chunk_free__;

}

}/*end namespace rpc*/
