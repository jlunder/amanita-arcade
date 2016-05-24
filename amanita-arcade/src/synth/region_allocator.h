#ifndef REGION_ALLOCATOR_H
#define REGION_ALLOCATOR_H


#include "synth_core.h"


#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus


/* todo: detect machine word length/recommended alignment */
#define REGION_ALLOCATOR_MEMORY_ALIGNMENT 4
#define REGION_ALLOCATOR_DECLARE_MEMORY(name, size) \
	uint8_t name[size] __attribute__((__aligned__( \
	REGION_ALLOCATOR_MEMORY_ALIGNMENT)))


typedef struct region_allocator_ {
	uint8_t * memory;
	size_t memory_size;
	size_t memory_used;
} region_allocator_t;


extern void region_allocator_init(region_allocator_t * allocator,
	void * memory_pool, size_t memory_pool_size);
extern void region_allocator_destroy(region_allocator_t * allocator);
extern void * region_allocator_allocate(region_allocator_t * allocator,
	size_t size);
extern intptr_t region_allocator_mark(region_allocator_t * allocator);
extern void region_allocator_release(region_allocator_t * allocator,
	intptr_t mark);


#ifdef __cplusplus
}
#endif // __cplusplus


#endif /* REGION_ALLOCATOR_H */


