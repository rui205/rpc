#include "MemoryPool.h"

#include "glog/logging.h"

namespace rpc {

/*
static void* __policy_block_alloc(pool_factory_t* factory, size_t size) {
    void* p = NULL;

    if (factory->on_block_alloc) {
        int rc = factory->on_block_alloc(factory, size);
        if (!rc) {
            return p;
        }


        printf("in malloc block\n");
        p = malloc(size + (SIG_SIZE << 1));
        if (p == NULL) {
            if (factory->on_block_free) {
                factory->on_block_free(factory, size);
            }
        } else {
            APPLY_SIG(p, size);
        }
    }

    if (p != NULL) {
        printf("the __policy_block_alloc success\n");
    }
    return p;
}

}

*/
void* chunk_alloc__(pool_factory_t* factory, size_t size) {
	if (factory->on_chunk_alloc) {
		if (!factory->on_chunk_alloc(factory, size)) {
			return NULL;
		}
	}

	void* p = calloc(1, size);
	if (p == NULL) {
		return NULL;
	}

	return p;
}

void chunk_free__(pool_factory_t* factory, void* memory, size_t size) {
	if (factory->on_chunk_free) {
		factory->on_chunk_free(factory, size);
	}

	free(memory);
	memory = NULL;
}

pool_factory_policy_t default_policy {
    chunk_alloc__,
    chunk_free__,
	0,
};

pool_factory_policy_t* pool_factory_get_default_policy() {
	return &default_policy;
}

}/*end namespace rpc*/
