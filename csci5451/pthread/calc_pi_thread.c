/*
 * This program is to calculate pi with pthread.
 * See calc_pi.c for the alogrithm.
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <pthread.h>
#include <time.h>

/* １億点をプロットする   */
/* １０個のスレッドを使う */
#define NUM_PLOTS   100000000
#define NUM_THREADS 10

/* 共有したいメモリはグローバルにする(あるいは構造体で渡すなど) */
int hits[NUM_THREADS];

void calcPI(void *arg){
	int *hit_ptr = (int*)arg;
	unsigned int seed = *hit_ptr;
	int hit  = 0;
	const int NUM_LOOP = NUM_PLOTS/NUM_THREADS;

	for(int i=0;i<NUM_LOOP;i++){
		double x = rand_r(&seed)/(double) RAND_MAX;
		double y = rand_r(&seed)/(double) RAND_MAX;
		double length_square = (x-0.5)*(x-0.5) + (y-0.5)*(y-0.5);
		if(length_square < 0.25) hit++;
		seed += seed/2 -seed % (seed/2);
	}
	*hit_ptr = hit;
	pthread_exit(0);
} 

int main(){
	pthread_t threads[NUM_THREADS];
	struct timeval start_time,end_time;

	srand((unsigned int) time(NULL));
	for(int i=0;i<NUM_THREADS;i++){
		hits[i] = rand() % 100;
	}

	/* 計測スタート */
	gettimeofday(&start_time,NULL);
	for(int i=0;i<NUM_THREADS;i++){
		pthread_create(&threads[i],NULL,calcPI,(void*)&hits[i]);
	}

	for(int i=0;i<NUM_THREADS;i++){
		pthread_join(threads[i],NULL);
	}

	gettimeofday(&end_time,NULL);
	int elapsed = (end_time.tv_sec-start_time.tv_sec) * (int)1e6 
		+ (end_time.tv_usec-start_time.tv_usec);
	int sum = 0;
	for(int i=0;i<NUM_THREADS;i++){
		sum += hits[i];
	}
	double ratio = sum / (double) NUM_PLOTS;
	printf("Total plots : %d\n",NUM_PLOTS);
	printf("Hit   plots : %d\n",sum);
	printf("PI = %lf\n",ratio * 4);
	printf("Elapsed time is %lf\n",elapsed/(double)1e6);
	return 0;
}

	
