/* We need to have some way to clean up unused memory in our interpreted
 * language. Mark and Sweep is probabbly our best bet since it's supposed
 * to be relatively simple and has fewer pitfalls than reference counting
 * (reference counting cannot deal with cyclic references
 * https://en.wikipedia.org/wiki/Reference_counting#Advantages_and_disadvantages)
 *
 *
 * Mark and Sweep:
 * https://en.wikipedia.org/wiki/Tracing_garbage_collection#Na%C3%AFve_mark-and-sweep
 *
 * Thinking right now: We create a custom gc_malloc that has extra information
 * hidden in the allocation and link together all allocations with a linked
 * list. The "root set" will just be all the variables that exist in the current
 * environment.
 *
 * Mark: Look through each variable and mark that we can access them, including
 * anything the object subsequently points to
 *
 * Sweep: Use the linked list to move through every allocation and free up
 * anything that wasn't marked
 * */

void *GCMalloc(unsigned int Size);
void GCMarkAllocation(void *Allocation);
void GCMarkAndSweep(void);
