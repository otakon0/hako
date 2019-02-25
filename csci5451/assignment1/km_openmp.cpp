/*
 * K-Means Algorithm with OpenMP
 */
#ifndef _POSIX_C_SOURCE
	#define _POSIX_C_SOURCE 199309L
#endif

#ifdef _OPENMP
#include <omp.h>
#endif

#include <time.h>

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <sstream>
#include <string>
#include <iomanip>

#define MAX_LOOP 20

static inline double monotonic_seconds();
static void print_time(const double seconds);
static inline double getDistance(double *point,double *centroid,int dim);

int main(int argc,char *argv[]){

	if(argc != 4){
		printf("Usage %s k-meshdata number_of_cluster number_of_threads\n",argv[0]);
		return 0;
	}

	int g_numclusters;
	int g_numcentroids;
	int g_numdimensions;
	int g_numthreads;
	int *g_currentPositions;
	int *g_previousPositions;
	int *g_counterAcc;
	double **g_centroids;
	double **g_centroidsAcc;
	double **g_dataList;

	g_numcentroids = std::stoi(argv[2]);
	g_numthreads   = std::stoi(argv[3]);

	std::ifstream file(argv[1]);
	if(!file){
		printf("Failure to open %s\n",argv[1]);
		exit(0);
	}

	/* read data from the file given as an argument */
	{
		std::string line;
		getline(file,line);
		std::istringstream stream(line);
		stream >> g_numclusters >> g_numdimensions;
		printf("The number of clusters    : %d\n"
			   "The number of dimension   : %d\n",
		   			g_numclusters,g_numdimensions);

		g_dataList = new double*[g_numclusters];
		for(int i=0;i<g_numclusters;++i)
			g_dataList[i] = new double[g_numdimensions];

		int i=0,j=0;
		while(getline(file,line)){
			std::istringstream st(line);
			std::string token;
			while(getline(st,token,' ')){
				g_dataList[i][j] = std::stod(token);
				j++;
				if(j>=g_numdimensions) break;
			}
			i++;
			j = 0;
			if(i>=g_numclusters) break;
		}


		/* Select the first some data points as the clusters */
		g_centroids = new double*[g_numcentroids];
		for(i=0;i<g_numcentroids;++i){
			g_centroids[i] = new double[g_numdimensions];
			for(j=0;j<g_numdimensions;++j){
				g_centroids[i][j] = g_dataList[i][j];
			}
		}

	}
	file.close();

	g_centroidsAcc = new double*[g_numcentroids];
	for(int i=0;i<g_numcentroids;++i){
		g_centroidsAcc[i] = new double[g_numdimensions];
		for(int j=0;j<g_numdimensions;++j){
			g_centroidsAcc[i][j] = 0.0;
		}
	}
	g_counterAcc = new int[g_numcentroids];
	for(int i=0;i<g_numcentroids;++i){
		g_counterAcc[i] = 0;
	}

	g_currentPositions  = new int[g_numclusters];
	g_previousPositions = new int[g_numclusters];

	/* k-means */
	bool isConvergence = false;
	const double start_time = monotonic_seconds();
	for(int i=0;i<MAX_LOOP && isConvergence == false;++i){
		double closeDst;
		int closePos;
		int k;
		double tmp;
		#pragma omp parallel num_threads(g_numthreads) shared(isConvergence)
		{
			#pragma omp for private(k,tmp,closeDst,closePos)
			for(int j=0;j<g_numclusters;++j){
				closeDst = getDistance(g_dataList[j],g_centroids[0],g_numdimensions);
				closePos = 0;
				for(k=0;k<g_numcentroids;++k){
					tmp = getDistance(g_dataList[j],g_centroids[k],g_numdimensions);
					if(tmp < closeDst){
						closeDst = tmp;
						closePos = k;
					}
				}
				#pragma omp critical
				{
					g_currentPositions[j] = closePos;
					g_counterAcc[closePos]++;
					for(k=0;k<g_numdimensions;++k){
						g_centroidsAcc[closePos][k] += g_dataList[j][k];
					}
				}
			}

			#pragma omp barrier
			#pragma omp single
			{
				bool flag = true;
				for(int l=0;l<g_numclusters && flag;++l){
					if(g_currentPositions[l] != g_previousPositions[l]) 
						flag = false;
				}
				
				isConvergence = flag;

				int *tmpPos = g_currentPositions;
				g_currentPositions  = g_previousPositions;
				g_previousPositions = tmpPos;

				for(int l=0;l<g_numcentroids;++l){
					double cnt = (double)g_counterAcc[l];
					g_counterAcc[l] = 0;
					for(int m=0;m<g_numdimensions;++m){
						g_centroids[l][m]    = g_centroidsAcc[l][m] / cnt;
						g_centroidsAcc[l][m] = 0.0;
					}
				}
				printf("%2d iteration complete\n",i); 
			}

		}

	}

	const double end_time   = monotonic_seconds();
	print_time(end_time - start_time);

	/* write date into file */
	const char* file1 = "clusters.txt";
	const char* file2 = "centroids.txt";

	std::ofstream outputFile;
	outputFile.open(file1,std::ios::out);
	for(int i=0;i<g_numclusters;++i){
		outputFile << g_currentPositions[i] << std::endl;
	}
	outputFile.close();

	outputFile.open(file2);
	for(int i=0;i<g_numcentroids;++i){
		for(int j=0;j<g_numdimensions;++j){
			outputFile << g_centroids[i][j] << " ";
		}
		outputFile << std::endl;
	}

	/* delete pointers */
	for(int i=0;i<g_numclusters;++i){
		delete [] g_dataList[i];
	}
	delete [] g_dataList;

	for(int i=0;i<g_numcentroids;++i){
		delete [] g_centroids[i];
		delete [] g_centroidsAcc[i];
	}
	delete [] g_centroids;
	delete [] g_centroidsAcc;
	delete [] g_counterAcc;
	delete [] g_currentPositions;
	delete [] g_previousPositions;


	return 0;
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
