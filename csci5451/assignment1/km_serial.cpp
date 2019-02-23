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


#define MAX_LOOP 20
/* Assume that this runs on LINUX */
static inline double monotonic_seconds(){
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC,&ts);
	return ts.tv_sec + ts.tv_nsec * 1e-9;
}
static void print_time(const double seconds){
	printf("k-means clustering time: %0.04fs\n",seconds);
}

static inline double getDistance(double *point,double *centroid,int dim){
	double distance = 0.0;
	for(int i=0;i<dim;++i){
	    distance += (point[i]-centroid[i])*(point[i]-centroid[i]);
	}
	return distance;
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

	int numclusters;
	int numdimensions;
	int numcentroids;
	int numthreads;

	/* read the data points from input files */
	{
	    // read the first line
	    std::string line;
	    getline(file,line);
	    std::istringstream stream(line);
	    std::string token;
	    stream >> numclusters >> numdimensions;
	}

	printf("The number of cluster    : %d\n"
	       "The number #of dimension : %d\n",
		numclusters,numdimensions);

	double **data_list;
	data_list = new double*[numclusters];
	for(int i=0;i<numclusters;++i){
	    data_list[i] = new double[numdimensions];
	}

	{
	    std::string line;
	    int i = 0,j = 0;
	    while(getline(file,line)){
                std::string token;
	        std::istringstream stream(line);
	        while(getline(stream,token,' ')){
		    data_list[i][j] = std::stod(token);
		    j++;
		    if(j>=numdimensions) break;
		}

	    i++;
	    j = 0;
	    if(i>=numclusters) break;
	    }
	}

	file.close();


        /* choose the first some data points as centroids */
        numcentroids = std::stoi(argv[2]);
        double **centroids;
        centroids = new double*[numcentroids];
        for(int i=0;i<numcentroids;++i){
            centroids[i] = new double[numdimensions];
        }
        for(int i=0;i<numcentroids;++i){
            for(int j=0;j<numdimensions;++j){
                centroids[i][j] = data_list[i][j];
            }
        }



	const double start_time = monotonic_seconds();
	/* k-means */
	int *position = new int[numclusters];
	int *counter  = new int[numcentroids];
	double **centroidsum  = new double*[numcentroids];
	for(int i=0;i<numcentroids;++i){
	    centroidsum[i] = new double[numdimensions];
	}
	for(int i=0;i<MAX_LOOP;++i){

	    for(int j=0;j<numcentroids;++j){
		counter[j] = 0;
		for(int k=0;k<numdimensions;++k)
		    centroidsum[j][k] = 0.0;
	    }

	    for(int j=0;j<numclusters;++j){
		double mindistance = getDistance(data_list[j],centroids[0],numdimensions);
		int minposition = 0;
		for(int k=1;k<numcentroids;++k){
		    double tmp = getDistance(data_list[j],centroids[k],numdimensions);
		    if(tmp < mindistance){
			mindistance = tmp;
			minposition = k;
		    }
		}
	        for(int k=0;k<numdimensions;++k){
		    centroidsum[minposition][k] += data_list[j][k];
		}
		counter[minposition]     += 1;
		position[j] = minposition;
	    }

	    /* update centroids position */
	    for(int j=0;j<numcentroids;++j){
		int cnt = counter[j];
		if(cnt == 0){
		    printf("Fatal Error : A centroid does not have any close data\n");
		    exit(1);
		}
	        for(int k=0;k<numdimensions;++k){
		    centroids[j][k] = centroidsum[j][k] / cnt;
		}
	    }
	}

	const double end_time   = monotonic_seconds();
	print_time(end_time - start_time);


	for(int i=0;i<numclusters;++i){
	    delete [] data_list[i];
	}
	for(int i=0;i<numcentroids;++i){
	    delete [] centroidsum[i];
	}
	delete [] data_list;
	delete [] centroids;
	delete [] centroidsum;
	delete [] counter;
	delete [] position;
	return 0;
}
