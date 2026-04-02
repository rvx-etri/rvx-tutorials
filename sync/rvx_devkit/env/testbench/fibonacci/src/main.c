#include "ervp_printf.h"
#include "ervp_core_id.h"
#include "ervp_multicore_synch.h"

#define ITERATIONS 10

int fib(int i) {
    return (i > 1) ? fib(i-1) + fib(i-2) : i;
}

int main() {
	int i;

	acquire_lock(0);

	printf("fibonacci starting...\n");

	for(i = 0; i < ITERATIONS; i++)
		printf("fib(%03d) = %d\n", i, fib(i));

	release_lock(0);

	return 0;
}
