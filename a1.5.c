/*
    The Merge Sort to use for Operating Systems Assignment 1 2019
    written by Robert Sheehan

    Modified by: Kungeng Wu
    UPI: kwu849

    By submitting a program you are claiming that you and only you have made
    adjustments and additions to this code.
 */

#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h>
#include <sys/resource.h>
#include <stdbool.h>
#include <pthread.h>
#include <sys/sysinfo.h>

#define SIZE    2
#define STK_SIZE 100000000

_Atomic int cores;
pthread_spinlock_t lock;

struct block {
    int size;
    int *first;
};

// void print_block_data(struct block *blk) {
//     printf("size: %d address: %p\n", blk->size, blk->first);
// }

/* Combine the two halves back together. */
void merge(struct block *left, struct block *right) {
	int* combined = malloc(sizeof(int) * (left->size + right->size));
	int dest = 0, l = 0, r = 0;
	while (l < left->size && r < right->size) {
		if (left->first[l] < right->first[r])
			combined[dest++] = left->first[l++];
		else
			combined[dest++] = right->first[r++];
	}
	while (l < left->size)
		combined[dest++] = left->first[l++];
	while (r < right->size)
		combined[dest++] = right->first[r++];
    memmove(left->first, combined, (left->size + right->size) * sizeof(int));
    free(combined);
}

/* Merge sort the data. */
void * merge_sort(struct block *my_data) {
    // print_block_data(my_data);
    pthread_t l_thread, r_thread;
    if (my_data->size > 1) {
        struct block left_block;
        struct block right_block;
	bool t1 = 0, t2 = 0;
	int* combined = malloc(sizeof(int) * (my_data->size));
        left_block.size = my_data->size / 2;
        left_block.first = my_data->first;
        right_block.size = left_block.size + (my_data->size % 2);
        right_block.first = my_data->first + left_block.size;
	// locks thread to check core count and create thread before releasing
	pthread_spin_lock(&lock);
	if (cores >= 1) {
	    cores -= 1;
	    pthread_create(&l_thread, NULL, (void *)*merge_sort, (void *)&left_block);
	    pthread_spin_unlock(&lock);
	    t1 = 1;
	} else {
	    // releases lock when thread is not created
	    pthread_spin_unlock(&lock);
	    merge_sort(&left_block);
	}
	pthread_spin_lock(&lock);
	if (cores >= 1) {
	    cores -= 1;
	    pthread_create(&r_thread, NULL, (void *)*merge_sort, (void *)&right_block);
	    pthread_spin_unlock(&lock);
	    t2 = 1;
	} else {
	    pthread_spin_unlock(&lock);
	    merge_sort(&right_block);
	}
	// joins the thread after finish merging
	if(t1 == 1){
	    pthread_join(l_thread, NULL);
	}
	if(t2 == 1){
	    pthread_join(r_thread, NULL);
	} 
        merge(&left_block, &right_block);
	free(combined);
    }
}

/* Check to see if the data is sorted. */
bool is_sorted(int data[], int size) {

    bool sorted = true;
    for (int i = 0; i < size - 1; i++) {
        if (data[i] > data[i + 1])
            sorted = false;
    }
    return sorted;
}

int main(int argc, char *argv[]) {
    cores = get_nprocs()+1;
    long size;
    // initialise spin lock
    if (pthread_spin_init(&lock, PTHREAD_PROCESS_SHARED) != 0)
    {
        printf("\n spin init failed\n");
        return 1;
    }
	if (argc < 2) {
		size = SIZE;
	} else {
		size = atol(argv[1]);
	}
    struct block start_block;
    // malloc allocates memory necessary based on size of input
    int* data = malloc(sizeof(int)*size);
    start_block.size = size;
    start_block.first = data;
    for (int i = 0; i < size; i++) {
        data[i] = rand();
    }
    printf("starting---\n");
    merge_sort(&start_block);
    printf("---ending\n");
    printf(is_sorted(data, size) ? "sorted\n" : "not sorted\n");
    free(data);
    exit(EXIT_SUCCESS);
}
