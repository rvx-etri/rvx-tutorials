#include "ervp_printf.h"
#include "ervp_multicore_synch.h"

#include <stdio.h>

#define NUMEL 16

const unsigned char init_nums[NUMEL] = {22, 5, 67, 98, 45, 32, 101, 99, 73, 10, 7, 120,1,55,21, 16};
unsigned char nums[NUMEL];
int SelectionSort(unsigned char [], int);
int BubbleSort(unsigned char [], int);

int main (void) {

	int i, moves;

	acquire_lock(0);

	printf ("* Original list : \n");
	printf ("  -> ");
	for (i=0; i < NUMEL; i++) {
		nums[i] = init_nums[i];
		printf("%d ", nums[i]);
	}
	printf("\n");
	printf ("* Sorting Algorithm : Selection(X), Bubble(O) \n");
	//moves = SelectionSort(nums, NUMEL);
	moves = BubbleSort(nums, NUMEL);

	//Print the list after sorting
	printf ("* Sorted list, in ascending order, is : \n");
	printf ("  -> ");
	for (i=0; i < NUMEL; i++) {
		printf("%d ", nums[i]);
	}

	printf("\n* %d moves were made to sort this list\n", moves);
	release_lock(0);
	return 0;
}

int BubbleSort(unsigned char num[], int numel){
	int i, j, temp, moves=0;

	for (i=0; i < (numel-1); i++) {
		for (j=1; j < numel; j++) {
			if (num[j] < num [j-1]) {
				temp = num[j];
				num[j] = num[j-1];
				num[j-1] =temp;
				moves++;
			}
		}
	}
	return(moves);
}


int SelectionSort(unsigned char num[], int numel) {
	int i, j, min, minidx, temp, moves=0;

	for(i=0; i < numel; i++) {
		min=num[i];
		minidx = i;
		for (j=i+1; j <numel; j++) {
			if (num[j] < min) {
				min = num[j];
				minidx = j;
			}
		}
		if (min < num[i]) {
			temp = num[i];
			num[i] = min;
			num[minidx] = temp;
			moves++;
		}
	}
	return (moves);
}


