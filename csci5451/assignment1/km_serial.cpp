/*
 * K-Means Algorithm without threads.
 */
#ifndef _POSIX_C_SOURCE
	#define _POSIX_C_SOURCE 199309L
#endif
#include <time.h>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <sstream>
#include <string>

/* Assume that this runs on LINUX */
static inline double monotonic_seconds(){
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC,&ts);
	return ts.tv_sec + ts.tv_nsec * 1e-9;
}
static void print_time(const double seconds){
	printf("k-means clustering time: %0.04fs\n",seconds);
}


/* large.txt 509256 500 */
int main(int argc,char *argv[]){

	if(argc != 4){
	    printf("Usage %s k-meshdata number_of_cluster number_of_threads\n",argv[0]);
	    return 0;
	}

	std::ifstream file(argv[1]);
	if(!file){
	    printf("Failure to open %s\n",argv[1]);
	    return 0;
	}

	/* Assume that the first line reads Number of data and The dimention */
	int ncluster,dimension;

	{
	    std::string line;
	    getline(file,line);
	    std::istringstream stream(line);
	    std::string token;
	    stream >> ncluster >> dimension;
	}

	printf("The number of cluster    : %d\n"
	       "The number #of dimension : %d\n",
		ncluster,dimension);

	double **data_list;
	data_list = new double*[ncluster];
	for(int i=0;i<ncluster;++i){
	    data_list[i] = new double[dimension];
	}

	/* read data from the inputfile */
	{
	    std::string line;
	    int i = 0,j = 0;
	    while(getline(file,line)){
                std::string token;
	        std::istringstream stream(line);
	        while(getline(stream,token,' ')){
		    data_list[i][j] = std::stod(token);
		    j++;
		    if(j>=dimension) break;
		}
	    
	    i++;
	    j = 0;
	    if(i>=ncluster) break;
	    }	
	}
	file.close();
	const double start_time = monotonic_seconds();

	/* k-means */

	const double end_time   = monotonic_seconds();
	print_time(end_time - start_time);

	for(int i=0;i<ncluster;++i){
	    delete [] data_list[i];
	}

	delete [] data_list;
	return 0;
}
