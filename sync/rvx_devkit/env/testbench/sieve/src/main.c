#include "ervp_printf.h"
#include "ervp_core_id.h"
#include "ervp_multicore_synch.h"

#define TRUE 1
#define FALSE 0
#define SIZE 256

char flags[SIZE + 1];

int main() {
    int i, prime, k, count, iter;

		acquire_lock(0);

    int print_cnt = 0;
    printf("\n** Eratostenes Sieve Prime Number Computation!\n");

    count = 0;
    for (i = 0; i <= SIZE; i++) flags[i] = TRUE;
    flags[0] = FALSE;
    flags[1] = FALSE;
    for (i = 2; i <= SIZE; i++) {
	    if (flags[i]) {
		    prime = i;
		    printf("%d ", prime);
		    print_cnt++;
		    if ((print_cnt & 0xf) == 0) printf("\n");
		    for (k = i + prime; k <= SIZE; k += prime) flags[k]=FALSE;
		    count++;
	    }
    }

    printf("\n** Finished computing %d prime numbers.\n",count);

		release_lock(0);

    return 0;
}
