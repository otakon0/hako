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
#include <iomanip>

#define MAX_LOOP 20

int g_numclusters;
int g_numdimensions;
int g_numcentroids;
int g_numthreads;
int g_blocksize;
int *g_index;
int **g_counterAcc;
int **g_positionAcc;
int **g_lastPositionAcc;
double **g_dataList;
double **g_centroids;
double ***g_centroidsAcc;
bool g_isConvergence;
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
	g_blocksize  = g_numclusters / g_numthreads;
	g_index = new int[g_numthreads];
	g_threads = new pthread_t[g_numthreads];
	g_centroidsAcc = new double**[g_numthreads];
	g_counterAcc   = new int*[g_numthreads];
	g_positionAcc  = new int*[g_numthreads];
	g_lastPositionAcc = new int*[g_numthreads];
	g_isConvergence = false;
    const double start_time = monotonic_seconds();
    /* k-means */
	for(int i=0;i<g_numthreads;++i){
		g_index[i] = i;
		pthread_create(&g_threads[i],NULL,
						calcClusters,(void*)&g_index[i]);
	}


	/* wait for all threads completing */
	for(int i=0;i<g_numthreads;++i){
		pthread_join(g_threads[i],NULL);
	}

	const double end_time   = monotonic_seconds();
	print_time(end_time - start_time);

	/* write date into file */
	const char* file1 = "clusters.txt";
	const char* file2 = "centroids.txt";

	std::ofstream outputFile;
	outputFile.open(file1,std::ios::out);
	for(int i=0;i<g_numthreads-1;++i){
		for(int j=0;j<g_blocksize;++j){
			outputFile << g_positionAcc[i][j] << std::endl;
		}
	}
	int len = g_numclusters - (g_blocksize)*(g_numthreads-1);
	for(int i=0;i<len;++i){
		outputFile << std::noshowpoint << g_positionAcc[g_numthreads-1][i] << std::endl;
	}
	outputFile.close();

	outputFile.open(file2);
	for(int i=0;i<g_numcentroids;++i){
		for(int j=0;j<g_numdimensions;++j){
			outputFile << g_centroids[i][j] << " ";
		}
		outputFile << std::endl;
	}


	deletePointer();
	return 0;
}

void *calcClusters(void *arg){
	/* use local variables as much as possible */
	int *ptr = (int*)arg;
	const int index = *ptr;
	const int loopStart = index*g_blocksize;
	const int loopEnd   = index == (g_numthreads-1) ? g_numclusters : g_blocksize*(index+1);
	const int numdimensions = g_numdimensions;
	const int numcentroids  = g_numcentroids;
	int *position     = new int[loopEnd - loopStart];
	int *lastPosition = new int[loopEnd-loopStart];
	int *counter      = new int[numcentroids];
	double **centroidSum = new double*[numcentroids];
	for(int i=0;i<numcentroids;++i){
		centroidSum[i] = new double[numdimensions];
	}

	g_centroidsAcc[index] = centroidSum;
	g_counterAcc[index]   = counter;
	g_positionAcc[index]  = position;
	g_lastPositionAcc[index] = lastPosition;

	for(int i=0;i<MAX_LOOP;++i){

		if(g_isConvergence) break;
		/* Initialize the local accumulator */
		for(int j=0;j<numcentroids;++j){
			counter[j] = 0;
			for(int k=0;k<numdimensions;++k)
				centroidSum[j][k] = 0.0;
		}
				
		for(int j=loopStart;j<loopEnd;++j){
			int closePos    = 0;
			double closeDst = getDistance(g_dataList[j],g_centroids[0],numdimensions);

			for(int k=1;k<numcentroids;++k){
				double tmp = getDistance(g_dataList[j],g_centroids[k],numdimensions);
				if(tmp < closeDst){
					closePos = k;
					closeDst  = tmp;
				}
			}
			position[j-loopStart] = closePos;
			counter[closePos]++;
			for(int k=0;k<numdimensions;++k)
				centroidSum[closePos][k] += g_dataList[j][k];
		}
		printf("[%2d] : g_clusters[%6d] ~ g_cluster[%6d] \n",i,loopStart,loopEnd-1);

		pthread_mutex_lock(&g_mutex);
		if(!updateCentroids())
			pthread_cond_wait(&g_cond_update,&g_mutex);
		pthread_mutex_unlock(&g_mutex);
	}

	for(int i=0;i<numcentroids;++i)
		delete [] centroidSum[i];
	delete [] centroidSum;
	// delete [] position; position will be deleted in main function.
	delete [] counter;
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
		for(int i=0;i<g_numcentroids;++i){

			for(int j=0;j<g_numdimensions;++j){
				g_centroids[i][j] = 0.0;
			}

			int centroidCounter = 0;
			for(int j=0;j<g_numthreads;++j){
				centroidCounter += g_counterAcc[j][i];
				for(int k=0;k<g_numdimensions;++k){
					g_centroids[i][k] += g_centroidsAcc[j][i][k];
				}
			}

			for(int j=0;j<g_numdimensions;++j){
				g_centroids[i][j] = g_centroids[i][j] / centroidCounter;
			}

		}

        /* Convergence is detected when no data points change their cluster assignmen */
        bool flag = true;
        for(int j=0;j<g_numthreads-1;++j){
            for(int k=0;k<g_blocksize;++k){
                if(flag && g_positionAcc[j][k] != g_lastPositionAcc[j][k]) flag = false;
                g_lastPositionAcc[j][k] = g_positionAcc[j][k];
            }
        }
        int len = g_numclusters - (g_blocksize)*(g_numthreads-1);
        int idx = g_numthreads-1;
        for(int j=0;j<len;++j){
            if(flag && g_positionAcc[idx][j] != g_lastPositionAcc[idx][j]) flag = false;
            g_lastPositionAcc[idx][j] = g_positionAcc[idx][j];
        }
		g_isConvergence = flag;


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
           "The number of dimension  : %d\n",
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

	for(int i=0;i<g_numcentroids;++i){
		delete [] g_centroids[i];
	}
	for(int i=0;i<g_numthreads;++i){
		delete [] g_positionAcc[i];
		delete [] g_lastPositionAcc[i];
	}
	delete [] g_centroids;
	delete [] g_centroidsAcc;
	delete [] g_index;
	delete [] g_counterAcc;
	delete [] g_positionAcc;
	delete [] g_lastPositionAcc;
	delete [] g_threads;
}
