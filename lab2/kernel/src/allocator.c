#include "../include/allocator.h"

/* the variables below all defined in linker script */
extern size_t _heap_stack_size; 
extern char _heap;
extern char _stack;

static char* heap_top = &_heap;
static char* stack_top = &_stack;

void* simple_malloc(size_t size)
{
    if (heap_top + size > stack_top) {
        return NULL;
    }

    char* ret_addr = heap_top;
    heap_top += size;

    return ret_addr;
}