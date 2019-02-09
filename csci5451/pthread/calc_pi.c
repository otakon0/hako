/* 
 * Calculate the value of pi by using the area of a square and
 * the area of a circle. These Area is calculated as 1 and pi/4.
 * By plotting many dot, we can get the ratio of how many 
 * the points are inside the circle. The ratio multipled by 4 is pi.
 */

/*
 * No threads
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>

/* １億点プロットする */
#define NUM_PLOTS 100000000

int main(){
	int hit = 0;

	srand((unsigned)time(NULL));

	/* 計算時間を計測したい */
	struct timeval start_time,end_time;
	
	gettimeofday(&start_time,NULL);

	for(int i=0;i<NUM_PLOTS;i++){
		double x = rand()/(double)RAND_MAX;
		double y = rand()/(double)RAND_MAX;

		// determine whether the generated point is inside the circle
		double length_square = (x-0.5) * (x-0.5) + (y-0.5) * (y-0.5);
		if (length_square < 0.25) hit++;
	}
	gettimeofday(&end_time,NULL);
	int elapsed = (end_time.tv_sec-start_time.tv_sec) * (int)1e6 + (end_time.tv_usec-start_time.tv_usec);

	double ratio = hit / (double) NUM_PLOTS;
	printf("Total plots : %d\n",NUM_PLOTS);
	printf("Hit   plots : %d\n",hit);
	printf("PI = %lf\n",ratio * 4);
	printf("Elapsed time is %lf\n",elapsed/(double)1e6);
	return 0;
}
