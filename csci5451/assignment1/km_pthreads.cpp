/*
 * K-Means Algorithm with pthreads.
 */
#ifndef _POSIX_C_SOURCE
	#define _POSIX_C_SOURCE 199309L
#endif
#include <time.h>
#include <pthread.h>

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <sstream>
#include <string>


#define MAX_LOOP 3

int g_numclusters;
int g_numdimensions;
int g_numcentroids;
int g_numthreads;
int *g_index;
double **g_dataList;
double **g_centroids;

pthread_t *g_threads;
pthread_cond_t g_cond_update;
pthread_mutex_t g_mutex;
pthread_mutex_t g_update_mutex;

static inline double monotonic_seconds();
static void print_time(const double seconds);
static inline double getDistance(double *point,double *centroid,int dim);
void readDataFromFile(const char *filepath);
void deletePointer();

void *calcClusters(void *arg);
bool updateCentroids();

int main(int argc,char *argv[]){

	if(argc != 4){
	    printf("Usage %s k-meshdata number_of_cluster number_of_threads\n",argv[0]);
	    return 0;
	}
	/* read clusters, dimension */
	readDataFromFile(argv[1]);

	pthread_cond_init(&g_cond_update,NULL);
	pthread_mutex_init(&g_mutex,NULL);
	pthread_mutex_init(&g_update_mutex,NULL);
	g_numcentroids = std::stoi(argv[2]);
	g_centroids = new double*[g_numcentroids];
	for(int i=0;i<g_numcentroids;++i){
		g_centroids[i] = new double[g_numdimensions];
		for(int j=0;j<g_numdimensions;++j)
			g_centroids[i][j] = g_dataList[i][j];
	}

	g_numthreads = std::stoi(argv[3]);
	g_index = new int[g_numthreads];
	g_threads = new pthread_t[g_numthreads];

    const double start_time = monotonic_seconds();
    /* k-means */
	for(int i=0;i<g_numthreads;++i){
		g_index[i] = i;
		pthread_create(&g_threads[i],NULL,
						calcClusters,(void*)&g_index[i]);
	}


	for(int i=0;i<g_numthreads;++i){
		pthread_join(g_threads[i],NULL);
	}

	const double end_time   = monotonic_seconds();
	print_time(end_time - start_time);

	deletePointer();
	return 0;
}

void *calcClusters(void *arg){
	int *ptr = (int*)arg;
	int index = *ptr;

	for(int i=0;i<MAX_LOOP;++i){
		pthread_mutex_lock(&g_mutex);
		if(!updateCentroids())
			pthread_cond_wait(&g_cond_update,&g_mutex);
		pthread_mutex_unlock(&g_mutex);
	}
	pthread_exit(0);
}

int cnt  = 0;
int loop = 1;
bool updateCentroids(){
	cnt++;
	if(loop == MAX_LOOP) return true;	
	if(cnt == g_numthreads){
		cnt = 0;
		loop++;
		pthread_cond_broadcast(&g_cond_update);
		return true;
	}
	return false;
}

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


void readDataFromFile(const char *filepath){
    /* read the data points from input files */
        // read the first line
	std::ifstream file(filepath);
    if(!file){
        printf("Failure to open %s\n",filepath);
        exit(0);
    }

	{
		std::string line;
		getline(file,line);
		std::istringstream stream(line);
		std::string token;
		stream >> g_numclusters >> g_numdimensions;
	}

    printf("The number of cluster    : %d\n"
           "The number of dimension : %d\n",
        g_numclusters,g_numdimensions);

    g_dataList = new double*[g_numclusters];
    for(int i=0;i<g_numclusters;++i){
        g_dataList[i] = new double[g_numdimensions];
    }

	int i = 0,j = 0;
	std::string line;
	while(getline(file,line)){
		std::istringstream stream(line);
		std::string token;
		while(getline(stream,token,' ')){
			g_dataList[i][j] = std::stod(token);
			j++;
			if(j>=g_numdimensions) break;
		}
		i++;
		j = 0;
		if(i>=g_numclusters) break;
	}

    file.close();
}

void deletePointer(){
	for(int i=0;i<g_numclusters;++i){
		delete [] g_dataList[i];
	}
	delete [] g_dataList;
}
