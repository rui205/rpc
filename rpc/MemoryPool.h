#ifndef __MEMORY_POOL_H__
#define __MEMORY_POOL_H__

#include <stdio.h>
#include <pthread.h>
#include <sys/queue.h>

namespace rpc {

#define POOL_ALIGNMENT 8
#define POOL_NAME_MAX 128
#define CACHING_POOL_ARRAY_SIZE 16

struct pool_s;
struct pool_mgr_s;
struct pool_chunk_s;
struct pool_factory_s;
struct pool_factory_policy_s;

typedef struct pool_s pool_t;
typedef struct pool_mgr_s pool_mgr_t;
typedef struct pool_chunk_s pool_chunk_t;
typedef struct pool_factory_s pool_factory_t;
typedef struct pool_factory_policy_s pool_factory_policy_t;

struct pool_s {
	char name_[POOL_NAME_MAX];
	size_t capacity_;
	size_t incr_size_;
	void* data_;
	pool_factory_t* factory_;
	TAILQ_ENTRY(pool_s) entry;
	TAILQ_HEAD(, pool_chunk_s) chunk_list_;
};

size_t get_pool_capacity(pool_t* pool);
const char* get_pool_name(pool_t* pool);
pool_t* create_pool(pool_factory_t* factory, const char* pool_name, size_t init_size, size_t incr_size);
void release_pool(pool_factory_t* factory, pool_t* pool);
void* pool_alloc_from_chunk(pool_chunk_t* chunk, size_t size);
void* pool_allocate_find(pool_t* pool, size_t size);
void* pool_alloc(pool_t* pool, size_t size);

struct pool_factory_policy_s {
	void* (*chunk_alloc)(pool_factory_t* factory, size_t size);
	void (*chunk_free)(pool_factory_t* factory, void* memory, size_t size);
	unsigned int flag_;
};

extern pool_factory_policy_t default_policy;
pool_factory_policy_t* pool_factory_get_default_policy();

struct pool_factory_s {
	pool_factory_policy_t policy_;
	pool_t* (*create_pool)(pool_factory_t* factory, const char* name, size_t init_size, size_t incr_size);
	void (*release_pool)(pool_factory_t* factory, pool_t* pool);
	void (*dump_status)(pool_factory_t* factory, bool detail);
	void (*on_chunk_free)(pool_factory_t* factory, size_t size);
	bool (*on_chunk_alloc)(pool_factory_t* factory, size_t size);
};

//pool_factory_t* create_pool_factory(const pool_factory_policy_t* policy, size_t max_capacity);

struct pool_mgr_s {
	pool_factory_t factory_;
	size_t capacity_;
	size_t max_capacity_;
	size_t used_count_;
	size_t used_size_;
	void* data_;
	pthread_mutex_t mutex_;

	TAILQ_HEAD(, pool_s) used_pool_list_;
	TAILQ_HEAD(, pool_s) free_pool_list_[CACHING_POOL_ARRAY_SIZE];	
};

/*pool manager public interface*/
void pool_mgr_init(pool_mgr_t* pool_mgr, const pool_factory_policy_t* policy, size_t max_capacity);
void pool_mgr_destroy(pool_mgr_t* pool_mgr);
size_t get_pool_manager_reference(pool_mgr_t* pool_mgr);
size_t get_pool_manager_max_capacity(pool_mgr_t* pool_mgr);
size_t get_pool_manager_memory_used_size(pool_mgr_t* pool_mgr);

}/*end namespce rpc*/

#endif
