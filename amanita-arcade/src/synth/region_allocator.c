#include "synth_core.h"
#include "region_allocator.h"


void region_allocator_init(region_allocator_t * allocator,
	void * memory_pool, size_t memory_pool_size)
{
	if(memory_pool == NULL) {
		fprintf(stderr, "invalid memory pool in region allocator init\n");
	}
	
	allocator->memory = memory_pool;
	allocator->memory_size = memory_pool_size;
	allocator->memory_used = 0;
}


void region_allocator_destroy(region_allocator_t * allocator)
{
	allocator->memory = NULL;
	allocator->memory_size = 0;
	allocator->memory_used = 0;
}


void * region_allocator_allocate(region_allocator_t * allocator,
	size_t size)
{
	void * result;
	
	size = (size + REGION_ALLOCATOR_MEMORY_ALIGNMENT - 1)
		& ~(REGION_ALLOCATOR_MEMORY_ALIGNMENT - 1);
	if(allocator->memory_size - allocator->memory_used < size) {
		fprintf(stderr, "allocation failed in region allocator\n");
		return NULL;
	}
	result = allocator->memory + allocator->memory_used;
	allocator->memory_used += size;
	return result;
}


intptr_t region_allocator_mark(region_allocator_t * allocator)
{
	return (intptr_t)allocator->memory_used;
}


void region_allocator_release(region_allocator_t * allocator, intptr_t mark)
{
	size_t release_used = (size_t)mark;
	if(release_used > allocator->memory_size) {
		fprintf(stderr, "release region allocator to invalid mark point\n");
		return;
	}
	allocator->memory_used = release_used;
}


