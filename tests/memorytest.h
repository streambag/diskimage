#ifndef __MEMORYTEST_H__
#define __MEMORYTEST_H__

/*
 * Add the _fake suffix to all function calls using the TESTSEAM macro.
 * This allows us to provide our own versions of functions from the standard
 * library.
 */
#define TESTSEAM(functionname) functionname ## _fake

/*
 * Should be called from the fake function when memory has been allocated.
 * Allocation registrations are compared to frees in check_memory()
 */
void register_allocation(void *ptr);

/*
 * Should be called from the fake function when memory has been freed.
 * Allocation registrations are compared to frees in check_memory()
 */
void register_free(void *ptr);

/*
 * Returns the total number of allocations performed since reset_memory was last
 * called. This includes allocations that have been freed.
 */
int get_allocation_count();

/*
 * Reset all memory statistics. Should be called before any test case that
 * performs memory checks.
 */
void reset_memory();

/*
 * Set up counters to simulate an allocation failure. If the argument is 0,
 * the first allocation (and all following) will fail. If the argument is 1,
 * the second allocation (and all following) will fail, and so on.
 */
void simulate_allocation_failure(int allocations_to_failure);

/*
 * Returns true if the next allocation should be a simulated failure.
 */
bool should_simulate_allocation_failure();

/*
 * Generates a test failure if all allocated memory has not been deallocated.
 */
void check_memory();

/*
 * Default test implementation of malloc that keeps track of all allocations,
 * and can also simulate allocation failures.
 */
void *malloc_fake(size_t size);

/*
 * Default test implementation of free that keeps track of freed memory.
 */
void free_fake(void *ptr);

#endif
