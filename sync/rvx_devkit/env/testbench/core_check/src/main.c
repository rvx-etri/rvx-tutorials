#include <stdio.h>
#include "ervp_core_id.h"
#include "ervp_multicore_synch.h"

int num_of_cores = 0;

int main()
{
	acquire_lock(0);
	num_of_cores++;
	printf("\nHello, I am CORE %02d!", EXCLUSIVE_ID);
	release_lock(0);
	
	if(request_unique_grant(0,NUM_CORE))
	{
		printf("\nThe number of cores is %d", num_of_cores);
		release_unique_grant(0,NUM_CORE);
	}
	return 0;
}
