#include <stdlib.h>
#include <errno.h>

#include <atf-c.h>

static int allocations;
static int deallocations;
static int allocations_to_failure;
static bool internal_should_simulate_allocation_failure;

/*
 * Should be called from the fake function when memory has been allocated.
 * Allocation registrations are compared to frees in check_memory()
 */
void register_allocation(void *ptr)
{
	allocations++;
	allocations_to_failure--;
}

/*
 * Should be called from the fake function when memory has been freed.
 * Allocation registrations are compared to frees in check_memory()
 */
void register_free(void *ptr)
{
	deallocations++;
}

/*
 * Returns the total number of allocations performed since reset_memory was last
 * called. This includes allocations that have been freed.
 */
int get_allocation_count()
{
	return allocations;
}

/*
 * Reset all memory statistics. Should be called before any test case that
 * performs memory checks.
 */
void reset_memory()
{
	allocations = 0;
	deallocations = 0;
	allocations_to_failure = -1;
	internal_should_simulate_allocation_failure = false;
}

/*
 * Set up counters to simulate an allocation failure. If the argument is 0,
 * the first allocation (and all following) will fail. If the argument is 1,
 * the second allocation (and all following) will fail, and so on.
 */
void simulate_allocation_failure(int allocations_to_failure_param)
{
	internal_should_simulate_allocation_failure = true;
	allocations_to_failure = allocations_to_failure_param;
}

/*
 * Returns true if the next allocation should be a simulated failure.
 */
bool should_simulate_allocation_failure()
{
	return (internal_should_simulate_allocation_failure && allocations_to_failure <= 0);

}

/*
 * Generates a test failure if all allocated memory has not been deallocated.
 */
void check_memory()
{
	ATF_CHECK_EQ(allocations, deallocations);
}

/*
 * Default test implementation of malloc that keeps track of all allocations,
 * and can also simulate allocation failures.
 */
void *malloc_fake(size_t size)
{
	if (should_simulate_allocation_failure()) {
		errno = ENOMEM;
		return NULL;
	}

	void *res = malloc(size);
	register_allocation(res);
	return res;
}

/*
 * Default test implementation of free that keeps track of freed memory.
 */
void free_fake(void *ptr)
{
	free(ptr);
	register_free(ptr);
}

